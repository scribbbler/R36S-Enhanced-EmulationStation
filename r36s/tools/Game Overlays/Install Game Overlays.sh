#!/bin/bash
# Install Jeltron black/minimal RetroArch overlays as per-core overrides.
#   Handhelds get a black frame + exact viewport; 4:3 consoles get a subtle
#   full-screen frame (CRT_FrameOnly) with no viewport change.
# Reversible: backs up any override it touches (.pre-overlay.bak) and there is
# a companion "Remove Game Overlays.sh". Re-runnable (idempotent).

RAHOME=/home/ark/.config/retroarch
OVSRC=/roms/tools/overlays/jeltron
OVDST=$RAHOME/overlay
CFGDIR=$RAHOME/config
ESS=/etc/emulationstation/es_systems.cfg
LOG=/roms/tools/es-custom/overlay-install.log
MANIFEST=/roms/tools/es-custom/overlay-manifest.txt
# Keep the previous manifest so files WE created in an earlier run stay "NEW"
# across re-runs (otherwise a re-run would back up our own overlay config and
# the remover would "restore" it instead of deleting it).
PREV="$MANIFEST.prev"
{ [ -f "$MANIFEST" ] && cp -f "$MANIFEST" "$PREV"; } || : > "$PREV"
: > "$MANIFEST"

exec > >(tee "$LOG") 2>&1
echo "=================================================================="
echo "  INSTALL GAME OVERLAYS   $(date '+%F %T')"
echo "=================================================================="

[ -f "$ESS" ] || ESS="$(find /etc /opt -name es_systems.cfg 2>/dev/null | head -1)"

# 1) copy overlay art + cfgs into RetroArch's overlay dir.
# shifted-r36s/ holds bezels nudged down a few px so their windows meet the
# integer-scaled (centered) game; they override the stock art by filename.
mkdir -p "$OVDST"
cp "$OVSRC"/*.cfg "$OVSRC"/*.png "$OVDST"/ 2>/dev/null
[ -d "$OVSRC/shifted-r36s" ] && cp "$OVSRC/shifted-r36s"/*.png "$OVDST"/ 2>/dev/null && echo "Applied shifted-r36s bezels"
echo "Copied overlays -> $OVDST"
echo

# first <core> listed under the retroarch emulator for a system (ArkOS default)
first_core() {
  awk -v s="$1" '
    $0 ~ "<name>"s"</name>" {f=1}
    f && /<emulator name="retroarch">/ {e=1}
    f && e && /<core>/ { line=$0; sub(/.*<core>/,"",line); sub(/<\/core>.*/,"",line); print line; exit }
  ' "$ESS"
}

# RetroArch override-folder name for a core (.so basename, no _libretro).
# Prefer the .info "corename" field; fall back to a known map.
corename_of() {
  local core="$1"
  local info="$RAHOME/cores/${core}_libretro.info"
  local cn=""
  [ -f "$info" ] && cn="$(grep -E '^corename' "$info" | head -1 | cut -d'"' -f2)"
  if [ -z "$cn" ]; then
    case "$core" in
      gambatte)            cn="Gambatte" ;;
      gpsp|gpsp_ezode_mod) cn="gpSP" ;;
      mgba|mgba_rumble)    cn="mGBA" ;;
      vbam)                cn="VBA-M" ;;
      vba_next)            cn="VBA Next" ;;
      mednafen_ngp)        cn="Beetle NeoPop" ;;
      race)                cn="RACE" ;;
      snes9x)              cn="Snes9x" ;;
      genesis_plus_gx)     cn="Genesis Plus GX" ;;
      fceumm)              cn="FCEUmm" ;;
      nestopia)            cn="Nestopia" ;;
      *)                   cn="" ;;
    esac
  fi
  echo "$cn"
}

setkey() { # file key value
  local f="$1" k="$2" v="$3"
  touch "$f"
  sed -i "/^${k} =/d" "$f"
  echo "${k} = \"${v}\"" >> "$f"
}

apply() { # system overlaycfg [x y w h] [smooth]   (x/y/w/h in LANDSCAPE screen coords)
  local sys="$1" ov="$2" x="$3" y="$4" w="$5" h="$6" smooth="${7:-false}"
  local core cn d f px py pw ph
  core="$(first_core "$sys")"
  if [ -z "$core" ]; then echo "  - $sys: not found in es_systems.cfg, skip"; return; fi
  cn="$(corename_of "$core")"
  if [ -z "$cn" ]; then echo "  ! $sys: core '$core' -> unknown override name, SKIP"; return; fi
  d="$CFGDIR/$cn"; f="$d/$cn.cfg"
  mkdir -p "$d"
  if [ ! -f "$f" ] || { grep -qF "NEW|$f" "$PREV" && [ ! -f "$f.pre-overlay.bak" ]; }; then
    echo "NEW|$f" >> "$MANIFEST"
  else
    [ -f "$f.pre-overlay.bak" ] || cp "$f" "$f.pre-overlay.bak"
    echo "MOD|$f" >> "$MANIFEST"
  fi
  setkey "$f" input_overlay "$OVDST/$ov"
  setkey "$f" input_overlay_enable "true"
  setkey "$f" input_overlay_opacity "1.000000"
  if [ -n "$x" ]; then
    # Custom viewports are unusable on this RetroArch fork (it applies them in
    # a panel space we could not reliably map - rendered oversized/offset).
    # Core aspect + integer scaling centers the image cleanly instead:
    # GB 3x = 480x432, NeoGeo 2x = 608x448, GBA a sharp integer step. The
    # bezel art is shifted a few px at install time to meet the centered game.
    setkey "$f" aspect_ratio_index "22"   # 22 = Core provided
    setkey "$f" video_scale_integer "true"
    setkey "$f" video_smooth "$smooth"
    sed -i -E '/^custom_viewport_(x|y|width|height) =/d' "$f"
    echo "  OK $sys -> $core -> [$cn] -> $ov  (core aspect, integer scale)"
  else
    echo "  OK $sys -> $core -> [$cn] -> $ov  (frame only)"
  fi
}

