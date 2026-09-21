#!/bin/bash
# Undo "Install Game Overlays.sh": restore backed-up per-core overrides and
# delete the ones we created. Does not remove the overlay art (harmless).

MANIFEST=/roms/tools/es-custom/overlay-manifest.txt
LOG=/roms/tools/es-custom/overlay-remove.log
exec > >(tee "$LOG") 2>&1
echo "=== REMOVE GAME OVERLAYS  $(date '+%F %T') ==="

if [ ! -f "$MANIFEST" ]; then
  echo "No manifest found ($MANIFEST) - nothing to undo."
  exit 0
fi

while IFS='|' read -r kind f; do
  [ -z "$f" ] && continue
  case "$kind" in
    NEW)
      rm -f "$f"; echo "  removed (was new): $f"
      # remove now-empty core dir
      rmdir "$(dirname "$f")" 2>/dev/null
      ;;
    MOD)
      if [ -f "$f.pre-overlay.bak" ]; then
        mv -f "$f.pre-overlay.bak" "$f"
        echo "  restored backup:   $f"
        # A backup taken by an older installer version could itself contain
        # overlay settings from an even earlier run. If it points at one of
        # OUR overlay cfgs, it isn't stock - strip our keys from it too.
        if grep -qE '^input_overlay = .*(GB_NoGrid|GB_Color|GBA_3x_BorderOnly|CRT_NeoGeo|CRT_FrameOnly)\.cfg' "$f"; then
          sed -i -E '/^(input_overlay|input_overlay_enable|input_overlay_opacity|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
          echo "  (backup contained our overlay settings - stripped them)"
        fi
      else
        # fallback: strip the keys we add
        sed -i -E '/^(input_overlay|input_overlay_enable|input_overlay_opacity|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
        echo "  stripped keys:     $f"
      fi
      ;;
  esac
done < "$MANIFEST"

# Reset overlay-experiment keys hand-saved into the GLOBAL configs (Quick
# Menu "Save Configuration" while testing) back to ArkOS stock: no overlay,
# core-provided aspect, integer scaling off, viewport offset zeroed.
echo "---- global config -> stock (integer off, no overlay, core aspect) ----"
for base in /home/ark/.config/retroarch/retroarch.cfg /home/ark/.config/retroarch32/retroarch.cfg; do
  [ -f "$base" ] || continue
  sed -i -E 's/^video_scale_integer = .*/video_scale_integer = "false"/;
             s|^input_overlay = .*|input_overlay = ""|;
             s/^aspect_ratio_index = .*/aspect_ratio_index = "22"/;
             s/^custom_viewport_x = .*/custom_viewport_x = "0"/;
             s/^custom_viewport_y = .*/custom_viewport_y = "0"/' "$base" \
    && echo "  reset in $base"
done

# Sweep stray overrides saved by hand from the Quick Menu while an overlay
# was active (game overrides etc. that our manifest never knew about).
echo "---- stray hand-saved overrides referencing overlays ----"
find /home/ark/.config/retroarch/config /home/ark/.config/retroarch32/config \
     -name '*.cfg' ! -name '*.pre-overlay.bak' 2>/dev/null | while read -r f; do
  grep -q '^input_overlay = ".*/overlay/' "$f" || continue
  sed -i -E '/^(input_overlay|input_overlay_enable|input_overlay_opacity|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
  echo "  stripped overlay keys: $f"
done

chown -R ark:ark /home/ark/.config/retroarch/config 2>/dev/null
sync
echo "=== DONE. Overlay art left in place (unused). ==="
