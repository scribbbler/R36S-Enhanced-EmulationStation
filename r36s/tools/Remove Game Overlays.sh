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
      else
        # fallback: strip the keys we add
        sed -i -E '/^(input_overlay|input_overlay_enable|input_overlay_opacity|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
        echo "  stripped keys:     $f"
      fi
      ;;
  esac
done < "$MANIFEST"

chown -R ark:ark /home/ark/.config/retroarch/config 2>/dev/null
sync
echo "=== DONE. Overlay art left in place (unused). ==="
