# imgbook

A C++ CLI tool that builds a print-ready PDF photobook from a folder of JPEG images.

Designed for professional print output (HP Indigo and similar), not screen viewing. Produces PDF/X-3 compliant output with a CMYK ICC output intent, proper page gutter mirroring, and two layout modes.

---

## Features

- **PDF/X-3 output** — embedded ICC output intent, PDF 1.3
- **CMYK conversion** — images are converted from sRGB to CMYK via lcms2 (16-bit precision, perceptual intent)
- **Gutter mirroring** — odd pages have spine gutter on the left, even pages on the right
- **Scaled layout** — image fits within the live area with configurable outer margins on all sides
- **Bleed layout** — image fills the full BleedBox; MediaBox is expanded by 0.125 in on the three outer sides; TrimBox and BleedBox are embedded in the PDF
- **Natural sort order** — images are ordered the way a human would expect (img2 before img10)
- **Pre-flight validation** — all inputs are checked (JPEG format, even page count, unit precision) before any processing begins

---

## Dependencies

| Library | Purpose | Install |
|---|---|---|
| [PoDoFo](http://podofo.sourceforge.net/) 0.9.x | PDF generation | `dnf install podofo-devel` |
| [Little CMS 2](https://www.littlecms.com/) | ICC color transforms | `dnf install lcms2-devel` |
| [libjpeg-turbo](https://libjpeg-turbo.org/) | JPEG decode/encode | `dnf install libjpeg-turbo-devel` |

All three are detected via `pkg-config`.

---

## Build

```bash
cmake -S . -B build
cmake --build build
./build/imgbook --help
```

Requires CMake ≥ 3.16 and a C++17 compiler.

---

## Usage

```
imgbook -f <folder> -o <output.pdf> -p <page-size> -u <units>
        -g <gutter> -i <icc-profile> -l <layout> [-m <margin>]
```

### Flags

| Flag | Description |
|---|---|
| `-f`, `--folder` | Directory of input JPEG images (processed in natural sort order) |
| `-o`, `--output` | Output PDF path |
| `-p`, `--page-size` | Page size as `WxH` in `--units`, or a standard name — see below |
| `-u`, `--units` | `in` or `mm`. Inches require 3 decimal places; mm require 2 |
| `-g`, `--gutter` | Spine margin in `--units` |
| `-m`, `--margin` | Outer margin (top/bottom/non-spine edge) for scaled layout. Default: `0.250 in` / `6.35 mm` |
| `-i`, `--icc-profile` | CMYK ICC profile path — embedded as the PDF/X-3 OutputIntent |
| `-l`, `--layout` | `scaled` or `bleed` — see Layout Modes below |

### Standard page size names

`A0`–`A10`, `Letter`, `Legal`, `Ledger`, `Executive`, `Statement`, `Folio`, `Quarto`, `ANSI-A`–`ANSI-E`

### Examples

```bash
# Scaled layout — images centered with margins, Letter size
imgbook -f ./photos -p Letter -u in -g 0.500 -m 0.250 -l scaled \
        -i SWOP2006_Coated3v2.icc -o book.pdf

# Bleed layout — images fill to the bleed edge, A4 size
imgbook -f ./photos -p A4 -u mm -g 12.70 -l bleed \
        -i SWOP2006_Coated3v2.icc -o book_bleed.pdf

# Custom page size in mm
imgbook -f ./photos -p 148.00x210.00 -u mm -g 10.00 -l scaled \
        -i PSOcoated_v3.icc -o book_custom.pdf
```

Tilde paths are supported in quoted arguments (e.g. `"~/profiles/CMYK.icc"`).

---

## Layout Modes

### Scaled

The image is scaled to fit entirely within the *live area* — the page minus spine gutter and outer margins — and centered within it. No part of the image reaches the page edge.

```
┌─────────────────────────┐
│  margin                 │
│  ┌───────────────────┐  │
│g │                   │  │
│u │     [ image ]     │m │
│t │                   │a │
│t │                   │r │
│e └───────────────────┘g │
│  margin                 │
└─────────────────────────┘
(odd page, gutter left)
```

### Bleed

The stated `--page-size` is treated as the *trim* size. The PDF MediaBox is expanded by 0.125 in (3.2 mm) on the three outer sides (top, outer edge, bottom). The image is cover-scaled to fill the full BleedBox. The spine side receives the gutter inset. TrimBox and BleedBox are written to each page.

```
┌───────────────────────────┐  ← BleedBox / MediaBox
│          bleed            │
│  ┌─────────────────────┐  │  ← TrimBox
│  │g                    │  │
│  │u    [ image ]       │  │
│  │t                    │  │
│  └─────────────────────┘  │
│          bleed            │
└───────────────────────────┘
(odd page, gutter left)
```

---

## Validation

imgbook pre-flights all inputs before writing any output:

- All files in `--folder` must be valid JPEGs (extension + magic bytes + libjpeg header)
- Image count must be even (books require full spreads)
- Unit values must match the required decimal precision (`3dp` for inches, `2dp` for mm)
- `--page-size` must be a known standard name or a correctly formatted `WxH` string
- `--icc-profile` must exist and describe a CMYK color space

---

## License

MIT — see [LICENSE](LICENSE)