# Content-directory override: config/<CoreName>/<contentdir>.cfg — layered on top
# of the core override, so systems that share a core can differ (e.g. gbc vs gb).
apply_contentdir() { # system contentdir overlaycfg
  local sys="$1" cdir="$2" ov="$3" core cn d f
  core="$(first_core "$sys")"
  if [ -z "$core" ]; then echo "  - $sys: not found, skip"; return; fi
  cn="$(corename_of "$core")"
  if [ -z "$cn" ]; then echo "  ! $sys: core '$core' -> unknown override name, SKIP"; return; fi
  d="$CFGDIR/$cn"; f="$d/$cdir.cfg"
  mkdir -p "$d"
  if [ ! -f "$f" ] || { grep -qF "NEW|$f" "$PREV" && [ ! -f "$f.pre-overlay.bak" ]; }; then
    echo "NEW|$f" >> "$MANIFEST"
  else
    [ -f "$f.pre-overlay.bak" ] || cp "$f" "$f.pre-overlay.bak"; echo "MOD|$f" >> "$MANIFEST"
  fi
  # Scaling comes from the CORE override; a content-dir override outranks it,
  # so strip any hand-saved scaling keys here or they silently win.
  [ -f "$f" ] && sed -i -E '/^(aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
  setkey "$f" input_overlay "$OVDST/$ov"
  setkey "$f" input_overlay_enable "true"
  setkey "$f" input_overlay_opacity "1.000000"
  echo "  OK $sys (content-dir '$cdir') -> [$cn] -> $ov"
}

echo "---- suppress 'Configuration override loaded' toast ----"
for base in /home/ark/.config/retroarch/retroarch.cfg /home/ark/.config/retroarch32/retroarch.cfg; do
  [ -f "$base" ] && setkey "$base" notification_show_config_override_load "false" && echo "  set in $base"
done

echo "---- reset global scaling keys (undo Quick-Menu 'Save Configuration' drift) ----"
# A hand-save while testing can bake a GB-sized custom viewport + overlay into
# the GLOBAL cfg, shrinking and shifting EVERY system. Reset to stock; our
# per-core overrides below carry the real overlay settings.
for base in /home/ark/.config/retroarch/retroarch.cfg /home/ark/.config/retroarch32/retroarch.cfg; do
  [ -f "$base" ] || continue
  sed -i -E 's/^video_scale_integer = .*/video_scale_integer = "false"/;
             s|^input_overlay = .*|input_overlay = ""|;
             s/^aspect_ratio_index = .*/aspect_ratio_index = "22"/;
             s/^custom_viewport_x = .*/custom_viewport_x = "0"/;
             s/^custom_viewport_y = .*/custom_viewport_y = "0"/' "$base" && echo "  reset $base"
done

echo "---- strip stray hand-saved overrides referencing overlays ----"
# Game / content-dir overrides saved from the Quick Menu while an overlay was
# active outrank what we write below; clean them before installing fresh.
find /home/ark/.config/retroarch/config /home/ark/.config/retroarch32/config \
     -name '*.cfg' ! -name '*.pre-overlay.bak' 2>/dev/null | while read -r f; do
  grep -q '^input_overlay = ".*/overlay/' "$f" || continue
  sed -i -E '/^(input_overlay|input_overlay_enable|input_overlay_opacity|aspect_ratio_index|video_scale_integer|video_smooth|custom_viewport_(x|y|width|height)) =/d' "$f"
  echo "  cleaned: $f"
done

echo "---- Handhelds ----"
# gb + gbc share the gambatte core: gb sets the core override (GB_NoGrid); gbc
# gets a content-directory override (GB_Color) layered on top of it.
apply gb  GB_NoGrid.cfg          80 12 480 432
apply_contentdir gbc gbc GB_Color.cfg
apply gba GBA_3x_BorderOnly.cfg   0  0 640 427 true

echo "---- NeoGeo (dedicated bezel) ----"
apply neogeo CRT_NeoGeo.cfg      16  8 608 448

echo "---- 4:3 consoles (frame only) ----"
for s in nes snes megadrive mastersystem pcengine pcenginecd segacd psx atari2600; do
  apply "$s" CRT_FrameOnly.cfg
done

# RetroArch runs as 'ark' -> make sure it can read what we wrote
chown -R ark:ark "$OVDST" "$CFGDIR" 2>/dev/null
sync
echo
echo "=================================================================="
echo "  DONE.  Launch a GB / GBA / SNES game to see it."
echo "  Log: $LOG"
echo "=================================================================="
