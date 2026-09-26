#!/bin/bash
# Hide the stock ArkOS PS1 m3u tools from the Options menu.
#
# All three are unsafe on a card with hand-made multi-disc playlists:
#
#   PS1 - Generate m3u files   opens with `rm -f /roms/psx/*.m3u`, so it
#                              destroys existing playlists before it starts.
#                              It then writes one playlist per file rather
#                              than per disc group, never descends into
#                              dotfolders (`*/` skips them), and - because
#                              it has no `cd || continue` guard - scatters
#                              broken playlists across the card root when
#                              psx/ has no visible subfolder.
#   PS1 - Delete m3u files     is that same rm with nothing else around it.
#   PS1 - Show only m3u games  rewrites es_systems.cfg so PSX lists .m3u
#                              alone, hiding every single-disc .chd.
#
# Use "PS1 - Build Multi-Disc Playlists" and "PS1 - Remove Stray Playlists"
# in the Tools system instead.
#
# Nothing is destroyed. The scripts move to a backup folder on the SD card,
# where they stay readable from a desktop machine, and "Restore Stock m3u
# Tools.sh" puts them back.
#
# The backups keep a .sh.bak extension, not .sh. The Options menu is built by
# walking /opt/system for *.sh, and /opt/system/Tools is /roms/tools - so a
# backup ending in .sh would come straight back as a submenu entry, and the
# search below would find its own output on the next run.

SRC="/opt/system"
BACKUP="/roms/tools/stock-options-backup"
LOG="$BACKUP/hide.log"

TARGETS=(
  "PS1 - Generate m3u files.sh"
  "PS1 - Delete m3u files.sh"
  "PS1 - Show only m3u games.sh"
)

mkdir -p "$BACKUP"
log() { mkdir -p "$(dirname "$LOG")" 2>/dev/null; echo "$(date '+%F %T')  $1" >> "$LOG" 2>/dev/null; }

sudo mount -o remount,rw / 2>/dev/null
log "==== hide run ===="
printf "\033c" >> /dev/tty1
printf "Hiding stock PS1 m3u tools from Options\n\n" >> /dev/tty1

moved=0
absent=0
for name in "${TARGETS[@]}"; do
  # search the whole tree: ES lists any .sh under /opt/system, subfolders included
  found=$(find "$SRC" -type f -name "$name" -not -path "$BACKUP/*" 2>/dev/null | head -1)
  if [ -z "$found" ]; then
    printf "  already hidden: %s\n" "$name" >> /dev/tty1
    log "not present, nothing to do: $name"
    absent=$((absent + 1))
    continue
  fi

  # copy first and confirm it landed before removing the original
  if ! sudo cp -f "$found" "$BACKUP/$name.bak"; then
    printf "  FAILED to back up: %s (left in place)\n" "$name" >> /dev/tty1
    log "ERROR: could not copy $found to backup - original left alone"
    continue
  fi
  if [ ! -s "$BACKUP/$name.bak" ]; then
    printf "  FAILED to back up: %s (left in place)\n" "$name" >> /dev/tty1
    log "ERROR: backup of $name is empty - original left alone"
    continue
  fi

  sudo rm -f "$found"
  printf "  hidden: %s\n" "$name" >> /dev/tty1
  log "moved $found -> $BACKUP/$name.bak"
  moved=$((moved + 1))
done

# "PS1 - Show all games.sh" is the repair script for a card whose
# es_systems.cfg has already been rewritten. Never hide it.
if [ -n "$(find "$SRC" -type f -name 'PS1 - Show all games.sh' 2>/dev/null | head -1)" ]; then
  printf "\n  kept: PS1 - Show all games (it is the repair tool)\n" >> /dev/tty1
  log "kept PS1 - Show all games.sh deliberately - it undoes Show only m3u games"
fi

sync
log "done: $moved moved, $absent already absent"
printf "\n%s moved, %s already hidden.\nBackup: %s\n" "$moved" "$absent" "$BACKUP" >> /dev/tty1
printf "\nEmulationStation will now restart.\n" >> /dev/tty1
sleep 4
printf "\033c" >> /dev/tty1

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
