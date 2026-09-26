#!/bin/bash
# Undo "Apply Theme Font.sh" -- restore the genuine stock default fonts.

RES="/usr/bin/emulationstation/resources"
LOG="/roms/tools/es-custom/font.log"

log() { echo "$(date '+%F %T')  $1" >> "$LOG"; }

sudo mount -o remount,rw / 2>/dev/null
echo "==== restore stock font ====" >> "$LOG"

restored=0
for f in opensans_hebrew_condensed_regular.ttf opensans_hebrew_condensed_light.ttf; do
  if [ -f "$RES/$f.stock" ]; then
    sudo cp -f "$RES/$f.stock" "$RES/$f"
    log "restored $f from $f.stock"
    restored=1
  fi
done

[ "$restored" = 0 ] && log "nothing to restore (no .stock backups found)"
sync
log "restart ES to see stock font"

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
