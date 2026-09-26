# Theme source

The six Mono themes are built from one base plus a delta each. Edit here, run
`build.py build`, and the change lands in all six.

```
base.xml            Mono Dark, verbatim -- the canonical theme
<Theme>.delta       what that theme changes about it
build.py            extract | build | check
```

## Why generate rather than share a file

EmulationStation cannot share a file between theme sets, for three separate
reasons, so every theme.xml has to ship self-contained:

- every directory under the themes path becomes a theme set, with no check for
  a theme.xml and hidden directories included, so a `_shared/` folder would
  appear in the Theme Set picker as a broken entry (`ThemeData.cpp`,
  `getThemeSets`)
- `<path>` values resolve against the file being parsed, not the theme root, so
  a shared file's `./art/x.svg` points into the shared folder (`ThemeData.cpp`,
  `case PATH`)
- a missing `<include>` logs a warning and returns, leaving the theme with no
  base at all rather than failing loudly -- so a theme installed on its own
  would render blank, and themes are installed one folder at a time
  (`ThemeData.cpp`, `parseInclude`)

Generating sidesteps all three: the source has one copy, the shipped themes
have none of the coupling.

## Commands

```sh
python3 build.py check      # rebuild in memory, diff against disk (exit 1 on drift)
python3 build.py build      # write ../theme/<Theme>/theme.xml
python3 build.py extract    # the other direction: hand-edited themes -> deltas
```

`check` is the one to run before shipping. `extract` is for when it is easier to
edit a theme.xml directly and fold the result back in; it is exact, so
extract-then-build is a no-op.

## Delta format

A property is addressed by the element that encloses it -- views are ignored,
since the element names are specific enough on their own:

```
[carousel systemcarousel]
selectorColor = 000000FF          ## black capsule on the paper ground
```

Everything after `##` is the XML comment written on the line. No `##` means no
comment: the delta fully specifies the line, so leaving it off strips the
base's comment rather than keeping it.

| form | meaning |
|---|---|
| `[text logoText #1]` | the second `logoText`; without an index, every occurrence |
| `prop = value @after other` | place a property the base lacks, after `other` |
| `prop = @remove` | drop the line |
| `[remove subset color-scheme]` | drop a whole element, as Mono Classic does |
| `[replace carousel systemcarousel logoSize]` … `[end]` | raw lines, for a multi-line trailing comment |

## Keep the deltas honest

A delta entry whose value matches the base and differs only in its comment
still blocks a base edit from reaching that theme. Two of those had already
accumulated in Mono Classic, which is why its help pill did not move when the
other five did. Before adding one, ask whether the comment is saying something
true of that variant -- Mono Fit's "max pill width in fit mode" earns its
entry; a stale "v2:" prefix does not.
