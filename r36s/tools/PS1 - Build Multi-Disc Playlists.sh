#!/bin/bash
# NAME: Build Multi-Disc Playlists
# Build .m3u playlists for multi-disc PlayStation games.
#
# Replaces ArkOS's "PS1 - Generate m3u files", which does three harmful things:
# it writes a playlist for EVERY game, it writes them to the card root as well
# as the ROM folder, and it overwrites existing playlists without a backup.
#
# Only multi-disc games get a playlist. EmulationStation has no .m3u handling
# at all -- it treats .m3u as just another ROM extension -- so a playlist for a
# single-disc game simply lists that game twice.
#
# Discs are found wherever they live under the ROM folder, including a hidden
# one such as .hidden/multi-disc, and the playlist records the path they were
# actually found at. Existing playlists are backed up, never silently replaced.

ROMS="/roms/psx"
BAK="/roms/tools/es-custom/m3u-backup"
LOG="/roms/tools/es-custom/m3u.log"

mkdir -p "$BAK"
echo "==== build run $(date '+%F %T') ====" >> "$LOG"

echo
echo "Scanning $ROMS for multi-disc games..."
echo

# every disc-numbered image, at any depth, hidden folders included
discs=$(find "$ROMS" -type f \( -iname '*.chd' -o -iname '*.cue' -o -iname '*.pbp' \) \
        2>/dev/null | grep -iE '\(Disc [0-9]+\)' | sort)

if [ -z "$discs" ]; then
  echo "  No disc-numbered files found. Nothing to do."
  echo "$(date '+%F %T')  no multi-disc games found" >> "$LOG"
  echo; echo "Press any key to close."; read -n 1 -s -r; exit 0
fi

# group by the name with the disc number removed
titles=$(echo "$discs" | sed -E 's|.*/||; s/ \(Disc [0-9]+\)//' | sort -u)

made=0; kept=0; saved=0
while IFS= read -r title; do
  [ -z "$title" ] && continue
  name="${title%.*}"
  # the disc list for this title, in disc order
  set -- ; list=""
  while IFS= read -r d; do
    base=$(basename "$d")
    [ "$(echo "$base" | sed -E 's/ \(Disc [0-9]+\)//')" = "$title" ] || continue
    rel="${d#$ROMS/}"
    list="${list}${rel}"$'\n'
  done <<< "$discs"

  count=$(echo "$list" | grep -c .)
  [ "$count" -lt 2 ] && continue          # single disc: no playlist, by design

  out="$ROMS/$name.m3u"
  if [ -e "$out" ] && [ "$(cat "$out")" = "$(echo "$list" | grep .)" ]; then
    echo "  unchanged  $name.m3u ($count discs)"
    kept=$((kept + 1)); continue
  fi
  if [ -e "$out" ]; then
    cp -f "$out" "$BAK/$name.m3u.$(date '+%m%d-%H%M')"
    saved=$((saved + 1))
    echo "$(date '+%F %T')  backed up $name.m3u" >> "$LOG"
  fi
  echo "$list" | grep . > "$out"
  echo "  written    $name.m3u ($count discs)"
  echo "$(date '+%F %T')  wrote $name.m3u ($count discs)" >> "$LOG"
  made=$((made + 1))
done <<< "$titles"

sync
echo
echo "==================================================="
echo "  $made written, $kept already correct"
[ "$saved" -gt 0 ] && echo "  $saved existing playlist(s) backed up to:" && echo "    $BAK"
echo "==================================================="
echo
echo "Single-disc games are left alone on purpose: a playlist"
echo "for one would make that game appear twice in the list."
echo
echo "Press any key to close."
read -n 1 -s -r
