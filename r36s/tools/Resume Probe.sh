#!/bin/bash
# Read-only probe for the 0.1 resume/continue work.
# Answers: is ArkOS Quick Mode present, what do the RetroArch hotkeys/save
# settings actually say, which emulator do the standalone systems resolve to,
# and is ES already writing the recovery XML that resume depends on.
# Changes nothing. Writes everything to the card.

OUT="/roms/tools/es-custom/resume-probe.txt"
{
  echo "==================================================================="
  echo "  RESUME PROBE  $(date '+%F %T')"
  echo "  Read-only. Nothing on the device is modified."
  echo "==================================================================="

  echo
  echo "########## 1. QUICK MODE PRESENT? ##########"
  for f in /usr/local/bin/quickmode.sh \
           /usr/local/bin/get_last_played.sh \
           /home/ark/.config/lastgame.sh \
           "/usr/local/bin/Enable Quick Mode.sh" \
           "/usr/local/bin/Disable Quick Mode.sh" \
           "/opt/system/Advanced/Enable Quick Mode.sh" \
           "/opt/system/Advanced/Disable Quick Mode.sh"; do
    if [ -e "$f" ]; then
      printf "  PRESENT  %-46s (%s bytes)\n" "$f" "$(stat -c%s "$f" 2>/dev/null)"
    else
      printf "  absent   %s\n" "$f"
    fi
  done
  echo
  echo "  -- anything Quick-Mode-shaped under /opt/system --"
  find /opt/system -iname "*quick*" 2>/dev/null | sed 's/^/    /' || true
  echo "  -- QBMODE flag (set while quickmode is shutting a game down) --"
  ls -la /dev/shm/QBMODE 2>/dev/null || echo "    not set (expected unless mid-shutdown)"

  echo
  echo "########## 2. RETROARCH SAVE/RESUME + HOTKEY SETTINGS ##########"
  for cfg in /home/ark/.config/retroarch/retroarch.cfg \
             /home/ark/.config/retroarch32/retroarch.cfg; do
    echo "---- $cfg ----"
    if [ -f "$cfg" ]; then
      grep -E '^[[:space:]]*(savestate_auto_save|savestate_auto_load|savestate_auto_index|network_cmd_enable|network_cmd_port|config_save_on_exit|quit_press_twice|confirm_quit|savestate_directory|savefile_directory|history_list_enable|content_history_path|content_history_size)[[:space:]]*=' "$cfg" | sed 's/^/    /'
      echo "    -- hotkey binds --"
      grep -E '^[[:space:]]*(input_enable_hotkey|input_exit_emulator|input_menu_toggle|input_save_state|input_load_state)(_btn|_axis)?[[:space:]]*=' "$cfg" | sed 's/^/    /'
    else
      echo "    (file not found)"
    fi
    echo
  done

  echo "########## 3. AUTO SAVESTATES ON DISK ##########"
  echo "  (*.state.auto proves the autosave path actually fires)"
  for d in /home/ark/.config/retroarch/states /home/ark/.config/retroarch32/states /roms/savestates; do
    [ -d "$d" ] && { echo "---- $d ----"; find "$d" -name "*.state.auto" -printf "    %TY-%Tm-%Td %TH:%TM  %s bytes  %p\n" 2>/dev/null | head -20; }
  done
  echo "  -- any .state.auto anywhere under /roms and ark config --"
  find /roms /home/ark/.config -name "*.state.auto" 2>/dev/null | head -20 | sed 's/^/    /'
  echo "  total found: $(find /roms /home/ark/.config -name "*.state.auto" 2>/dev/null | wc -l)"

  echo
  echo "########## 4. CONTENT HISTORY PLAYLISTS ##########"
  for lpl in /home/ark/.config/retroarch/content_history.lpl \
             /home/ark/.config/retroarch32/content_history.lpl; do
    if [ -f "$lpl" ]; then
      echo "---- $lpl ($(stat -c%s "$lpl") bytes) ----"
      head -24 "$lpl" | sed 's/^/    /'
    else
      echo "  absent   $lpl"
    fi
  done

  echo
  echo "########## 5. ES RECOVERY XML (what resume reads) ##########"
  REC=/home/ark/.emulationstation/recovery
  if [ -d "$REC" ]; then
    echo "  systems with recovery entries: $(find "$REC" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | wc -l)"
    echo "  total entries:                 $(find "$REC" -name "*.xml" 2>/dev/null | wc -l)"
    echo "  -- 10 most recent --"
    find "$REC" -name "*.xml" -printf "    %TY-%Tm-%Td %TH:%TM  %p\n" 2>/dev/null | sort -r | head -10
    echo "  -- newest entry, in full --"
    NEWEST=$(find "$REC" -name "*.xml" -printf "%T@ %p\n" 2>/dev/null | sort -rn | head -1 | cut -d' ' -f2-)
    [ -n "$NEWEST" ] && sed 's/^/    /' "$NEWEST"
  else
    echo "  $REC does not exist"
  fi

  echo
  echo "########## 6. COLLECTIONS: ARE RECENT/FAVORITES ENABLED? ##########"
  grep -E 'CollectionSystems(Auto|Custom)|StartupSystem|SaveGamelistsOnExit|SortAllSystems' \
    /home/ark/.emulationstation/es_settings.cfg 2>/dev/null | sed 's/^/    /' \
    || echo "    es_settings.cfg not readable"
  echo "  -- gamelists carrying lastplayed (the Recent data source) --"
  echo "     files with <lastplayed>: $(grep -rl "<lastplayed>" /roms/*/gamelist.xml 2>/dev/null | wc -l)"

  echo
  echo "########## 7. WHICH EMULATOR DO THE STANDALONE SYSTEMS USE? ##########"
  ESS=/etc/emulationstation/es_systems.cfg
  [ -f "$ESS" ] || ESS=$(find /etc /opt /home -name es_systems.cfg 2>/dev/null | head -1)
  echo "  es_systems.cfg: $ESS"
  for sys in nds psp n64 scummvm dreamcast pico-8 saturn; do
    echo "---- $sys ----"
    awk -v s="<name>$sys</name>" '
      /<system>/ {buf=""; hit=0}
      {buf = buf $0 "\n"; if (index($0, s)) hit=1}
      /<\/system>/ {if (hit) printf "%s", buf}
    ' "$ESS" 2>/dev/null | grep -E "<(command|emulators|emulator name|core)" | head -8 | sed 's/^/    /'
    echo "    per-system override in es_settings: $(grep -oE "\"$sys\.(emulator|core)\" value=\"[^\"]*\"" /home/ark/.emulationstation/es_settings.cfg 2>/dev/null | tr '\n' ' ')"
  done

  echo
  echo "########## 8. CONTROLS: DOES R3 EXIST? ##########"
  echo "  (Quick Mode's save-and-shutdown combo is R3 + Power)"
  for js in /dev/input/js*; do
    [ -e "$js" ] || continue
    echo "---- $js ----"
    udevadm info --query=property --name="$js" 2>/dev/null | grep -E "ID_INPUT|NAME" | sed 's/^/    /'
  done
  echo "  -- ES input config: buttons mapped --"
  grep -oE 'name="[a-z0-9]+"' /home/ark/.emulationstation/es_input.cfg 2>/dev/null | sort -u | tr '\n' ' ' | sed 's/^/    /'
  echo
  echo "  -- device detect (hotkey type) --"
  /usr/local/bin/console_detect -s 2>/dev/null | grep -iE "hotkey|joystick|device" | sed 's/^/    /'

  echo
  echo "  -- WHICH POWER GESTURE ACTUALLY POWERS OFF? --"
  echo "     finish.sh and pause.sh are mirror images gated on this flag."
  echo "     Only the POWEROFF path runs quickmode.sh; suspend never saves."
  if [ -e /home/ark/.config/.SWAPPOWERANDSUSPEND ]; then
    echo "     flag .SWAPPOWERANDSUSPEND: PRESENT (swapped)"
    echo "     => TAP the power button = power off  (this one saves)"
    echo "     => HOLD the power button = suspend   (does NOT save)"
  else
    echo "     flag .SWAPPOWERANDSUSPEND: absent (default)"
    echo "     => HOLD the power button = power off  (this one saves)"
    echo "     => TAP the power button  = suspend    (does NOT save)"
  fi
  echo "     (ES setting: Advanced Settings -> Switch Power Button Tap To Off)"

  echo
  echo "########## 9. IS QUICK MODE SAFE TO ENABLE AND UNDO? ##########"
  echo "  Enabling swaps the power-button scripts for .qm variants;"
  echo "  disabling restores them from .orig. Both must exist for a clean"
  echo "  round trip -- neither ships in the ArkOS4Clone overlay repo."
  MISSING=0
  for f in /usr/local/bin/finish.sh /usr/local/bin/finish.sh.qm /usr/local/bin/finish.sh.orig \
           /usr/local/bin/pause.sh  /usr/local/bin/pause.sh.qm  /usr/local/bin/pause.sh.orig \
           /usr/local/bin/buttonmon.sh; do
    if [ -e "$f" ]; then
      printf "  PRESENT  %-40s (%s bytes)\n" "$f" "$(stat -c%s "$f" 2>/dev/null)"
    else
      printf "  MISSING  %s\n" "$f"
      case "$f" in *.qm|*.orig) MISSING=1 ;; esac
    fi
  done
  echo
  echo "  -- do the live scripts actually MATCH their .orig? --"
  echo "     (existence is not enough: if finish.sh differs from finish.sh.orig,"
  echo "      disabling Quick Mode silently replaces it with something else)"
  DRIFT=0
  for base in finish pause; do
    live="/usr/local/bin/$base.sh"
    orig="/usr/local/bin/$base.sh.orig"
    if [ -e "$live" ] && [ -e "$orig" ]; then
      if cmp -s "$live" "$orig"; then
        printf "     %-10s live == .orig  (clean restore)\n" "$base.sh"
      else
        printf "     %-10s live != .orig  <-- DRIFT, restore would change behaviour\n" "$base.sh"
        DRIFT=1
      fi
    fi
  done

  echo
  echo "  -- full contents of all six scripts (they are tiny) --"
  for f in /usr/local/bin/finish.sh /usr/local/bin/finish.sh.qm /usr/local/bin/finish.sh.orig \
           /usr/local/bin/pause.sh  /usr/local/bin/pause.sh.qm  /usr/local/bin/pause.sh.orig; do
    echo "     ---------- $f ----------"
    [ -e "$f" ] && sed 's/^/       /' "$f"
  done

  echo
  if [ "$MISSING" -eq 1 ]; then
    echo "  >>> VERDICT: do NOT enable yet. A .qm or .orig is missing, so either"
    echo "      enabling half-succeeds or disabling cannot restore the power button."
  elif [ "$DRIFT" -eq 1 ]; then
    echo "  >>> VERDICT: enable only with a manual backup first. Your live power-button"
    echo "      script differs from the .orig that Disable restores, so turning Quick"
    echo "      Mode off would NOT return the device to its current behaviour."
  else
    echo "  >>> VERDICT: round trip is clean."
  fi
  echo
  echo "  -- firstboot.service: enabled by Enable, NOT undone by Disable --"
  systemctl is-enabled firstboot 2>&1 | sed 's/^/    is-enabled: /'
  grep -E "^(ExecStart|User|WorkingDirectory)=" /etc/systemd/system/firstboot.service 2>/dev/null | sed 's/^/    /'

  echo
  echo "  -- THE BOOT-INTO-GAME HIJACK --"
  echo "     On ArkOS4Clone this is NOT done by repointing ExecStart. The hook"
  echo "     lives inside the script ExecStart already runs, so a clean-looking"
  echo "     ExecStart proves nothing. Check the script itself:"
  BOOTSH=$(grep -E "^ExecStart=" /etc/systemd/system/firstboot.service 2>/dev/null | cut -d= -f2- | awk '{print $1}')
  [ -z "$BOOTSH" ] && BOOTSH=/boot/firstboot.sh
  echo "     ExecStart script: $BOOTSH"
  if [ -e "$BOOTSH" ] && grep -q "lastgame.sh" "$BOOTSH" 2>/dev/null; then
    echo "     >>> HIJACK ACTIVE: it runs lastgame.sh, so with Quick Mode on the"
    echo "         device boots straight into the last game, skipping ES."
    grep -n "lastgame.sh" "$BOOTSH" 2>/dev/null | sed 's/^/         /'
  else
    echo "     >>> no lastgame.sh call found; boot should land on the ES home."
  fi
  echo "     pending lastgame.sh (written at power-off, consumed at boot):"
  if [ -e /home/ark/.config/lastgame.sh ]; then
    echo "        PRESENT -- next boot WILL jump straight into a game"
  else
    echo "        absent -- next boot goes to the ES home"
  fi

  echo
  echo "########## 10. WAS THAT A REAL POWER CYCLE, OR A WAKE? ##########"
  echo "  Suspend/wake returns you to your game with no savestate involved,"
  echo "  and looks identical to a resume. Boot time tells them apart."
  echo "    now:            $(date '+%F %T')"
  echo "    last boot:      $(uptime -s 2>/dev/null || who -b 2>/dev/null)"
  echo "    uptime:         $(uptime -p 2>/dev/null || cat /proc/uptime | cut -d' ' -f1)"
  echo
  echo "  If 'last boot' is AFTER the moment you powered the device down,"
  echo "  it really did power off and come back. If it predates that, the"
  echo "  device only ever suspended."
  echo
  echo "  -- auto savestates, newest first (proof a save actually happened) --"
  find /roms /home/ark/.config -name "*.state.auto" -printf "    %TY-%Tm-%Td %TH:%TM  %8s bytes  %p\n" 2>/dev/null | sort -r | head -10
  echo "    total: $(find /roms /home/ark/.config -name '*.state.auto' 2>/dev/null | wc -l)"

  echo
  echo "==================================================================="
  echo "  END  $(date '+%F %T')"
  echo "==================================================================="
} > "$OUT" 2>&1

