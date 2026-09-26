#!/bin/bash
# Put the stock ArkOS PS1 m3u tools back into the Options menu.
#
# Undoes "Hide Stock m3u Tools.sh". The scripts come back from the SD card
# backup, where they are held as .sh.bak so the Options menu does not list
# them; the exec bit is reapplied because exFAT cannot carry it.
#
# They are still the tools that wipe /roms/psx/*.m3u without asking - restore
# them only if you know why you want them.

SRC="/opt/system"
BACKUP="/roms/tools/stock-options-backup"
LOG="$BACKUP/hide.log"

log() { mkdir -p "$(dirname "$LOG")" 2>/dev/null; echo "$(date '+%F %T')  $1" >> "$LOG" 2>/dev/null; }

sudo mount -o remount,rw / 2>/dev/null
printf "\033c" >> /dev/tty1

if [ ! -d "$BACKUP" ] || [ -z "$(ls -A "$BACKUP"/*.sh.bak 2>/dev/null)" ]; then
  printf "No backup found at %s - nothing to restore.\n" "$BACKUP" >> /dev/tty1
  log "ERROR: no backup present, nothing to restore"
  sleep 5
  exit 1
fi

log "==== restore run ===="
printf "Restoring stock PS1 m3u tools to Options\n\n" >> /dev/tty1

restored=0
for f in "$BACKUP"/*.sh.bak; do
  name=$(basename "$f" .bak)          # stored as .sh.bak so the menu ignores it
  sudo cp -f "$f" "$SRC/$name"
  sudo chmod 755 "$SRC/$name"        # exFAT drops the exec bit
  printf "  restored: %s\n" "$name" >> /dev/tty1
  log "restored $name to $SRC"
  restored=$((restored + 1))
done

sync
log "done: $restored restored"
printf "\n%s restored.\nEmulationStation will now restart.\n" "$restored" >> /dev/tty1
sleep 4
printf "\033c" >> /dev/tty1

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
