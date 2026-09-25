Mono Max — the carousel by logo rather than by name.

Same geometry and colour as Mono Dark; the difference is `<image name="logo">`
pointing at `./system/${system.theme}.svg`, so each row draws its system's
logo. Any system with no file here falls back to `logoText`, which is styled
to match — so the two coexist and nothing is ever blank.

The system logos are from **Monochrome Gaming Logos** by HVR88
(https://github.com/HVR88/Monochrome-Gaming-Logos), renamed to the theme ids
EmulationStation looks for — the source names them by manufacturer and model
(`nintendo_snes`, `nec_turbografx16`, `snk_neogeo`, `laserdisc_daphne`), not by
theme id. Where a system had several variants, the most recognisable wide form
was taken, because a carousel row is 4.4:1 and a stacked logo reads small in it.

They are single-fill white artwork, which is what lets the engine tint them:
white on the unselected rows, black once a row is selected and the pill is
under it. They are also tight-cropped, with no padding baked into the canvas —
so `logoSize` in `theme.xml` is the drawn height, not an upper bound on a
mostly-empty box.

Copy them verbatim. An optimiser broke this set once before by stripping the
ids that `url(#...)` references.

These systems have no logo in that collection and keep their earlier artwork:
`arduboy`, `doom`, `easyrpg`, `j2me`, `megaduck`, `mplayer`, `pc88`, `pc98`,
`pokemini`, `pokemonmini`, `st-v`, `stv`, `supervision`, `uzebox`, `zx81`, and
the non-system entries `completed`, `custom-collections`, `idtech`,
`imageviewer`, `music`, `now-playing`, `retroarch`, `tools`.

The auto-collection icons (`auto-*.svg`) are drawn separately, in the Mono
wordmark style rather than taken from any collection.
