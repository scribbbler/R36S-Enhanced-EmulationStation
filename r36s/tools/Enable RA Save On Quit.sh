#!/bin/bash
# Set "Save Configuration on Quit" directly in both RetroArch configs
# (the RA menu refuses to save the global config while a per-core override
# is loaded, which on ArkOS is most of the time).

for base in /home/ark/.config/retroarch/retroarch.cfg /home/ark/.config/retroarch32/retroarch.cfg; do
  [ -f "$base" ] || continue
  sed -i '/^config_save_on_exit =/d' "$base"
  echo 'config_save_on_exit = "true"' >> "$base"
  chown ark:ark "$base" 2>/dev/null
  echo "enabled in $base"
done
sync
echo "Done. RetroArch will save settings on quit."
echo "Note: for a system whose core has an override file, RA still saves"
echo "those tweaks via Quick Menu > Overrides > Save Core Overrides instead."
