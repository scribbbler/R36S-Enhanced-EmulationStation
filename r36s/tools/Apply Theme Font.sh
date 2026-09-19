#!/bin/bash
# Replace EmulationStation's stock default font (OpenSans Hebrew Condensed)
# with the Super Min UI theme font (BPreplay), so unthemed UI text -- most
# importantly the battery % -- matches the theme. No ES rebuild needed:
# ES loads :/ fonts from this resources dir at runtime.
#
# Reversible via "Restore Stock Font.sh".

RES="/usr/bin/emulationstation/resources"
FONTS="/roms/tools/es-custom-fonts"
LOG="/roms/tools/es-custom/font.log"

log() { echo "$(date '+%F %T')  $1" >> "$LOG"; }

sudo mount -o remount,rw / 2>/dev/null
echo "==== apply theme font ====" >> "$LOG"

if [ ! -f "$FONTS/BPreplay-Regular.otf" ]; then
  log "ERROR: $FONTS/BPreplay-Regular.otf not found on card"
  exit 1
fi

# The stock default fonts we override (regular = battery %, menus; light = small text)
for f in opensans_hebrew_condensed_regular.ttf opensans_hebrew_condensed_light.ttf; do
  if [ -f "$RES/$f" ]; then
    # one-time backup of the genuine stock font
    if [ ! -f "$RES/$f.stock" ]; then
      sudo cp -f "$RES/$f" "$RES/$f.stock"
      log "backed up stock $f -> $f.stock"
    fi
    sudo cp -f "$FONTS/BPreplay-Regular.otf" "$RES/$f"
    log "replaced $f with BPreplay-Regular"
  fi
done

sync
log "SUCCESS - restart ES to see the new font"

# restart ES cleanly so fonts reload
touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
