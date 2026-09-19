# EmulationStation for the R36S (Text UI)

A customized build of EmulationStation (the [fcamod](https://github.com/lcdyk0517/EmulationStation-fcamod) fork) for the **R36S / RK3326 ArkOS4Clone** handheld (640×480), plus a minimal **Text UI** theme and on‑device install tools.

Everything here targets what a theme alone can't change — the battery indicator, selection highlights, the on‑screen keyboard, the screensaver clock, screenshots, and more — all made theme‑driven so other themes are unaffected unless they opt in.

> The separate **music player** for the R36S lives in its own repository.

## Features

- **Battery indicator** — crisp rasterized icon, 21‑level (5%) SVG selection, and a themeable icon set + font/size via `<batteryIndicator>`.
- **Pill‑shaped selectors** — theme‑driven rounded selection highlights for the carousel, gamelist and menu (`selectorColor`/`selectorRadius`/`selectorHeight`/`selectorWidth`, fit‑content mode, and `logoColor`/`logoSelectedColor` tinting).
- **Themeable screensaver clock** — custom font via `<screensaverClock>` and a `19th Sep, 2026` style date.
- **On‑screen keyboard** — themeable SVG icons for backspace/enter/shift/alt, solid key fill + gap + corner radius, fixed grid width/position, a simplified two‑layer search layout, and footer button hints.
- **FN + B screenshot** — saves a PNG to `/roms/screenshots`.
- **Clock + battery on the menu** overlay, themeable **menu arrow** icons, and Title‑Cased custom collections.

## Repository layout

```
es-app/, es-core/, …     the modified EmulationStation source
r36s/
  theme/Text UI/         the Text UI theme
  tools/                 on-device install/utility scripts (see below)
  docker/                Docker build environment + notes
```

## Install (prebuilt)

1. Grab `emulationstation.custom` from the [Releases](../../releases) page.
2. Copy the tools onto your card:
   - `r36s/tools/*` → `/roms/tools/` (the ES **Options → Tools** menu)
   - the binary → `/roms/tools/es-custom/emulationstation.custom`
   - `r36s/theme/Text UI` → `/roms/themes/Text UI`
3. On the device: **Menu → Options → Tools → Install Custom ES**. It backs up the stock binary, sanity‑checks the new one, installs it with a crash‑failsafe, and restarts EmulationStation. **Restore Stock ES** reverts.
4. Select the **Text UI** theme in *UI Settings → Theme*.

The installer only ever swaps `/usr/bin/emulationstation/emulationstation`; a crash within 15s of launch (that isn't an intentional restart/shutdown) auto‑restores the stock binary.

### Tools included

| Tool | Purpose |
|---|---|
| Install Custom ES / Restore Stock ES | swap in/out the custom binary (with failsafe) |
| Apply Theme Font / Restore Stock Font | swap the stock UI font for BPreplay |
| Battery Probe / Hardware Probe | dump battery / LED / display / input info |
| LED Test 2 / LED Write Test | diagnose & set the power LED (clone driver needs manual mode) |
| Copy ES Log | copy the ES logs to the card for inspection |

## Build

See [`r36s/docker/BUILD.md`](r36s/docker/BUILD.md). In short: an `ubuntu:19.10` (eoan) arm64 Docker image with the fcamod deps, built with `cmake -DGLES=ON . && make`. Two device‑specific blobs (`libgo2.so` and the ARM **Mali** userspace driver) are **not** redistributable and must be extracted from your own device's rootfs — the build notes explain how.

## Credits & license

EmulationStation is MIT‑licensed, © 2014 Alec Lofquist. This is a downstream customization of the fork chain **Aloshi → [christianhaitian](https://github.com/christianhaitian/EmulationStation-fcamod) → [lcdyk0517](https://github.com/lcdyk0517/EmulationStation-fcamod)**. The upstream readme is kept as [`README-upstream.md`](README-upstream.md); see [`LICENSE.md`](LICENSE.md). The BPreplay font and third‑party theme assets retain their own licenses.