sync

# The Quick Mode verdict is the reason most people run this, so put it on screen
# rather than making them open the file to find it.
printf "\n===================================================\n"
if [ "${MISSING:-0}" -eq 1 ]; then
  printf "  QUICK MODE: DO NOT ENABLE YET\n"
  printf "===================================================\n"
  printf "\n  A .qm or .orig power-button script is missing, so\n"
  printf "  either enabling half-succeeds or disabling cannot\n"
  printf "  restore your power button.\n"
elif [ "${DRIFT:-0}" -eq 1 ]; then
  printf "  QUICK MODE: BACK UP FIRST\n"
  printf "===================================================\n"
  printf "\n  Your live power-button script does NOT match the\n"
  printf "  .orig that the Disable script restores, so turning\n"
  printf "  Quick Mode off would not return the device to how\n"
  printf "  it behaves today. See section 9 for the contents.\n"
else
  printf "  QUICK MODE: SAFE TO ENABLE\n"
  printf "===================================================\n"
  printf "\n  The power-button scripts round trip cleanly.\n"
  printf "  Note: firstboot.service stays enabled afterwards;\n"
  printf "  the Disable script does not undo that one.\n"
fi

printf "\n  Quick Mode is currently: "
if [ -e /usr/local/bin/quickmode.sh ]; then printf "ENABLED\n"; else printf "not enabled\n"; fi

STATES=$(find /roms /home/ark/.config -name '*.state.auto' 2>/dev/null | wc -l)
printf "  Auto savestates on disk:  %s\n" "$STATES"
printf "  Last boot:                %s\n" "$(uptime -s 2>/dev/null || who -b 2>/dev/null)"

printf "\n"
if [ "$STATES" -gt 0 ]; then
  printf "  => Savestates exist, so Quick Mode really is saving.\n"
  printf "     Compare 'Last boot' above against when you powered\n"
  printf "     down: later means a true power cycle, earlier means\n"
  printf "     the device only suspended.\n"
else
  printf "  => NO savestates yet. If a game appeared to resume, the\n"
  printf "     device suspended rather than powered off -- suspend\n"
  printf "     restores from RAM and writes nothing to disk.\n"
fi

printf "\n  Full report: %s\n" "$OUT"
printf "  Nothing was modified.\n"
printf "\nPress any key to close.\n"
read -n 1 -s -r
