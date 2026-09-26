#!/bin/bash
# Restore the stock EmulationStation binary and original launcher wrappers

ESDIR="/usr/bin/emulationstation"
LOG="/roms/tools/es-custom/install.log"

log() { echo "$(date '+%F %T')  $1" >> "$LOG"; }

sudo mount -o remount,rw / 2>/dev/null
echo "==== restore run ====" >> "$LOG"

if [ ! -f "$ESDIR/emulationstation.stock" ]; then
  log "ERROR: no stock backup found at $ESDIR/emulationstation.stock - nothing to restore"
  exit 1
fi

sudo cp -f "$ESDIR/emulationstation.stock" "$ESDIR/emulationstation"
sudo chmod 755 "$ESDIR/emulationstation"
log "stock binary restored"

for W in emulationstation.sh emulationstation.sh.es; do
  if [ -f "$ESDIR/$W.orig" ]; then
    sudo cp -f "$ESDIR/$W.orig" "$ESDIR/$W"
    log "$W restored from $W.orig"
  fi
done

sync
log "RESTORE COMPLETE - restarting EmulationStation"

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
