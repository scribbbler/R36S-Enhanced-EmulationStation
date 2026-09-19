#!/bin/bash
# Install custom EmulationStation build (with auto-restore failsafe)
# Custom binary expected at: /roms/tools/es-custom/emulationstation.custom

ESDIR="/usr/bin/emulationstation"
SRC="/roms/tools/es-custom"
LOG="$SRC/install.log"

log() { echo "$(date '+%F %T')  $1" >> "$LOG"; }

sudo mount -o remount,rw / 2>/dev/null
echo "==== install run ====" >> "$LOG"

# 0) sanity: custom binary present and its libraries resolve
if [ ! -f "$SRC/emulationstation.custom" ]; then
  log "ERROR: $SRC/emulationstation.custom not found"
  exit 1
fi
MISSING=$(ldd "$SRC/emulationstation.custom" 2>/dev/null | grep -c "not found")
if [ "$MISSING" -gt 0 ]; then
  log "ERROR: custom binary has $MISSING unresolved libraries:"
  ldd "$SRC/emulationstation.custom" | grep "not found" >> "$LOG"
  exit 1
fi
log "custom binary OK (all libraries resolve)"

# 1) one-time backup of the stock binary
if [ ! -f "$ESDIR/emulationstation.stock" ]; then
  sudo cp -f "$ESDIR/emulationstation" "$ESDIR/emulationstation.stock"
  log "stock binary backed up to emulationstation.stock"
else
  log "stock backup already exists, keeping it"
fi

# 2) launcher wrappers: back up once, then (re)apply a CLEAN crash failsafe.
#    We restore from .orig first so any earlier (buggy) failsafe is wiped and
#    replaced, making this idempotent and self-correcting.
#
#    The failsafe must fire ONLY on a genuine crash, never on an intentional
#    restart/shutdown. ES touches /tmp/es-restart|es-sysrestart|es-shutdown on
#    intentional exits (and the installer touches es-restart before killing ES);
#    a real crash leaves none of those. So: exit>=128 AND no intent-flag AND
#    within 15s of launch => crash => restore stock.
for W in emulationstation.sh emulationstation.sh.es; do
  if [ ! -f "$ESDIR/$W.orig" ]; then
    sudo cp -f "$ESDIR/$W" "$ESDIR/$W.orig"
    log "$W backed up to $W.orig"
  else
    sudo cp -f "$ESDIR/$W.orig" "$ESDIR/$W"
    log "$W restored from .orig before re-patching"
  fi
  sudo sed -i 's|^      "\$esdir/emulationstation" "\$@"$|      es_start=$(date +%s) # es-custom-failsafe\n      echo "=== ES launch $(date "+%F %T") ===" >> /roms/tools/es-custom/es-stderr.log\n      "$esdir/emulationstation" "$@" 2>> /roms/tools/es-custom/es-stderr.log|' "$ESDIR/$W"
  sudo sed -i 's|^      ret=\$[?]$|      ret=$?\n      if [ -f "$esdir/emulationstation.stock" ] \&\& [ $ret -ge 128 ] \&\& [ ! -f /tmp/es-restart ] \&\& [ ! -f /tmp/es-sysrestart ] \&\& [ ! -f /tmp/es-shutdown ] \&\& [ $(($(date +%s) - es_start)) -lt 15 ]; then sudo mount -o remount,rw / 2>/dev/null; sudo cp -f "$esdir/emulationstation.stock" "$esdir/emulationstation"; sync; continue; fi # es-custom-failsafe|' "$ESDIR/$W"
  if grep -q "es-custom-failsafe" "$ESDIR/$W"; then
    log "clean failsafe applied to $W"
  else
    log "WARNING: failsafe patch did not apply to $W (unexpected wrapper content)"
  fi
done

# 3) install the custom binary
sudo cp -f "$SRC/emulationstation.custom" "$ESDIR/emulationstation"
sudo chmod 755 "$ESDIR/emulationstation"
sync
log "custom binary installed"
log "SUCCESS - restarting EmulationStation"

# 4) restart ES cleanly via its own respawn loop
touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
