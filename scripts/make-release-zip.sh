#!/bin/bash
# Assemble the one-zip release asset: unzips straight onto the ROMs card
# (top-level tools/ and themes/ merge into place; the prebuilt binary sits
# at tools/es-custom/emulationstation.custom where Install Custom ES expects it).
#
#   ./scripts/make-release-zip.sh v1.2.0
#
# Run from the repo root. Expects a freshly built ./emulationstation.custom.
set -euo pipefail

VER="${1:?usage: make-release-zip.sh <version>}"
OUT="mono-r36s-$VER.zip"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

[ -f emulationstation.custom ] || { echo "missing ./emulationstation.custom - build first"; exit 1; }

mkdir -p "$STAGE/tools/es-custom" "$STAGE/themes"
cp -R r36s/tools/. "$STAGE/tools/"
cp emulationstation.custom "$STAGE/tools/es-custom/emulationstation.custom"

# Ship the released themes; Mono Classic is WIP and stays out for now.
for t in "Mono Dark" "Mono Light" "Mono Fit Dark" "Mono Fit Light"; do
  cp -R "r36s/theme/$t" "$STAGE/themes/"
done

# no mac cruft in the zip
find "$STAGE" \( -name '._*' -o -name '.DS_Store' \) -delete

( cd "$STAGE" && zip -qr9 "$OLDPWD/$OUT" tools themes )
echo "built $OUT"
unzip -l "$OUT" | tail -3
