#!/bin/bash
# NAME: Restore Gamelists
#
# Undo "Rebuild Gamelists.sh" by putting back the gamelist.xml files it saved.
# Only systems that had a gamelist before the rebuild are restored; a system
# whose list the rebuild created from nothing keeps the generated one, because
# there is no earlier version to return to.

BACKUPS="/roms/tools/es-custom/gamelist-backup"
# Each rebuild saves into its own dated folder; the newest is the one to undo.
BACKUP="$(ls -1d "$BACKUPS"/*/ 2>/dev/null | sort | tail -1)"
LOG="/roms/tools/es-custom/gamelist.log"

mkdir -p "$(dirname "$LOG")" 2>/dev/null
printf "\033c" >> /dev/tty1

if [ -z "$BACKUP" ] || [ ! -d "$BACKUP" ] || [ -z "$(ls -A "$BACKUP" 2>/dev/null)" ]; then
  printf "No backup found under %s - nothing to restore.\n" "$BACKUPS" >> /dev/tty1
  echo "$(date '+%F %T')  restore: no backup present" >> "$LOG" 2>/dev/null
  sleep 6
  exit 1
fi

echo "==== restore run $(date '+%F %T') ====" >> "$LOG" 2>/dev/null
printf "Restoring gamelists from %s\n\n" "$(basename "$BACKUP")" >> /dev/tty1

restored=0
for f in "$BACKUP"/*; do
  [ -f "$f" ] || continue
  # saved as a path relative to /roms with "/" written as "__"
  rel=$(basename "$f" | sed 's|__|/|g')
  dst="/roms/$rel"
  if [ ! -d "$(dirname "$dst")" ]; then
    printf "  skipped (system gone): %s\n" "$rel" >> /dev/tty1
    continue
  fi
  cp -f "$f" "$dst"
  printf "  restored: %s\n" "$rel" >> /dev/tty1
  echo "$(date '+%F %T')  restored $rel" >> "$LOG" 2>/dev/null
  restored=$((restored + 1))
done

sync
printf "\n%s restored.\nEmulationStation will now restart.\n" "$restored" >> /dev/tty1
echo "$(date '+%F %T')  restore done: $restored" >> "$LOG" 2>/dev/null
sleep 5
printf "\033c" >> /dev/tty1

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
