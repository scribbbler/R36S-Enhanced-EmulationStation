# R36S EmulationStation — Theme Extensions

These are the `theme.xml` properties this build adds on top of the stock
ArkOS / fcamod EmulationStation theme format. Everything here is **opt‑in**:
if a theme doesn't set a property, the built‑in behaviour is unchanged, so
existing themes keep working untouched.

Values follow the normal ES conventions:

- **`NORMALIZED_PAIR`** / **`FLOAT`** — fractions of the screen (`0`–`1`). e.g. a
  `fontSize` of `0.0333` is ≈16px on a 480px‑tall panel.
- **`COLOR`** — `RRGGBB` or `RRGGBBAA` hex.
- **`PATH`** — file path relative to the theme folder (`./art/...`).

All examples below are taken verbatim from the included **Mono Dark** reference
theme, so they are known‑good on a 640×480 panel.

> Screenshots: _TODO — add a capture next to each section._

---

## `batteryIndicator`  ·  view: `system`

A fully themeable battery readout (icon set + optional `%` text) that replaces
the stock hardcoded indicator. Uses a 21‑step (5%) SVG icon set for crisp,
granular level display.

| Property | Type | Notes |
|---|---|---|
| `pos` / `size` | NORMALIZED_PAIR | Placement of the indicator block. |
| `horizontalAlignment` | STRING | `left` / `center` / `right`. |
| `itemSpacing` | FLOAT | Gap between the icon and the `%` text. |
| `fontPath` / `fontSize` | PATH / FLOAT | Font for the `%` text. |
| `color` | COLOR | Tint for the icon + text. |
| `incharge` | PATH | Icon shown while charging. |
| `full` `at75` `at50` `at25` `empty` | PATH | Level icons (any subset). |
| `visible` | BOOLEAN | Hide the indicator entirely. |

```xml
<batteryIndicator name="batteryIndicator">
  <color>FFFFFF</color>
  <pos>0 0.0166666667</pos>
  <size>0.98125 0.0666666667</size>
  <horizontalAlignment>right</horizontalAlignment>
  <fontPath>./fonts/BPreplay-Regular.otf</fontPath>
  <fontSize>0.0333333</fontSize>
  <itemSpacing>0.009375</itemSpacing>
  <incharge>./art/battery/battery-charging.svg</incharge>
  <full>./art/battery/battery-100.svg</full>
  <at75>./art/battery/battery-75.svg</at75>
  <at50>./art/battery/battery-50.svg</at50>
  <at25>./art/battery/battery-25.svg</at25>
</batteryIndicator>
```

---

## Selection pills  ·  `carousel` / `textlist` / `menuText`

Rounded ("pill") selection highlights for the carousel, game lists and menu,
plus tinting of the selected vs unselected row.

**Carousel** (`<carousel>`):

| Property | Type | Notes |
|---|---|---|
| `selectorColor` | COLOR | Pill fill. |
| `selectorWidth` | FLOAT | Fixed pill width (else fit‑content). |
| `selectorHeight` | FLOAT | Pill height. |
| `selectorRadius` | FLOAT | Corner radius (½ height = full pill). |
| `selectorFitContent` | BOOLEAN | Size the pill to the label instead of full width. |
| `logoColor` | COLOR | Unselected row text/logo tint. |
| `logoSelectedColor` | COLOR | Selected row tint (on the pill). |

```xml
<selectorColor>FFFFFFFF</selectorColor>
<selectorWidth>0.9375</selectorWidth>
<selectorHeight>0.15</selectorHeight>
<selectorRadius>0.075</selectorRadius>
<logoColor>FFFFFFFF</logoColor>
<logoSelectedColor>000000FF</logoSelectedColor>
```

The same `selectorColor` / `selectorRadius` / `selectorHeight` (plus
`selectorColorEnd` for a gradient) are honoured on `textlist` and `menuText`
so game lists and the main menu match.

---

## `helpsystem`  ·  view: `system`, `basic`, `detailed`, `video`

