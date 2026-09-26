#!/usr/bin/env python3
"""Rebuild every system's gamelist.xml from the ROMs actually present.

EmulationStation discovers ROMs from the filesystem, so a game with no
gamelist entry still appears - named after its file. A gamelist that has
drifted from the card is therefore not broken, only untrue: it carries
entries for games that were deleted, and says nothing about games that
were added. This makes it true again.

What it does per system, driven by es_systems.cfg so the extension list is
the one this build actually uses rather than a guess:

  * every file whose extension the system declares becomes an entry
  * entries whose file no longer exists are dropped
  * existing metadata is kept verbatim - playcount, lastplayed, favorite,
    rating, desc, and a scraped <image> whose file is still on the card
  * a media tag pointing at a file that is gone is dropped, so nothing is
    left referring to artwork that was deleted

Safe to re-run: a second pass with nothing changed rewrites the same bytes.

Note on timing: EmulationStation's own updateGamelist() re-reads the file
from disk and merges only the games whose metadata changed during that
session, so it will not undo this. ES still holds the old list in memory
until it restarts, which is why the wrapper restarts it.
"""

import datetime
import html
import os
import re
import sys

CFG_CANDIDATES = [
    "/etc/emulationstation/es_systems.cfg",
    "/roms/tools/es-custom/es_systems.cfg",
]
BACKUP = "/roms/tools/es-custom/gamelist-backup"
# Each run saves into its own dated folder. Overwriting one shared folder
# would mean a second run backs up the first run's output, leaving nothing
# to return to.

# Directories that hold artwork or emulator data, never games.
MEDIA_DIRS = {"downloaded_images", "downloaded_videos", "images", "media", "videos"}

# Tags that name a file on disk. Kept only while that file exists.
MEDIA_TAGS = {"image", "thumbnail", "marquee", "video", "fanart", "titleshot",
              "boxart", "manual", "map", "bezel", "mix", "cartridge",
              "boxback", "wheel"}

# Metadata worth carrying forward, in the order ES writes it.
KEEP_TAGS = ["desc", "rating", "releasedate", "developer", "publisher", "genre",
             "players", "playcount", "lastplayed", "favorite", "hidden",
             "kidgame", "sortname", "region", "lang", "core", "emulator"]

# Systems this tool must not touch, and why.
SKIP_SYSTEMS = {
    "ports":   "PortMaster owns ports/gamelist.xml and rewrites it itself",
    "j2me":    "its only .jar files are JDK internals (jdk/demo/jfc/...), not games",
    "scummvm": "holds launcher stubs only",
    "easyrpg": "holds launcher stubs only",
    "wolf":    "holds launcher stubs only",
    "pymo":    "holds launcher stubs only",
}

# Launcher stubs that ES lists like games but which are not games.
STUB_NAMES = {"scan_for_new_games", "menu"}

# Titles for arcade sets missing from EmulationStation's mamenames.xml, kept
# beside this script. Only consulted when the gamelist has no name of its own.
HERE = os.path.dirname(os.path.abspath(__file__))
EXTRA_NAMES_FILE = os.path.join(HERE, "mame-extra-names.tsv")

# EmulationStation can turn a MAME set name into a real title by itself, but
# only if it finds this table at runtime and only for arcade platforms. Rather
# than depend on that, the rebuilder looks the names up and writes them into
# the gamelist, where nothing can fail to apply them.
MAMENAMES_FILES = [
    os.path.join(HERE, "mamenames.xml"),
    "/usr/share/emulationstation/resources/mamenames.xml",
    "/etc/emulationstation/resources/mamenames.xml",
    "/usr/local/share/emulationstation/resources/mamenames.xml",
]


