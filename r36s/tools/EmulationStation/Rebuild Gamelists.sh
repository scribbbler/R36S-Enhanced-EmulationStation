#!/bin/bash
# NAME: Rebuild Gamelists
#
# Make every system's gamelist.xml describe the games actually on the card:
# add the ones missing from it, drop entries whose file is gone, and keep all
# existing metadata. See rebuild-gamelists.py beside this file for detail.
#
# "Restore Gamelists.sh" puts the previous gamelists back.

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PY="$HERE/rebuild-gamelists.py"
LOG="/roms/tools/es-custom/gamelist.log"

mkdir -p "$(dirname "$LOG")" 2>/dev/null
printf "\033c" >> /dev/tty1

if ! command -v python3 >/dev/null 2>&1; then
  printf "python3 is not installed on this device.\nCannot rebuild gamelists.\n" >> /dev/tty1
  echo "$(date '+%F %T')  ERROR: no python3" >> "$LOG" 2>/dev/null
  sleep 6
  exit 1
fi

if [ ! -f "$PY" ]; then
  printf "Missing %s\n" "$PY" >> /dev/tty1
  echo "$(date '+%F %T')  ERROR: $PY not found" >> "$LOG" 2>/dev/null
  sleep 6
  exit 1
fi

{
  echo "==== rebuild run $(date '+%F %T') ===="
  python3 "$PY"
  rc=$?
  echo "==== exit $rc ===="
} 2>&1 | tee -a "$LOG" >> /dev/tty1

sync
printf "\nEmulationStation will now restart so it reloads the lists.\n" >> /dev/tty1
sleep 5
printf "\033c" >> /dev/tty1

touch /tmp/es-restart
kill $(pidof emulationstation) 2>/dev/null
exit 0
