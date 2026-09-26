#!/bin/bash
# Comprehensive hardware/state probe for R36S customization work.
# Dumps LEDs, battery, charging, device detect, and which ES is running.
# Runs on-device; writes everything to the card. No ES rebuild needed.

OUT="/roms/tools/es-custom/hardware-probe.txt"
{
  echo "==================================================================="
  echo "  HARDWARE PROBE  $(date '+%F %T')"
  echo "  (Run this with USB-C UNPLUGGED so charging state reads correctly)"
  echo "==================================================================="

  echo
  echo "########## RUNNING ES BINARY ##########"
  ES=/usr/bin/emulationstation/emulationstation
  echo "md5:     $(md5sum $ES 2>/dev/null | cut -d' ' -f1)"
  echo "stock:   $(md5sum $ES.stock 2>/dev/null | cut -d' ' -f1)"
  echo "custom:  $(md5sum /roms/tools/es-custom/emulationstation.custom 2>/dev/null | cut -d' ' -f1)"

  echo
  echo "########## DEVICE DETECT ##########"
  /usr/local/bin/console_detect -s 2>/dev/null || echo "(console_detect not available)"
  echo "/boot/.console: $(cat /boot/.console 2>/dev/null)"

  echo
  echo "########## ALL LEDs (/sys/class/leds) ##########"
  for led in /sys/class/leds/*; do
    [ -e "$led" ] || continue
    echo "---- $(basename "$led") ----"
    for attr in brightness max_brightness trigger battery_threshold mode; do
      if [ -f "$led/$attr" ]; then
        printf "  %-18s = %s\n" "$attr" "$(cat "$led/$attr" 2>/dev/null | tr '\n' ' ')"
      fi
    done
  done

  echo
  echo "########## POWER SUPPLY / BATTERY ##########"
  for ps in /sys/class/power_supply/*; do
    [ -e "$ps" ] || continue
    echo "---- $(basename "$ps") ----"
    if [ -f "$ps/uevent" ]; then sed 's/^/  /' "$ps/uevent"; fi
  done

  echo
  echo "########## ES SETTINGS (led / battery / theme) ##########"
  grep -iE "PowerLed|JoyLed|ShowBattery|ThemeSet|subset" /home/ark/.emulationstation/es_settings.cfg 2>/dev/null || echo "(none found)"

  echo
  echo "########## es-stderr.log (crash output, if any) ##########"
  tail -40 /roms/tools/es-custom/es-stderr.log 2>/dev/null || echo "(no es-stderr.log)"

} > "$OUT" 2>&1

# Also grab the ES logs
cp -f /home/ark/.emulationstation/es_log.txt      /roms/tools/es-custom/ 2>/dev/null
cp -f /home/ark/.emulationstation/es_log.txt.bak  /roms/tools/es-custom/ 2>/dev/null
sync
echo "hardware probe written to $OUT"
exit 0
