#!/bin/bash
# Dump everything about the device's power-supply sysfs so we can write
# correct charging-detection logic. Writes to the card; no ES rebuild needed.

OUT="/roms/tools/es-custom/battery-probe.txt"
{
  echo "==== battery probe $(date '+%F %T') ===="
  echo "device charging state RIGHT NOW: (make sure USB-C is UNPLUGGED when you run this)"
  echo
  for ps in /sys/class/power_supply/*; do
    echo "---- $ps ----"
    for f in type status present online capacity capacity_level charge_type health voltage_now current_now; do
      [ -f "$ps/$f" ] && printf "  %-16s = %s\n" "$f" "$(cat "$ps/$f" 2>/dev/null)"
    done
    if [ -f "$ps/uevent" ]; then
      echo "  uevent:"
      sed 's/^/    /' "$ps/uevent"
    fi
    echo
  done
} > "$OUT" 2>&1
sync
echo "battery probe written to $OUT"
exit 0
