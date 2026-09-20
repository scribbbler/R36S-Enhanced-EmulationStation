# R36S Enhanced EmulationStation

A modified EmulationStation build **developed and tested on an R36S clone running
ArkOS4Clone**, adding `theme.xml` control over UI elements that were previously
hardcoded.

Looking for testers on **genuine R36S units and other RK3326 variants** — see
[Compatibility](#compatibility).

**Themes can now customise:** battery indicator · selection pills · menu UI ·
screensaver clock · on‑screen keyboard · selector geometry · button‑hint bar ·
volume/brightness pop‑ups · icons · and more.

Every addition is **opt‑in** — existing themes are unaffected unless they use the
new properties. The bundled **Text UI** theme is included as a reference
implementation; other theme authors can use the new properties in their own
themes. See **[THEME‑EXTENSIONS.md](THEME-EXTENSIONS.md)** for the full list.

> _Screenshots: **TODO** — add carousel / keyboard / screensaver / pop‑up captures here._

**Safe to try:** the installer backs up your original EmulationStation binary and
provides a **Restore Stock ES** tool. If the custom build crashes during startup,
the stock binary is **automatically restored**. (Still keep an SD‑card backup.)

> The separate **music player** for the R36S lives in its own repository.

## Features

- **Battery indicator** — crisp rasterized icon, 21‑level (5%) SVG selection, and a themeable icon set + font/size via `<batteryIndicator>`.
- **Pill‑shaped selectors** — theme‑driven rounded selection highlights for the carousel, gamelist and menu (`selectorColor`/`selectorRadius`/`selectorHeight`/`selectorWidth`, fit‑content mode, and `logoColor`/`logoSelectedColor` tinting).
- **Clock screensaver** (Select on the carousel) — themeable font/date, 12‑hour support, a lock icon while input is locked, and a battery icon + charge % while charging (`screensaverClock` / `screensaverBattery` / `screensaverLock`).
- **On‑screen keyboard** — themeable SVG icons for backspace/enter/shift/alt, solid key fill + gap + corner radius, fixed grid width/position, a simplified two‑layer search layout, and footer button hints.
- **Themeable button‑hint bar** and **volume/brightness pop‑ups** — custom labels, icons, spacing and background via `<helpsystem>` / `<volumeIndicator>` / `<brightnessIndicator>`.
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

## Compatibility

This build has currently been tested on an **R36S clone running ArkOS4Clone**.

**Tested**
- R36S clone / RK3326‑based device
- ArkOS4Clone
- 640×480 display

**Not yet verified**
- Genuine R36S hardware
- Standard / community ArkOS builds for genuine R36S units
- Other R35S/R36S clone variants
- Different display panels or resolutions

Because R36S clones can differ significantly in hardware, DTB files, audio
configuration, display panels and system builds, compatibility should not be
assumed across every device. The installer backs up the existing EmulationStation
binary and provides a stock‑restore option, but **keep a backup of your SD card
before testing**.

If you successfully test this build on another R36S or clone variant, please open
an issue with your device type, ArkOS version, and display/panel information —
hardware coverage is exactly what this project needs.

## Build

See [`r36s/docker/BUILD.md`](r36s/docker/BUILD.md). In short: an `ubuntu:19.10` (eoan) arm64 Docker image with the fcamod deps, built with `cmake -DGLES=ON . && make`. Two device‑specific blobs (`libgo2.so` and the ARM **Mali** userspace driver) are **not** redistributable and must be extracted from your own device's rootfs — the build notes explain how.

## Credits & license

EmulationStation is MIT‑licensed, © 2014 Alec Lofquist. This is a downstream customization of the fork chain **Aloshi → [christianhaitian](https://github.com/christianhaitian/EmulationStation-fcamod) → [lcdyk0517](https://github.com/lcdyk0517/EmulationStation-fcamod)**. The upstream readme is kept as [`README-upstream.md`](README-upstream.md); see [`LICENSE.md`](LICENSE.md). The BPreplay font and third‑party theme assets retain their own licenses.
