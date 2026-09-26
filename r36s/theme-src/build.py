#!/usr/bin/env python3
"""Build the Mono theme.xml files from one base plus a delta per theme.

EmulationStation cannot share a file between theme sets -- every directory under
the themes path becomes a set whether or not it holds a theme.xml, <path> values
resolve against the file being parsed rather than the theme root, and a missing
include leaves the theme with no base at all rather than failing loudly. So each
theme.xml has to ship self-contained. This keeps one source of truth anyway and
writes the six files out.

    build.py extract   read r36s/theme/*/theme.xml, write the deltas here
    build.py build     read the deltas, write r36s/theme/*/theme.xml
    build.py check     build into memory and diff against what is on disk

A delta addresses a property by the element that encloses it:

    [carousel systemcarousel]
    selectorColor = 000000FF          ## black capsule on the paper ground

`##` is the XML comment to put on the line. An element name that appears more
than once -- logoText and help both do -- takes every occurrence unless an
index is given, as in `[text logoText #1]`. Two other forms exist:

    someProperty = @remove            drop the line
    [insert after carousel systemcarousel]   ... raw XML, one block per line
"""

import os, re, sys, difflib

HERE   = os.path.dirname(os.path.abspath(__file__))
THEMES = os.path.normpath(os.path.join(HERE, "..", "theme"))
BASE   = os.path.join(HERE, "base.xml")
# Mono Dark IS the base; the rest carry deltas against it.
ORDER  = ["Mono Dark", "Mono Light", "Mono Fit Dark", "Mono Fit Light",
          "Mono Classic", "Mono Max"]

EL_OPEN  = re.compile(r'<([a-zA-Z]+)\s+name="([^"]*)"')
PROP     = re.compile(r'^(\s*)<([a-zA-Z][a-zA-Z0-9]*)>(.*?)</\2>(.*)$', re.S)


def block_map(lines):
    """For each line, the (tag, name, occurrence) of the element enclosing it.

    Views are skipped deliberately: they repeat constantly and the elements
    inside them are specific enough on their own."""
    out, stack, seen = [], [], {}
    for ln in lines:
        opened = None
        m = EL_OPEN.search(ln)
        if m and m.group(1) != "view" and not ln.strip().startswith("<!--"):
            key = (m.group(1), m.group(2))
            seen[key] = seen.get(key, -1) + 1
            opened = key + (seen[key],)
            # a one-line element opens and closes on the same line
            if not re.search(r'</%s>' % m.group(1), ln):
                stack.append(opened)
        out.append(stack[-1] if stack else opened)
        if stack and ln.strip().startswith("</%s>" % stack[-1][0]):
            stack.pop()
    return out


def read(p):
    with open(p, encoding="utf-8", errors="surrogateescape") as f:
        return f.read().split("\n")


def write(p, lines):
    os.makedirs(os.path.dirname(p), exist_ok=True)
    with open(p, "w", encoding="utf-8", errors="surrogateescape") as f:
        f.write("\n".join(lines))


def parse_delta(path):
    """-> (props, inserts, header) where props is {(tag,name,idx|None): [(prop, value, comment|None)]}"""
    props, inserts, header = {}, [], {}
    section, raw = None, None
    for ln in read(path):
        if raw is not None:
            if ln.strip() == "[end]":
                inserts.append((raw[0], raw[1], raw[2])); raw = None
            else:
                raw[2].append(ln)
            continue
        s = ln.strip()
        if not s or s.startswith("#") and not s.startswith("##"):
            continue
        if s.startswith("@"):
            k, _, v = s[1:].partition(" ")
            header[k.strip()] = v.strip()
            continue
        if s.startswith("[replace "):
            m = re.match(r'\[replace ([a-zA-Z]+) (.+?)(?: #(\d+))? ([A-Za-z][A-Za-z0-9]*)\]$', s)
            raw = ("@replace", (m.group(1), m.group(2), int(m.group(3)) if m.group(3) else None, m.group(4)), [])
            continue
        if s.startswith("[remove "):
            m = re.match(r'\[remove ([a-zA-Z]+) (.+)\]$', s)
            props.setdefault(("@block", m.group(1), m.group(2)), [])
            continue
        if s.startswith("[insert "):
            m = re.match(r'\[insert (before|after) ([a-zA-Z]+) (.+)\]$', s)
            raw = (m.group(1), (m.group(2), m.group(3)), [])
            continue
        if s.startswith("["):
            m = re.match(r'\[([a-zA-Z]+) (.+?)(?: #(\d+))?\]$', s)
            section = (m.group(1), m.group(2), int(m.group(3)) if m.group(3) else None)
            props.setdefault(section, [])
            continue
        key, _, rest = s.partition("=")
        val, _, comment = rest.partition("##")
        val = val.strip()
        # "@after someProperty" says where to put a property the base lacks
        anchor = None
        am = re.search(r'@after\s+([A-Za-z][A-Za-z0-9]*)$', val)
        if am:
            anchor = am.group(1)
            val = val[:am.start()].strip()
        props[section].append((key.strip(), val, comment.strip() or None, anchor))
    return props, inserts, header


