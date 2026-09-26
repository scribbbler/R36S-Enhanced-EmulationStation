#!/bin/bash
# Put the live power-button scripts back after testing ArkOS Quick Mode.
#
# Run this AFTER "Disable Quick Mode", because Disable restores finish.sh and
# pause.sh from base-ArkOS .orig copies that are missing ArkOS4Clone's
# "systemctl stop emulationstation" line -- the one that flushes gamelist.xml
# (playcount/lastplayed) before the device powers off.

BAK="/roms/tools/es-custom/power-scripts-backup"
LOG="/roms/tools/es-custom/power-scripts.log"

sudo mount -o remount,rw / 2>/dev/null

echo
if [ ! -d "$BAK" ]; then
  echo "No backup found at:"
  echo "  $BAK"
  echo
  echo "Run \"Backup Power Scripts\" before enabling Quick Mode."
  echo
  echo "Press any key to close."
  read -n 1 -s -r
  exit 1
fi

echo "==== restore run $(date '+%F %T') ====" >> "$LOG"

echo "Restoring the power-button scripts..."
echo

STILL_QM=0
COUNT=0
for base in finish pause; do
  SRC="$BAK/$base.sh.bak"
  DST="/usr/local/bin/$base.sh"

  if [ ! -e "$SRC" ]; then
    echo "  MISSING  no backup of $base.sh"
    echo "$(date '+%F %T')  no backup for $base.sh" >> "$LOG"
    continue
  fi

  if cmp -s "$SRC" "$DST"; then
    echo "  already correct: $base.sh"
    echo "$(date '+%F %T')  $base.sh already matches backup" >> "$LOG"
    COUNT=$((COUNT + 1))
    continue
  fi

  grep -q "quickmode.sh" "$DST" 2>/dev/null && STILL_QM=1

  sudo cp -f "$SRC" "$DST"
  sudo chmod 755 "$DST"
  echo "  restored: $base.sh"
  echo "$(date '+%F %T')  restored $base.sh" >> "$LOG"
  COUNT=$((COUNT + 1))
done

sync

echo
echo "==================================================="
if [ "$COUNT" -eq 2 ]; then
  echo "  RESTORE COMPLETE"
else
  echo "  RESTORE INCOMPLETE ($COUNT of 2)"
fi
echo "==================================================="

if [ "$STILL_QM" -eq 1 ]; then
  echo
  echo "  NOTE: a script still contained the Quick Mode"
  echo "  version, which means Quick Mode had not been"
  echo "  disabled first. Run:"
  echo "    Options -> Advanced -> Disable Quick Mode"
  echo "  then run this tool again to confirm."
fi

echo
echo "  Quick Mode is currently: "
if [ -e /usr/local/bin/quickmode.sh ]; then
  echo "    ENABLED (quickmode.sh still present)"
else
  echo "    not enabled"
fi

echo
echo "Press any key to close."
read -n 1 -s -r
