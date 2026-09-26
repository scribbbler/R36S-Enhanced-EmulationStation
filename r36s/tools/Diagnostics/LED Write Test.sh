#!/bin/bash
# Actively test what the power LEDs do when we write to them directly.
# Watch the physical LED while this runs (it pauses between steps), then
# read the results file on the card. USB-C should be UNPLUGGED.

OUT="/roms/tools/es-custom/led-test.txt"
BLUE=/sys/class/leds/led-blue
RED=/sys/class/leds/led-red

rd() { cat "$1/brightness" 2>/dev/null; }
w()  { sudo sh -c "echo $2 > $1/brightness" 2>/dev/null; }
wt() { sudo sh -c "echo $2 > $1/trigger" 2>/dev/null; }

sudo mount -o remount,rw / 2>/dev/null

{
  echo "==== LED write test $(date '+%F %T') ===="
  echo "initial: blue=$(rd $BLUE) red=$(rd $RED)"
  echo

  echo "[1] set trigger=none on both"
  wt $BLUE none; wt $RED none
  echo "    blue=$(rd $BLUE) red=$(rd $RED)"

  echo "[2] blue OFF (echo 0), red OFF (echo 0)  -- expect both dark"
  w $BLUE 0; w $RED 0; sleep 2
  echo "    blue=$(rd $BLUE) red=$(rd $RED)   <-- LOOK: are both LEDs off?"

  echo "[3] blue ON (echo 1)  -- expect blue lit"
  w $BLUE 1; sleep 2
  echo "    blue=$(rd $BLUE) red=$(rd $RED)   <-- LOOK: is blue on?"

  echo "[4] blue OFF, red ON (echo 1)  -- expect only red"
  w $BLUE 0; w $RED 1; sleep 2
  echo "    blue=$(rd $BLUE) red=$(rd $RED)   <-- LOOK: only red on?"

  echo "[5] try max value 255 on blue (in case driver wants raw)"
  w $BLUE 255; sleep 1
  echo "    blue=$(rd $BLUE)"

  echo "[6] restore: blue off, red off"
  w $BLUE 0; w $RED 0
  echo "    blue=$(rd $BLUE) red=$(rd $RED)"

  echo
  echo "Note which steps matched the physical LED so we know the real on/off values."
} > "$OUT" 2>&1
sync
echo "LED test written to $OUT"
exit 0