def apply_delta(base, props, inserts, header):
    lines = list(base)
    # header lines carry the theme's identity
    for i, ln in enumerate(lines[:8]):
        if header.get("name") and ln.startswith("theme name:"):
            lines[i] = "theme name:     " + header["name"]
        if header.get("family") and ln.startswith("Family:"):
            lines[i] = "Family:         " + header["family"]

    for (tag, name, idx), entries in props.items():
        if tag == "@block":
            keep, drop = [], False
            for ln in lines:
                m = EL_OPEN.search(ln)
                if m and (m.group(1), m.group(2)) == (name, idx):
                    drop = True
                if not drop:
                    keep.append(ln)
                if drop and ln.strip().startswith("</%s>" % name):
                    drop = False
            lines = keep
            continue
        blocks = block_map(lines)
        for prop, val, comment, anchor in entries:
            hit = False
            for i, ln in enumerate(lines):
                b = blocks[i]
                if not b or b[0] != tag or b[1] != name:
                    continue
                if idx is not None and b[2] != idx:
                    continue
                m = PROP.match(ln)
                if not m or m.group(2) != prop:
                    continue
                hit = True
                if val == "@remove":
                    lines[i] = None
                else:
                    tail = ("<!-- %s -->" % comment) if comment else ""
                    lines[i] = "%s<%s>%s</%s>%s" % (m.group(1), prop, val, prop, tail)
            if not hit and val != "@remove":
                # The base has no such property. anchor says which existing one
                # to sit after, so a variant's extra lines land where they were
                # written rather than all bunched at the end of the block.
                at = None
                for i, ln in enumerate(lines):
                    b = blocks[i]
                    if not b or b[0] != tag or b[1] != name or (idx is not None and b[2] != idx):
                        continue
                    m = PROP.match(ln)
                    if anchor and m and m.group(2) == anchor:
                        at = i + 1
                    elif at is None:
                        at = i + 1
                if at is not None:
                    tail = ("<!-- %s -->" % comment) if comment else ""
                    indent = None
                    for i, ln in enumerate(lines):
                        b = blocks[i]
                        if b and b[0] == tag and b[1] == name and (idx is None or b[2] == idx) and PROP.match(ln):
                            indent = re.match(r'\s*', ln).group(0); break
                    if indent is None:
                        indent = re.match(r'\s*', lines[at - 1]).group(0)
                    lines.insert(at, "%s<%s>%s</%s>%s" % (indent, prop, val, prop, tail))
            lines = [l for l in lines if l is not None]
            blocks = block_map(lines)

    for where, target, body in inserts:
        if where == "@replace":
            tag, name, idx, prop = target
            blocks = block_map(lines)
            for i, ln in enumerate(lines):
                b, m = blocks[i], PROP.match(ln)
                if not b or b[0] != tag or b[1] != name or (idx is not None and b[2] != idx):
                    continue
                if not m or m.group(2) != prop:
                    continue
                lines[i:prop_span(lines, i) + 1] = body
                break
            continue
        tag, name = target
        blocks = block_map(lines)
        at = None
        for i, b in enumerate(blocks):
            if b and b[0] == tag and b[1] == name:
                if where == "before" and at is None:
                    at = i
                elif where == "after":
                    at = i + 1
        if at is not None:
            lines[at:at] = body
    return lines


def prop_span(lines, i):
    """Last line of the property starting at i -- its comment may run on."""
    tail = PROP.match(lines[i]).group(4) or ""
    if "<!--" in tail and "-->" not in tail:
        j = i + 1
        while j < len(lines) and "-->" not in lines[j]:
            j += 1
        return min(j, len(lines) - 1)
    return i


def prop_index(lines):
    """(tag, name, idx, prop) -> (line number, value, comment)"""
    out, blocks = {}, block_map(lines)
    for i, ln in enumerate(lines):
        b = blocks[i]
        m = PROP.match(ln)
        if b and m:
            end = prop_span(lines, i)
            c = re.match(r'\s*<!--\s*(.*?)\s*-->\s*$', m.group(4) or "")
            out[(b[0], b[1], b[2], m.group(2))] = (
                i, m.group(3), c.group(1) if c else None, end, lines[i:end + 1])
    return out