def load_mame_names(override=None):
    """MAME set name -> real title, from whichever copy of the table exists."""
    for path in ([override] if override else []) + MAMENAMES_FILES:
        if not os.path.isfile(path):
            continue
        try:
            text = open(path, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        pairs = re.findall(
            r"<mamename>(.*?)</mamename>\s*<realname>(.*?)</realname>", text, re.S)
        if pairs:
            return {k.strip(): html.unescape(v).strip() for k, v in pairs}, path
    return {}, None


def load_extra_names():
    out = {}
    try:
        fh = open(EXTRA_NAMES_FILE, encoding="utf-8")
    except OSError:
        return out
    with fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#") or "\t" not in line:
                continue
            code, title = line.split("\t", 1)
            if code and title:
                out[code.strip()] = title.strip()
    return out


def find_cfg(roms_root=None):
    candidates = list(CFG_CANDIDATES)
    if roms_root:
        candidates.insert(0, os.path.join(roms_root, "tools/es-custom/es_systems.cfg"))
    for p in candidates:
        if os.path.isfile(p):
            return p
    return None


def systems(cfg_path, roms_root=None):
    """(name, rom_dir, {extensions}, platform) for every system with a folder.

    roms_root relocates the /roms prefix, so the same code can be pointed at
    a copy of the card for testing instead of the live one.
    """
    cfg = open(cfg_path, encoding="utf-8", errors="replace").read()
    out = []
    for block in re.findall(r"<system>(.*?)</system>", cfg, re.S):
        def tag(t):
            m = re.search(r"<%s>(.*?)</%s>" % (t, t), block, re.S)
            return m.group(1).strip() if m else ""
        name, path, ext = tag("name"), tag("path"), tag("extension")
        platform = tag("platform").lower()
        path = path.replace("~", os.path.expanduser("~"))
        if not path.startswith(("/roms/", "/roms2/")):
            continue
        rom_dir = path.rstrip("/")
        if roms_root:
            # "/roms/snes" -> "<root>/snes"; "/roms2/psx" -> "<root>/psx"
            parts = rom_dir.split("/", 2)
            rel = parts[2] if len(parts) > 2 else ""
            rom_dir = os.path.join(roms_root, rel) if rel else roms_root
        if not os.path.isdir(rom_dir):
            continue
        exts = {e.lower() for e in ext.split() if e.startswith(".")}
        if exts:
            out.append((name, rom_dir, exts, platform))
    return out


def roms(rom_dir, exts):
    found = []
    for dirpath, dirnames, files in os.walk(rom_dir):
        # Dot-directories are deliberately hidden - psx/.hidden holds the
        # individual discs of multi-disc games, which must stay out of the
        # list or every such game appears once per disc.
        dirnames[:] = [d for d in dirnames
                       if not d.startswith(".") and d.lower() not in MEDIA_DIRS]
        for f in files:
            if f.startswith("."):
                continue
            stem, ext = os.path.splitext(f)
            if ext.lower() not in exts:
                continue
            if stem.lower().replace(" ", "_") in STUB_NAMES:
                continue
            found.append("./" + os.path.relpath(os.path.join(dirpath, f), rom_dir))
    return sorted(found, key=str.lower)


def read_existing(gamelist, rom_dir):
    """path -> {tag: value}, media tags kept only if the file still exists."""
    if not os.path.isfile(gamelist):
        return {}
    text = open(gamelist, encoding="utf-8", errors="replace").read()
    out = {}
    for block in re.findall(r"<game>(.*?)</game>", text, re.S):
        pm = re.search(r"<path>(.*?)</path>", block, re.S)
        if not pm:
            continue
        key = html.unescape(pm.group(1).strip())
        if not key.startswith("./"):
            key = "./" + key.lstrip("/")
        meta = {}
        for t, v in re.findall(r"<(\w+)>(.*?)</\1>", block, re.S):
            if t == "path":
                continue
            v = html.unescape(v)
            if t in MEDIA_TAGS:
                target = v[2:] if v.startswith("./") else v.lstrip("/")
                if not v or not os.path.exists(os.path.join(rom_dir, target)):
                    continue        # artwork is gone; drop the dangling reference
            meta[t] = v
        out[key] = meta
    return out


def esc(s):
    return html.escape(s, quote=False)


def render(entries, existing, names=None):
    """names: set name -> real title, or None for systems that do not use them."""
    names = names or {}
    lines = ['<?xml version="1.0"?>', "<gameList>"]
    for path in entries:
        meta = dict(existing.get(path, {}))
        stem = os.path.splitext(os.path.basename(path))[0]
        name = meta.pop("name", "")
        lines.append("\t<game>")
        lines.append("\t\t<path>%s</path>" % esc(path))
        # Never write a name that is merely the filename. EmulationStation
        # derives that itself - and for arcade and neogeo it runs the stem
        # through mamenames.xml first, so "aodk" is shown as "Aggressors of
        # Dark Kombat". Writing the stem as an explicit <name> overrides that
        # lookup and leaves the short MAME code on screen. ES omits the name
        # for the same reason when it saves a gamelist itself.
        if not name or name == stem:
            name = names.get(stem, "")
        if name and name != stem:
            lines.append("\t\t<name>%s</name>" % esc(name))
        for t in KEEP_TAGS:
            if meta.get(t):
                lines.append("\t\t<%s>%s</%s>" % (t, esc(meta[t]), t))
        for t in sorted(MEDIA_TAGS):
            if meta.get(t):
                lines.append("\t\t<%s>%s</%s>" % (t, esc(meta[t]), t))
        lines.append("\t</game>")
    lines.append("</gameList>")
    return "\n".join(lines) + "\n"


def main():
    dry = "--dry-run" in sys.argv
    roms_root = None
    mamenames_override = None
    backup = BACKUP
    for a in sys.argv[1:]:
        if a.startswith("--mamenames="):
            mamenames_override = a.split("=", 1)[1]
        if a.startswith("--roms-root="):
            roms_root = a.split("=", 1)[1].rstrip("/")
            backup = os.path.join(roms_root, "tools/es-custom/gamelist-backup")
    backup = os.path.join(backup, datetime.datetime.now().strftime("%Y%m%d-%H%M%S"))

    cfg = find_cfg(roms_root)
    if not cfg:
        print("ERROR: no es_systems.cfg found. Looked in:")
        for p in CFG_CANDIDATES:
            print("   " + p)
        return 1
    print("Reading systems from %s" % cfg)
    print()

    if not dry:
        os.makedirs(backup, exist_ok=True)

    mame, mame_src = load_mame_names(mamenames_override)
    extra = load_extra_names()
    lookup = dict(mame)
    lookup.update(extra)          # our own titles win over the shipped table
    if mame_src:
        print("MAME titles: %d from %s" % (len(mame), mame_src))
    else:
        print("MAME titles: none found - arcade names will be left to ES")
    if extra:
        print("plus %d from mame-extra-names.tsv" % len(extra))
    print()

    rows, skipped = [], []
    for name, rom_dir, exts, platform in systems(cfg, roms_root):
        if name in SKIP_SYSTEMS:
            skipped.append((name, SKIP_SYSTEMS[name]))
            continue
        entries = roms(rom_dir, exts)
        if not entries:
            continue
        gamelist = os.path.join(rom_dir, "gamelist.xml")
        existing = read_existing(gamelist, rom_dir)
        present = set(entries)
        dropped = [k for k in existing if k not in present]
        added = [e for e in entries if e not in existing]
        new_file = not os.path.isfile(gamelist)

        if not dry:
            if os.path.isfile(gamelist):
                tag = os.path.relpath(gamelist, roms_root or "/roms").replace("/", "__")
                try:
                    with open(gamelist, "rb") as src, open(os.path.join(backup, tag), "wb") as dst:
                        dst.write(src.read())
                except OSError as e:
                    print("  SKIPPED %-14s could not back up: %s" % (name, e))
                    continue
            with open(gamelist, "w", encoding="utf-8") as fh:
                # Only arcade-family systems use MAME set names; everywhere
                # else the filename already is the title.
                uses_mame = ("arcade" in platform) or (platform == "neogeo")
                fh.write(render(entries, existing, lookup if uses_mame else None))

        rows.append((name, len(entries), len(existing) - len(dropped),
                     len(added), len(dropped), new_file))

    print("%-16s %6s %7s %7s %8s" % ("SYSTEM", "GAMES", "KEPT", "ADDED", "DROPPED"))
    print("-" * 50)
    tot_add = tot_drop = 0
    for name, n, kept, added, dropped, new_file in rows:
        tot_add += added
        tot_drop += dropped
        print("%-16s %6d %7d %7d %8d%s"
              % (name, n, kept, added, dropped, "   (new)" if new_file else ""))
    print()
    print("%d systems, %d entries added, %d stale dropped"
          % (len(rows), tot_add, tot_drop))
    if skipped:
        print()
        print("Left alone:")
        for name, why in skipped:
            print("   %-10s %s" % (name, why))
    if dry:
        print()
        print("DRY RUN - nothing was written.")
    else:
        print()
        print("Previous gamelists saved in %s" % backup)
    return 0


if __name__ == "__main__":
    sys.exit(main())
