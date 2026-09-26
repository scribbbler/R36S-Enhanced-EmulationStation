# Tools

These land in `/roms/tools` on the card, which EmulationStation surfaces as
**Options → Tools** (`/opt/system/Tools` is a symlink to it). Any `.sh` found
there becomes a menu entry and any directory becomes a submenu, so the folders
below *are* the menu structure — moving a file moves the entry.

Two consequences worth remembering:

* Anything a tool leaves behind ending in `.sh` comes back as a menu entry.
  Backups are therefore written as `.sh.bak`.
* A menu label comes from a `# NAME:` comment in the first ten lines if there
  is one, otherwise from the filename. That is how `PS1 - Build Multi-Disc
  Playlists.sh` shows up as just "Build Multi-Disc Playlists" inside the PS1
  folder.

| Folder | What is in it |
| --- | --- |
| `EmulationStation` | install and roll back the custom build, fonts, gamelists, logs |
| `PS1` | multi-disc playlists, and hiding ArkOS's destructive m3u tools |
| `Game Overlays` | per-core overlay install/remove, and a probe |
| `Power & Resume` | power-script backups, resume behaviour |
| `Diagnostics` | battery, hardware, LED probes |

Most tools pair up: one does a thing, its sibling undoes it. Each backs up
before it writes, verifies the backup landed before removing an original, and
is safe to run twice.

`Rebuild Gamelists` is the odd one out in being Python rather than shell — it
parses and rewrites XML, which shell does badly. It reads the extension list
from the running system's own `es_systems.cfg` rather than assuming one, and
takes `--dry-run` and `--roms-root=PATH` if you want to try it against a copy
of the card first.