def extract(theme):
    base = read(BASE)
    cur  = read(os.path.join(THEMES, theme, "theme.xml"))
    bi, ci = prop_index(base), prop_index(cur)

    hdr = {}
    for ln in cur[:8]:
        if ln.startswith("theme name:"): hdr["name"]   = ln.split(":", 1)[1].strip()
        if ln.startswith("Family:"):     hdr["family"] = ln.split(":", 1)[1].strip()

    # for a property the base lacks, remember the one it follows in its block
    order = {}
    for i, ln in enumerate(cur):
        b = block_map(cur)[i] if False else None
    blocks_cur = block_map(cur)
    prev_in_block = {}
    last_seen = {}
    for i, ln in enumerate(cur):
        b, m = blocks_cur[i], PROP.match(cur[i])
        if b and m:
            key = (b[0], b[1], b[2])
            prev_in_block[(b[0], b[1], b[2], m.group(2))] = last_seen.get(key)
            last_seen[key] = m.group(2)

    changed, removed = [], []
    for k, (i, val, com, end, raw) in ci.items():
        if k not in bi or (bi[k][1], bi[k][2]) != (val, com) or end != i or bi[k][3] != bi[k][0]:
            anchor = None
            if k not in bi:
                anchor = prev_in_block.get(k)
            changed.append((k, val, com, anchor))
    for k in bi:
        if k not in ci:
            removed.append(k)

    # anything the property view cannot express: whole blocks only in the theme
    sm = difflib.SequenceMatcher(None, base, cur, autojunk=False)
    raw = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != "insert":
            continue
        body = cur[j1:j2]
        if all(PROP.match(l) or not l.strip() for l in body):
            continue                      # covered by the property entries
        anchor = None
        for k in range(j2, len(cur)):
            m = EL_OPEN.search(cur[k])
            if m and m.group(1) != "view":
                anchor = (m.group(1), m.group(2)); break
        if anchor:
            raw.append(("before", anchor, body))

    lines = ["# %s -- generated by build.py extract; edit and re-run build." % theme, ""]
    if hdr.get("name"):   lines.append("@name    " + hdr["name"])
    if hdr.get("family"): lines.append("@family  " + hdr["family"])
    lines.append("")
    repeats = set()
    seen_names = {}
    for (t, n, idx, _p) in list(bi) + list(ci):
        seen_names.setdefault((t, n), set()).add(idx)
    for k, v in seen_names.items():
        if len(v) > 1:
            repeats.add(k)

    groups = {}
    for (t, n, idx, prop), val, com, anchor in changed:
        groups.setdefault((t, n, idx if (t, n) in repeats else None), []).append((idx, prop, val, com, anchor))
    for (t, n, gidx), items in groups.items():
        lines.append("[%s %s%s]" % (t, n, (" #%d" % gidx) if gidx is not None else ""))
        for idx, prop, val, com, anchor in items:
            rawlines = ci[(t, n, idx, prop)][4]
            if len(rawlines) > 1:
                lines.append("[replace %s %s%s %s]" % (t, n, (" #%d" % gidx) if gidx is not None else "", prop))
                lines += rawlines
                lines.append("[end]")
            else:
                lines.append("%s = %s%s%s" % (prop, val,
                    (" @after " + anchor) if anchor else "",
                    ("   ## " + com) if com else ""))
        lines.append("")
    def top_elements(ls):
        out = []
        for ln in ls:
            m = EL_OPEN.search(ln)
            if m and m.group(1) not in ("view",) and not ln.strip().startswith("<!--"):
                out.append((m.group(1), m.group(2)))
        return out
    gone = [e for e in dict.fromkeys(top_elements(base)) if e not in set(top_elements(cur))]
    for t, n in gone:
        lines.append("[remove %s %s]" % (t, n)); lines.append("")

    for (t, n, idx, prop) in removed:
        lines.append("[%s %s]" % (t, n)); lines.append("%s = @remove" % prop); lines.append("")
    for where, (t, n), body in raw:
        lines.append("[insert %s %s %s]" % (where, t, n)); lines += body; lines.append("[end]"); lines.append("")
    write(os.path.join(HERE, theme + ".delta"), lines)
    return len(changed), len(removed), len(raw)


def build(theme):
    base = read(BASE)
    d = os.path.join(HERE, theme + ".delta")
    if not os.path.exists(d):
        return base
    props, inserts, header = parse_delta(d)
    return apply_delta(base, props, inserts, header)


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "extract":
        for t in ORDER:
            if t == "Mono Dark":
                write(BASE, read(os.path.join(THEMES, t, "theme.xml"))); print("base.xml  <- Mono Dark"); continue
            print("%-16s %d changed, %d removed, %d raw" % ((t,) + extract(t)))
    elif cmd in ("build", "check"):
        bad = 0
        for t in ORDER:
            got  = build(t)
            path = os.path.join(THEMES, t, "theme.xml")
            have = read(path) if os.path.exists(path) else []
            if cmd == "build":
                write(path, got); print("wrote %s" % t)
            else:
                same = got == have
                bad += 0 if same else 1
                print("%-16s %s" % (t, "identical" if same else
                      "DIFFERS (%d lines)" % sum(1 for _ in difflib.unified_diff(have, got, n=0) if _[:1] in "+-")))
        sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
