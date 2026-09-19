#!/bin/bash
# LED test v2: the brightness node ignored writes, so test this driver's REAL
# control surface -- kernel triggers and battery_threshold. Watch the LED.
# USB-C UNPLUGGED.

OUT="/roms/tools/es-custom/led-test2.txt"
BLUE=/sys/class/leds/led-blue
RED=/sys/class/leds/led-red

rd()  { echo "b=$(cat $BLUE/brightness 2>/dev/null) trg=$(cat $BLUE/trigger 2>/dev/null | grep -oE '\[[a-z-]+\]')"; }
wr()  { sudo sh -c "echo $2 > $1" 2>&1; }   # capture any error

sudo mount -o remount,rw / 2>/dev/null
{
  echo "==== LED test v2 $(date '+%F %T') ===="
  echo "initial: $(rd)"
  echo

  echo "[A] DECISIVE: set blue trigger = heartbeat  (LED should BLINK if node works)"
  echo "    write result: $(wr $BLUE/trigger heartbeat)"
  echo "    state: $(rd)"
  echo "    >>> WATCH ~4s: is the blue LED BLINKING? <<<"
  sleep 4

  echo "[B] set blue trigger = none, then brightness 0 (should go dark if works)"
  wr $BLUE/trigger none >/dev/null
  echo "    write result: $(wr $BLUE/brightness 0)"
  echo "    state: $(rd)"
  echo "    >>> WATCH ~3s: is blue OFF now? <<<"
  sleep 3

  echo "[C] set blue battery_threshold = 100 (driver may drive LED by battery%)"
  echo "    write result: $(wr $BLUE/battery_threshold 100)"
  echo "    threshold now: $(cat $BLUE/battery_threshold 2>/dev/null)"
  sleep 2

  echo "[D] set blue battery_threshold = 0"
  wr $BLUE/battery_threshold 0 >/dev/null
  echo "    threshold now: $(cat $BLUE/battery_threshold 2>/dev/null)"

  echo "[E] try RED trigger = heartbeat (does RED blink?)"
  echo "    write result: $(wr $RED/trigger heartbeat)"
  sleep 4
  wr $RED/trigger none >/dev/null
  wr $RED/brightness 0 >/dev/null

  echo
  echo "[F] restore blue to default-on look: trigger none"
  wr $BLUE/trigger none >/dev/null
  echo "    final: $(rd)"
  echo
  echo "KEY QUESTIONS: In [A] did BLUE blink? In [E] did RED blink? In [B] did blue turn off?"
} > "$OUT" 2>&1
sync
echo "written to $OUT"
exit 0