The on‑screen button‑hint bar, made fully theme‑driven: custom labels, custom
button icons, spacing, a rounded background "pill", and per‑view visibility.
Mono Dark uses it to render two fixed‑width pills.

| Property | Type | Notes |
|---|---|---|
| `pos` / `origin` | NORMALIZED_PAIR | Placement / anchor. |
| `fontPath` / `fontSize` | PATH / FLOAT | Label font. |
| `textColor` / `iconColor` | COLOR | Label and icon tint. |
| `iconSize` | FLOAT | Button icon size. |
| `iconTextSpacing` | FLOAT | Gap between an icon and its label. |
| `entrySpacing` | FLOAT | Gap between entries. |
| `textUppercase` | BOOLEAN | Upper‑case the auto (system) labels. |
| `backgroundColor` | COLOR | Pill fill (omit for none). |
| `backgroundRadius` | FLOAT | Pill corner radius. |
| `backgroundPadding` | NORMALIZED_PAIR | Inner padding. |
| `backgroundWidth` | FLOAT | Fixed pill width (else content width). |
| `visible` | BOOLEAN | Hide help in this view. |
| `iconA…iconSelect` | PATH | Custom icon per button (`iconA`, `iconB`, `iconX`, `iconY`, `iconL`, `iconR`, `iconStart`, `iconSelect`, `iconUpDown`, `iconLeftRight`, `iconUpDownLeftRight`). |
| `labelA…labelSelect` | STRING | Verbatim label override per button (same suffixes as icons). |

```xml
<helpsystem name="help">
  <fontPath>./fonts/BPreplay-Regular.otf</fontPath>
  <pos>0.0125 0.9875</pos>
  <origin>0 0</origin>
  <fontSize>0.0333333</fontSize>
  <textColor>FFFFFF</textColor>
  <iconColor>FFFFFF</iconColor>
  <iconSize>0.05</iconSize>
  <iconTextSpacing>0.009375</iconTextSpacing>
  <entrySpacing>0.025</entrySpacing>
  <textUppercase>true</textUppercase>
  <labelSelect>Clock</labelSelect>
  <labelStart>Menu</labelStart>
  <labelY>Search</labelY>
  <labelX>Random</labelX>
  <backgroundColor>2A2A2AFF</backgroundColor>
  <backgroundWidth>0.375</backgroundWidth>
</helpsystem>
```

---

## `keyboard`  ·  view: `system`

Styling for the on‑screen search keyboard: SVG icons for the special keys plus
grid geometry and key appearance.

| Property | Type | Notes |
|---|---|---|
| `fontPath` `fontSize` | PATH / FLOAT | Key‑letter face and size. Unset, the keys use the menu row font, so their size is tied to `menutext`; either property detaches them. |
| `backspace` `enter` `shift` `alt` | PATH | SVG icon for each special key. |
| `iconSize` | FLOAT | Fixed icon size on every special key. |
| `width` | FLOAT | Total key‑grid width. |
| `posY` | FLOAT | Grid top, from the screen top. |
| `keySpacing` | FLOAT | Gap between keys. |
| `keyRadius` | FLOAT | Key corner radius. |
| `keyColor` | COLOR | Unfocused key fill (focused key stays white/black). |

```xml
<keyboard name="keyboard">
  <backspace>./art/keyboard/backspace.svg</backspace>
  <enter>./art/keyboard/enter.svg</enter>
  <shift>./art/keyboard/shift.svg</shift>
  <alt>./art/keyboard/alt.svg</alt>
  <iconSize>0.05</iconSize>
  <width>0.975</width>
  <posY>0.2416666667</posY>
  <keySpacing>0.009375</keySpacing>
  <keyRadius>0.0166666667</keyRadius>
  <keyColor>0A0A0AFF</keyColor>
</keyboard>
```

---

## `menuArrow`  ·  view: `menu` (name `menuarrow`)

Themeable arrow/chevron icons in the menu.

| Property | Type | Notes |
|---|---|---|
| `path` | PATH | Submenu `>` chevron / option‑list right arrow. |
| `optionPath` | PATH | `<` `>` arrows around an option value. |

