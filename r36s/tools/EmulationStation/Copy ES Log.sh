#!/bin/bash
# Copy EmulationStation logs and settings to the card for diagnosis

OUT="/roms/tools/es-custom"
mkdir -p "$OUT"

for F in /home/ark/.emulationstation/es_log.txt /home/ark/.emulationstation/es_log.txt.bak /home/ark/.emulationstation/es_settings.cfg; do
  [ -f "$F" ] && cp -f "$F" "$OUT/$(basename $F)"
done
sync
echo "logs copied to $OUT"
exit 0
