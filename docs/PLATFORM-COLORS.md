# Light palette sampled from the platform screenshot

[English](PLATFORM-COLORS.md) | [Русский](PLATFORM-COLORS.ru.md)


On 31 October 2026 the user supplied a screenshot of the Fore editor. The project's earlier light palette used different colors and did not reproduce that reference.

RGB values were sampled programmatically from the original PNG without resizing:

| Text in the reference | Sampled color | Plugin styles |
|---|---|---|
| Identifiers and ordinary code | `#000000` | Default, Identifiers, Operators, Types, Numbers |
| Begin and As keywords | `#008787` | Keywords |
| `//…` comments | `#008000` | Comments, Documentation Comments |
| Strings containing the object identifier and Export | `#800000` | Strings, Characters |

True/False/Null constants use the keyword color; character literals use the string color. Separate colors for numbers, built-in types, constants and character literals are not established by this screenshot: their mapping is a project choice, not a measurement of those platform categories.

The bright red `#FF0000` belongs to the drawn rectangle. The sampled string color is dark red `#800000`.

ClearType and antialiasing introduce many additional edge colors. Repeated interior glyph pixels were sampled. These RGB values are present in the image, but do not prove the numeric values of the platform's settings. Font, scale and antialiasing may differ in Notepad++.

Region coordinates, pixel counts and the original image SHA-256: [palette-sampling.json](../qa/palette-sampling.json). The user's image is not distributed in the project archive.

Both the light XML preset and its embedded DLL copy were updated.

## Adaptation for a dark background

The dark palette keeps the teal, green and red roles while increasing brightness for `#1E1E1E`. Ordinary black text becomes light gray. This is an adaptation, not a measurement of the platform's own dark mode.

| Category | Color | Contrast against `#1E1E1E` |
|---|---|---|
| Ordinary code, types, numbers, operators | `#D4D4D4` | 11.25:1 |
| Keywords and constants | `#4FD1D1` | 9.03:1 |
| Comments | `#6EC76E` | 7.98:1 |
| Strings and character literals | `#F08080` | 6.43:1 |

Contrast is calculated from relative sRGB luminance; comparison with 4.5:1 uses unrounded values. This ordinary-text reference is described in [WCAG 2.2, Contrast Minimum](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html). It checks color pairs; it does not certify the entire application as WCAG-conformant. All 11 categories and the preset SHA-256: [dark-palette-contrast.json](../qa/dark-palette-contrast.json).

The background is inherited from the Notepad++ theme. The plugin selects Light/Dark from the editor background: [automatic themes](AUTOMATIC-THEMES.md). Recalculate these ratios for other backgrounds.