```xml
<menuArrow name="menuarrow">
  <path>./art/icons/arrow.svg</path>
  <optionPath>./art/icons/arrow.svg</optionPath>
</menuArrow>
```

---

## `volumeIndicator` / `brightnessIndicator`  ·  view: `screen`

Icon + font for the volume and brightness pop‑ups.

| Property | Type | Notes |
|---|---|---|
| `icon` | PATH | Glyph shown in the pop‑up (white‑tinted). |
| `fontPath` | PATH | Font for the `%` text. |

```xml
<volumeIndicator name="volumeIndicator">
  <icon>./art/icons/volume.svg</icon>
  <fontPath>./fonts/BPreplay-Bold.otf</fontPath>
</volumeIndicator>
<brightnessIndicator name="brightnessIndicator">
  <icon>./art/icons/brightness.svg</icon>
  <fontPath>./fonts/BPreplay-Bold.otf</fontPath>
</brightnessIndicator>
```

---

## Screensaver clock  ·  view: `screen`

The idle clock screensaver (toggled with **Select** on the carousel) reads
three standard elements by name. The time follows the *12‑hour clock* setting;
while charging, the clock/date are swapped for the battery icon + charge %.

| Element (name / type) | Property | Notes |
|---|---|---|
| `screensaverClock` (`text`) | `fontPath` / `fontSize` | Clock/date font. |
| `screensaverBattery` (`image`) | `path` | Icon shown in place of the clock while charging. |
| `screensaverLock` (`image`) | `path` | Small padlock shown above the date (input is locked). |

```xml
<view name="screen">
  <text name="screensaverClock">
    <fontPath>./fonts/BPreplay-Bold.otf</fontPath>
  </text>
  <image name="screensaverBattery">
    <path>./art/battery/battery-charging.svg</path>
  </image>
  <image name="screensaverLock">
    <path>./art/icons/lock.svg</path>
  </image>
</view>
```

---

---

## Gradient & fit-content selectors  ·  `carousel` / `textlist` / `menuText`

| Property | Element(s) | Type | Notes |
|---|---|---|---|
| `selectorColorEnd` | carousel, textlist, menuText | COLOR | Bottom color of a vertical gradient on the selection pill (works with any corner radius). Equal to `selectorColor` (or unset) = solid. The **Mono Classic** theme uses it for the classic blue iPod bar. |
| `selectorFitContent` | carousel, textlist | BOOLEAN | The pill hugs the selected text instead of spanning a fixed width. On the carousel the pad around the text is `selectorPadding`; on the textlist it is `horizontalMargin`. |
| `selectorPadding` | carousel | FLOAT | Horizontal pad (fraction of screen height) around the text in fit mode. Unset = 0.30 × pill height. |
| `selectorWidth` | carousel | FLOAT | In fit mode this becomes the pill's **maximum** width. |
| `listScroll` | carousel | BOOLEAN | Gamelist-style navigation: the selection walks the visible slots (top → middle → bottom at the ends) and the list scrolls only in between, instead of the selection always sitting centered. See the **Mono Fit** themes. |

## Keyboard press feedback & active shift  ·  `keyboard` · view: `screen`

| Property | Type | Notes |
|---|---|---|
| `shiftActive` / `altActive` | PATH | Alternate icon shown while shift/alt is engaged (e.g. a filled arrow). When set, replaces the legacy red tint. |
| `keyPressColor` | COLOR | Key background flash color on every key press. |
| `keyPressMs` | FLOAT | Flash duration in milliseconds (default 100). |

## Text `padding` on the carousel name  ·  `logoText`

The carousel's `logoText` now honours the standard text `padding` property
(`left top right bottom`, screen fractions) for fine vertical nudges — e.g.
`<padding>0 0 0 0.0041667</padding>` lifts the vertically-centred name 1px.

_These properties are additive and version‑tolerant: unknown properties are
ignored by stock builds, and unset properties fall back to the built‑in
behaviour here, so a theme using them stays usable on either frontend._
