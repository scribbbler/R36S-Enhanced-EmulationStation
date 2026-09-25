Mono Max — the carousel by logo rather than by name.

Same geometry and colour as Mono Dark; the difference is `<image name="logo">`
pointing at `./system/${system.theme}.svg`, so each row draws its system's
logo. Any system with no file here falls back to `logoText`, which is styled
to match — so the two coexist and nothing is ever blank.

The console logos are from **Monochrome Gaming Logos** by HVR88
(https://github.com/HVR88/Monochrome-Gaming-Logos), renamed to the theme ids
EmulationStation looks for. They are single-fill white artwork, which is what
lets the engine tint them: white on the unselected rows, black once a row is
selected and the pill is under it.

The remaining files are the auto-collection icons from Onyx UI.
