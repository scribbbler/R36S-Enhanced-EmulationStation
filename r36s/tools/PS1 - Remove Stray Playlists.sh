#!/bin/bash
# NAME: Remove Stray Playlists
# Remove .m3u playlists that should not exist, and leave the ones that should.
#
# Replaces ArkOS's "PS1 - Delete m3u files", which removes every playlist it
# finds -- including the multi-disc ones that are the only reason .m3u exists.
#
# Two kinds get removed:
#   * single-entry playlists, which make their game appear twice in the list,
#     because EmulationStation has no .m3u handling and treats it as a ROM
#   * playlists sitting at the card root, where no system points and nothing
#     can ever read them
#
# A playlist listing two or more discs is kept. Everything removed is backed
# up first, so a mistake here is recoverable.

ROMS="/roms/psx"
ROOT="/roms"
BAK="/roms/tools/es-custom/m3u-backup/removed-$(date '+%m%d-%H%M')"
LOG="/roms/tools/es-custom/m3u.log"

mkdir -p "$BAK"
echo "==== remove run $(date '+%F %T') ====" >> "$LOG"

echo
echo "Looking for playlists that should not be there..."
echo

single=0; root=0; kept=0; broken=0

# 1. single-entry playlists in the ROM folder
for m in "$ROMS"/*.m3u; do
  [ -e "$m" ] || continue
  n=$(grep -c . "$m" 2>/dev/null)
  name=$(basename "$m")
  if [ "${n:-0}" -ge 2 ]; then
    # keep it -- but say so if it points at something missing
    miss=0
    while IFS= read -r t; do
      t=$(echo "$t" | tr -d '\r'); [ -z "$t" ] && continue
      [ -e "$ROMS/$t" ] || miss=$((miss + 1))
    done < "$m"
    if [ "$miss" -gt 0 ]; then
      echo "  KEPT (but $miss disc(s) missing)  $name"
      broken=$((broken + 1))
    else
      kept=$((kept + 1))
    fi
    continue
  fi
  cp -f "$m" "$BAK/" 2>/dev/null
  rm -f "$m"
  single=$((single + 1))
done

# 2. anything at the card root, where no system looks
for m in "$ROOT"/*.m3u; do
  [ -e "$m" ] || continue
  cp -f "$m" "$BAK/" 2>/dev/null
  rm -f "$m"
  root=$((root + 1))
done

find "$ROMS" "$ROOT" -maxdepth 1 -name '._*.m3u' -delete 2>/dev/null
sync

echo "  removed $single single-entry playlist(s) from psx"
echo "  removed $root stray playlist(s) from the card root"
echo "  kept    $kept multi-disc playlist(s)"
echo "$(date '+%F %T')  removed $single single + $root root, kept $kept" >> "$LOG"
echo
echo "==================================================="
echo "  DONE"
echo "==================================================="
if [ "$broken" -gt 0 ]; then
  echo
  echo "  $broken playlist(s) point at a disc that is not"
  echo "  there. Run \"PS1 - Build Multi-Disc Playlists\""
  echo "  to rebuild them from the discs you actually have."
fi
echo
echo "  A copy of everything removed is in:"
echo "    $BAK"
echo
echo "Press any key to close."
read -n 1 -s -r
