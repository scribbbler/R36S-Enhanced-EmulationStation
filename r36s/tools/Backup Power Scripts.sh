#!/bin/bash
# Back up the live power-button scripts before enabling ArkOS Quick Mode.
#
# Quick Mode's Enable swaps finish.sh/pause.sh for its .qm variants, and its
# Disable restores them from .orig -- but on ArkOS4Clone the live scripts are
# NEITHER of those. They carry an extra "systemctl stop emulationstation" that
# flushes gamelist.xml (playcount/lastplayed) before power-off, and both the
# .qm and .orig variants drop it. So Disable cannot put things back.
#
# Run this first. "Restore Power Scripts.sh" puts the originals back.

BAK="/roms/tools/es-custom/power-scripts-backup"
LOG="/roms/tools/es-custom/power-scripts.log"

sudo mount -o remount,rw / 2>/dev/null

mkdir -p "$BAK"
{
  echo "==== backup run $(date '+%F %T') ===="
} >> "$LOG"

echo
echo "Backing up the live power-button scripts..."
echo

COUNT=0
for base in finish pause; do
  SRC="/usr/local/bin/$base.sh"
  DST="$BAK/$base.sh"

  if [ ! -e "$SRC" ]; then
    echo "  MISSING  $SRC -- nothing to back up"
    echo "$(date '+%F %T')  MISSING $SRC" >> "$LOG"
    continue
  fi

  if [ -e "$DST" ]; then
    if cmp -s "$SRC" "$DST"; then
      echo "  already backed up: $base.sh (unchanged)"
      echo "$(date '+%F %T')  $base.sh unchanged, kept existing backup" >> "$LOG"
      COUNT=$((COUNT + 1))
      continue
    fi
    # Never overwrite a good backup with a Quick-Mode-modified script.
    if grep -q "quickmode.sh" "$SRC"; then
      echo "  SKIPPED  $base.sh currently holds the Quick Mode version;"
      echo "           keeping the earlier backup so it is not lost."
      echo "$(date '+%F %T')  $base.sh is .qm version, backup preserved" >> "$LOG"
      COUNT=$((COUNT + 1))
      continue
    fi
    cp -f "$DST" "$DST.prev-$(date '+%m%d-%H%M')"
  fi

  cp -f "$SRC" "$DST"
  echo "  backed up: $base.sh ($(stat -c%s "$SRC") bytes)"
  echo "$(date '+%F %T')  backed up $base.sh" >> "$LOG"
  COUNT=$((COUNT + 1))
done

sync

echo
echo "==================================================="
if [ "$COUNT" -eq 2 ]; then
  echo "  BACKUP COMPLETE"
  echo "==================================================="
  echo
  echo "  Saved to: $BAK"
  echo
  echo "  You can now enable Quick Mode from"
  echo "  Options -> Advanced -> Enable Quick Mode."
  echo
  echo "  To undo fully afterwards:"
  echo "    1. Options -> Advanced -> Disable Quick Mode"
  echo "    2. Options -> Tools -> Restore Power Scripts"
  echo
  echo "  Step 2 matters: Disable restores an older ArkOS"
  echo "  variant, not the scripts you have today."
else
  echo "  BACKUP INCOMPLETE -- do not enable Quick Mode"
  echo "==================================================="
  echo
  echo "  Only $COUNT of 2 scripts were backed up."
fi

echo
echo "Press any key to close."
read -n 1 -s -r
