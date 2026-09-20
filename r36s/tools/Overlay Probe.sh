#!/bin/bash
# Probe the RetroArch layout so we can wire up Jeltron overlays for the
# handhelds (gb / gbc / gba / ngp). Read-only. Writes a report to the card.

OUT="/roms/tools/es-custom/overlay-probe.txt"
mkdir -p /roms/tools/es-custom
{
  echo "==================================================================="
  echo "  OVERLAY / RETROARCH PROBE   $(date '+%F %T')"
  echo "==================================================================="

  echo
  echo "########## retroarch.cfg locations ##########"
  find /home /opt /usr/local /etc -maxdepth 5 -name 'retroarch.cfg' 2>/dev/null

  RACFG=""
  for c in /home/ark/.config/retroarch/retroarch.cfg /opt/retroarch/retroarch.cfg; do
    [ -f "$c" ] && RACFG="$c" && break
  done
  [ -z "$RACFG" ] && RACFG="$(find /home/ark -name retroarch.cfg 2>/dev/null | head -1)"
  echo "USING: $RACFG"

  echo
  echo "########## key directory settings ##########"
  grep -E '^(config_directory|overlay_directory|rgui_config_directory|libretro_directory|system_directory|savestate_directory|core_options_path|joypad_autoconfig_dir) ' "$RACFG" 2>/dev/null

  echo
  echo "########## current overlay settings in base cfg ##########"
  grep -E '^(input_overlay|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport)' "$RACFG" 2>/dev/null

  # resolve overlay_directory
  OVDIR="$(grep -E '^overlay_directory ' "$RACFG" 2>/dev/null | sed 's/.*= *"//;s/"$//')"
  echo
  echo "########## overlay_directory = $OVDIR ##########"
  ls -la "$OVDIR" 2>/dev/null | head -40

  # resolve config_directory (per-core overrides live here)
  CFGDIR="$(grep -E '^rgui_config_directory ' "$RACFG" 2>/dev/null | sed 's/.*= *"//;s/"$//')"
  [ -z "$CFGDIR" ] && CFGDIR="$(dirname "$RACFG")/config"
  echo
  echo "########## override config dir = $CFGDIR (subfolders = core names) ##########"
  ls -la "$CFGDIR" 2>/dev/null | head -60

  echo
  echo "########## installed libretro cores (.so) ##########"
  for d in /home/ark/.config/retroarch/cores /usr/lib/libretro /opt/retroarch/cores /usr/local/lib/libretro; do
    [ -d "$d" ] && echo "-- $d --" && ls -1 "$d"/*.so 2>/dev/null | xargs -n1 basename 2>/dev/null | grep -iE 'gambatte|gpsp|mgba|vba|gambatte|beetle.*gba|mednafen.*ngp|race|tgbdual'
  done

  echo
  echo "########## how ES launches gb / gbc / gba / ngp ##########"
  for sys in gb gbc gba ngp ngpc; do
    echo "---- system: $sys ----"
    grep -A2 "name>$sys<" /etc/emulationstation/es_systems.cfg 2>/dev/null | grep -E 'command|path' | head -3
    grep -A6 "\"$sys\"" /etc/emulationstation/es_systems.cfg 2>/dev/null | grep -iE 'command|core|libretro|gptokeyb|retroarch' | head -3
    # ArkOS per-system chosen emulator
    ls /roms/$sys/ 2>/dev/null | grep -iE '\.opt$|\.cfg$|\.emu$' | head
  done

  echo
  echo "########## ArkOS launch script(s) ##########"
  find /opt /usr/local/bin -maxdepth 3 -iname '*.sh' 2>/dev/null | xargs grep -lE 'appendconfig|input_overlay|retroarch' 2>/dev/null | head -10

  echo
  echo "########## all ES systems + launch command (full es_systems.cfg copied too) ##########"
  ESS=/etc/emulationstation/es_systems.cfg
  [ -f "$ESS" ] || ESS="$(find /etc /opt /home -name es_systems.cfg 2>/dev/null | head -1)"
  echo "es_systems.cfg: $ESS"
  cp "$ESS" /roms/tools/es-custom/es_systems.cfg 2>/dev/null && echo "(copied to /roms/tools/es-custom/es_systems.cfg)"
  # name + the core/command per system
  awk '/<name>/{n=$0} /<command>/{print n"  =>  "$0}' "$ESS" 2>/dev/null | sed 's/<[^>]*>//g' | head -80

  echo
  echo "########## ArkOS per-system emulator selection ##########"
  # ArkOS stores the chosen core per system; find where
  find /roms -maxdepth 2 -iname '*.cfg' 2>/dev/null | grep -iE '/(gb|gbc|gba|snes|nes|genesis|psx|ngp)/' | head
  ls -la /opt/system/configs 2>/dev/null | head
  find /opt -maxdepth 3 -iname '*.cfg' 2>/dev/null | grep -iE 'emu|core|system' | head

  echo
  echo "########## file counts per system (which we actually have) ##########"
  for sys in $(ls /roms 2>/dev/null); do
    [ -d "/roms/$sys" ] || continue
    n=$(ls "/roms/$sys" 2>/dev/null | grep -viE '^\.|gamelist|images|media|videos|manuals' | wc -l | tr -d ' ')
    [ "$n" -gt 0 ] && printf "  %-14s %s\n" "$sys" "$n"
  done

  echo
  echo "==================================================================="
  echo "  DONE -> $OUT"
  echo "==================================================================="
} > "$OUT" 2>&1
sync
echo "Overlay probe written to $OUT"
