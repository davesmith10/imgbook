# imgbook

A C++ CLI tool that builds a print-ready PDF photobook from a folder of JPEG images.

Designed for professional print output (HP Indigo and similar), not screen viewing. Produces output with a CMYK ICC output intent, proper page gutter mirroring, and two layout modes.

---

## Features

- **CMYK output** — images are converted from sRGB to CMYK via lcms2 (16-bit precision, perceptual intent) and embedded as CMYK JPEG
- **ICC OutputIntent** — the supplied CMYK ICC profile is embedded as the PDF OutputIntent, satisfying prepress requirements
- **Gutter mirroring** — odd pages carry the spine gutter on the left, even pages on the right
- **Scaled layout** — image fits within the live area with configurable outer margins on all sides
- **Bleed layout** — image cover-scales to fill the full BleedBox; TrimBox and BleedBox are embedded; spine bleed is configurable
- **Natural sort order** — images are ordered the way a human would expect (img2 before img10)
- **Pre-flight validation** — all inputs are checked (JPEG format, even page count, unit precision) before any processing begins

---

## Dependencies

| Library | Purpose | Install |
|---|---|---|
| [libharu](http://libharu.org/) 2.3 | PDF generation | `dnf install libharu-devel` |
| [Little CMS 2](https://www.littlecms.com/) | ICC color transforms | `dnf install lcms2-devel` |
| [libjpeg-turbo](https://libjpeg-turbo.org/) | JPEG decode/encode | `dnf install libjpeg-turbo-devel` |

lcms2 and libjpeg-turbo are detected via `pkg-config`. libharu ships no pkg-config file on this platform and is linked directly with `-lhpdf`.

---

## Build

```bash
cmake -S . -B build
cmake --build build
./build/imgbook --help
```

Requires CMake ≥ 3.16, a C++17 compiler, and a C11 compiler (one C source file uses internal libharu headers — see [Implementation Notes](#implementation-notes) below).

---

## Usage

```
imgbook -f <folder> -o <output.pdf> -p <page-size> -u <units>
        -g <gutter> -i <icc-profile> -l <layout> [-m <margin>] [--full-bleed]
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
| `-i`, `--icc-profile` | CMYK ICC profile path — embedded as the PDF OutputIntent |
| `-l`, `--layout` | `scaled` or `bleed` — see Layout Modes below |
| `--full-bleed` | Extend bleed to all 4 sides including the spine (requires `-l bleed`) |

### Standard page size names

`A0`–`A10`, `Letter`, `Legal`, `Ledger`, `Executive`, `Statement`, `Folio`, `Quarto`, `ANSI-A`–`ANSI-E`

### Examples

```bash
# Scaled layout — images centered with margins, Letter size
imgbook -f ./photos -p Letter -u in -g 0.500 -m 0.250 -l scaled \
        -i SWOP2006_Coated3v2.icc -o book.pdf

# Bleed layout — 3-sided bleed (spine flush), A4 size
imgbook -f ./photos -p A4 -u mm -g 12.70 -l bleed \
        -i SWOP2006_Coated3v2.icc -o book_bleed.pdf

# Bleed layout — 4-sided bleed (spine included), for KDP / full-bleed POD services
imgbook -f ./photos -p Letter -u in -g 0.500 -l bleed --full-bleed \
        -i SWOP2006_Coated3v2.icc -o book_fullbleed.pdf

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

The stated `--page-size` is the *trim* size. The PDF MediaBox is expanded by 0.125 in (9 pts) to create a bleed zone. The image is cover-scaled to fill the full BleedBox. TrimBox and BleedBox are written to each page dictionary.

**3-sided bleed (default):** bleed extends on the top, outer edge, and bottom. The spine side stays flush at the trim edge — appropriate for offset printing where the binding fold is physical and the spine bleed serves no purpose.

**4-sided bleed (`--full-bleed`):** bleed extends on all four sides including the spine. The image fills the entire BleedBox with no gutter offset. Required by some POD services (KDP, IngramSpark) that handle spine clearance mathematically during imposition.

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
(odd page, 3-sided bleed, gutter left)
```

---

## Validation

imgbook pre-flights all inputs before writing any output:

- All files in `--folder` must be valid JPEGs (extension + magic bytes + libjpeg header)
- Image count must be even (books require full spreads)
- Unit values must match the required decimal precision (`3dp` for inches, `2dp` for mm)
- `--page-size` must be a known standard name or a correctly formatted `WxH` string
- `--icc-profile` must exist and describe a CMYK color space
- `--full-bleed` requires `-l bleed`

---

## Implementation Notes

### PDF library: libharu

imgbook uses [libharu 2.3](http://libharu.org/) for PDF generation. libharu was chosen over PoDoFo for its native TrueType font embedding support, which is needed for future text overlay features.

### TrimBox and BleedBox: working around a libharu limitation

libharu was designed for digital document generation (invoices, reports, forms), not for prepress workflows. As a result, `HPDF_Page_SetBoxValue` — the public API function for writing page box entries — only operates on boxes that already exist in the page dictionary. MediaBox is always present (libharu creates it when a page is added), but TrimBox and BleedBox are prepress-only concepts that libharu never creates automatically.

Calling `HPDF_Page_SetBoxValue(page, "TrimBox", ...)` returns `HPDF_PAGE_CANNOT_FIND_OBJECT` and silently records an error flag in the document. That flag causes `HPDF_SaveToFile` to subsequently fail with `HPDF_INVALID_DOCUMENT`. The public API provides no other mechanism to write these entries.

**The workaround** is in `src/pdf/hpdf_box_inject.c`. libharu's internal headers (installed with the devel package) expose `HPDF_Box_Array_New` and `HPDF_Dict_Add`, which together can write any array-valued entry into any page dictionary. Internally, `HPDF_Page` is simply `HPDF_Dict` (`HPDF_Dict_Rec*`) — the same type used for all dictionary objects — so `HPDF_Dict_Add(page, "TrimBox", array)` works correctly.

This file must be compiled as **C** (not C++) and must **not** include `hpdf.h`. The reason: in the public header, `HPDF_Dict` is `typedef HPDF_HANDLE` (i.e., `void*`), while in the internal header `hpdf_objects.h` it is `typedef struct _HPDF_Dict_Rec*`. Including both in one translation unit produces a typedef conflict. The C file includes only the internal headers; the C++ PDF layer calls it via a plain C function declared in `hpdf_box_inject.h`.

The capability was always present inside the library — it was simply never surfaced in the public API for print use cases.

---

## License

MIT — see [LICENSE](LICENSE)
