#!/bin/bash
# Root-cause fix for RetroArch's "not saving - overrides active":
# RA refuses to save the global config while the current core has ANY
# override file - even an empty one. Override files accumulated during the
# overlay experiments; after removal many are empty shells. This deletes
# override files with no settings left and logs the ones that still hold
# real settings (review before deleting those - they may be ArkOS defaults).

LOG=/roms/tools/es-custom/fix-config-saving.log
{
  echo "=== FIX CONFIG SAVING  $(date '+%F %T') ==="
  for cfgroot in /home/ark/.config/retroarch/config /home/ark/.config/retroarch32/config; do
    [ -d "$cfgroot" ] || continue
    find "$cfgroot" -mindepth 2 -maxdepth 2 -name '*.cfg' ! -name '*.bak' | while read -r f; do
      if grep -qE '^[a-zA-Z0-9_]+ =' "$f"; then
        echo "KEEP (has settings): $f"
        sed 's/^/    /' "$f"
      else
        rm -f "$f" && echo "DELETED (empty): $f"
        rmdir "$(dirname "$f")" 2>/dev/null
      fi
    done
  done
  echo
  echo "=== DONE."
  echo "Cores with no override left can now save the global config normally."
  echo "Cores listed KEEP still block global saves; their contents are shown"
  echo "above so you can decide whether to delete them too."
} | tee "$LOG"
sync
