# Game overlays (RetroArch)

On-device tools that wire up **black / minimal RetroArch overlays** for the
handhelds and 4:3 consoles, as per-core (and per-content-directory) overrides —
so the game sits sharp in the middle and the unused screen area becomes a clean
black bezel.

## Overlay art is NOT included

The overlays themselves are **Jeltron's "480p overlay set"**, from
<https://github.com/Jeltr0n/Retro-Overlays>. That project ships **no license**
(default: all rights reserved), so the `.png`/`.cfg` art is **not redistributed
here**. Download it yourself and drop the files below into
`/roms/tools/overlays/jeltron/` on the card:

| Purpose | Files |
|---|---|
| Game Boy (gb) | `GB_NoGrid.cfg` + `GB_NoGrid.png` |
| Game Boy Color (gbc) | `GB_Color.cfg` + `GB_Color.png` |
| Game Boy Advance (gba) | `GBA_3x_BorderOnly.cfg` + `GBA_3x_BorderOnly.png` |
| 4:3 consoles | `CRT_FrameOnly.cfg` + `CRT_FrameOnly.png` |
| NeoGeo | `CRT_NeoGeo.cfg` + `CRT_NeoGeo.png` |

All credit for the overlay artwork goes to **Jeltron**.

## Tools (run from ES → Options → Tools)

- **`Overlay Probe.sh`** — read-only. Dumps the RetroArch layout (overlay dir,
  per-core override dir, cores per system) to `/roms/tools/es-custom/` so the
  installer can be targeted correctly. Run this first on a new device.
- **`Install Game Overlays.sh`** — copies the overlays into RetroArch's overlay
  directory and writes per-core overrides:
  - **gb** → `GB_NoGrid`, viewport `80/13/480/432`
  - **gbc** → `GB_Color` (content-dir override, since gb+gbc share the gambatte core)
  - **gba** → `GBA_3x_BorderOnly`, viewport `0/0/640/427`
  - **neogeo** → `CRT_NeoGeo`, viewport `16/8/608/448`
  - **nes, snes, megadrive, mastersystem, pcengine, pcenginecd, segacd, psx,
    atari2600** → `CRT_FrameOnly` (subtle frame, no viewport change)

  It maps each system to its **default core** (first core in `es_systems.cfg`),
  resolves the RetroArch override-folder name from the core's `.info`, backs up
  anything it touches (`*.pre-overlay.bak` + a manifest), and is re-runnable. It
  also silences the "Configuration override loaded" toast.
- **`Remove Game Overlays.sh`** — undoes the installer using the manifest
  (restores backups / deletes files it created). Leaves the overlay art in place.

## Notes / limitations

- Overrides target each system's **default core**. If you've switched a system
  to a non-default core, that system won't pick up the overlay until re-mapped.
- Overlays only apply to **RetroArch** cores (not standalone emulators).
- 4:3 consoles already fill the screen, so `CRT_FrameOnly` is deliberately subtle.
