# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Goal

`imgbook` — a C++ CLI tool that iteratively builds a pdf photobook or artist book (collection of images that have been layed out, presentation style, in a booklet format).

## Concepts

### The Target Document is for Printing

Our goal is not to make an online pdf, but a printer-friendly, high-resolution inside contents of a book that will be printed, most likely on a high-end printer, perhaps an HP Indigo.

This objective has many repercussions to the design:

### Image Considerations

         1) The images will all be JPEG. This is due to various requirements we need not discuss here. No TIFF or PNG. The program must pre-flight validate this is true for the input.

         2) Because the target is print, we will need to embed a CMYK ICC Color profile into the pdf itself, as a required output intent: --icc-profile|-i <path-to-profile>

         3) Each image will be converted to CMYK (they will start as sRGB) as part of this page build. The images need not have an embedded CMYK ICC profile per image, 
            as this is handled by the pdf container's output intent, but the embedded image itself must have CMYK-ready pixel data.
       
            
            
### Image Organization

To the make process of image organization as dead-simple as possible, we will pass in a folder and take the images in *natural sort order*. 

Where the folder path is provided by a flag:  --folder|-f <path-to-directory>

### Image Embedding and Scaling
           
          

### Layout Considerations

1) Because the target is print, we will need to have some pre-determined layouts that respect the properties/limitations of the printed page:

	Page Mirroring:

	a) The internal contents of a printed book always starts with a single page (page 1) that has a gutter on left side.
	
	b) Succeeding pages alternate as left and right (the gutter is on the right for page 2 and on the left for page 3, and so on). 
	   In other words, odd-numbered pages have a gutter to the left, and even-numbered pages have a gutter to the right.
	   
	c) The internal printed book always has an even number of page content, else the last page would be blank. So It is an error to produce an output that does not 
	   total to an even number of pages. The last page is also always an even-numbered page with a gutter on the right side.
	   
	Validations: We can check the output meets these criteria.
	
	  
	Settable Gutter:
	
	We would want to set the overall gutter to be adjustable via a flag. --gutter|-g <amount>  in units as indicated globally with the --units flag (below)
	
	Units:
	
	The user will want to work either in inches, or in millimeters, but not both. One or the other must be selected.
	We will provide a flag to set the kind of units, these will be used uniformly in all cases: --units|-u (in|mm)
	
	Units only need to be set once for the entire book.
	
	Unit Precision:
	
	Units must be precise to 3 decimal places for inches and 2 decimal places for millimeters, vis., `-p 8.250x11.850 -u in`, or `-p 209.55x300.99 -u mm`.
	
	Validation: user units must be in the required precision or it is an error and we exit with a message.
	
	Page Sizes:
	
	Because the document is the internal part of a book, it should always have a consistent page size, which we will pass in with a flag: --page-size|-p <dimensions>|<standard-name>
	
	We can standard standard page names, such as A4 and Letter, which translate to well-known paper dimensions, for ease of use. We can use "Size" as the name.
	
	ISO 216 A Series Paper Sizes
	Size	mm	inches	points (PDF)	300 dpi (pixels)
	A0	841 × 1189	33.11 × 46.81	2384 × 3371	9933 × 14043
	A1	594 × 841	23.39 × 33.11	1684 × 2384	7016 × 9933
	A2	420 × 594	16.54 × 23.39	1191 × 1684	4961 × 7016
	A3	297 × 420	11.69 × 16.54	842 × 1191	3508 × 4961
	A4	210 × 297	8.27 × 11.69	595 × 842	2480 × 3508
	A5	148 × 210	5.83 × 8.27	420 × 595	1748 × 2480
	A6	105 × 148	4.13 × 5.83	298 × 420	1240 × 1748
	A7	74 × 105	2.91 × 4.13	210 × 298	874 × 1240
	A8	52 × 74	2.05 × 2.91	148 × 210	614 × 874
	A9	37 × 52	1.46 × 2.05	105 × 148	437 × 614
	A10	26 × 37	1.02 × 1.46	74 × 105	307 × 437

	Standard American Paper Sizes
	Size	inches	mm	points (PDF)	300 dpi (pixels)
	Letter	8.5 × 11	216 × 279	612 × 792	2550 × 3300
	Legal	8.5 × 14	216 × 356	612 × 1008	2550 × 4200
	Ledger	11 × 17	279 × 432	792 × 1224	3300 × 5100
	Executive	7.25 × 10.5	184 × 267	522 × 756	2175 × 3150
	Statement	5.5 × 8.5	140 × 216	396 × 612	1650 × 2550
	Folio	8.5 × 13	216 × 330	612 × 936	2550 × 3900
	Quarto	8.5 × 10.83	216 × 275	612 × 780	2550 × 3249
	ANSI-A	8.5 × 11	216 × 279	612 × 792	2550 × 3300
	ANSI-B	11 × 17	279 × 432	792 × 1224	3300 × 5100
	ANSI-C	17 × 22	432 × 559	1224 × 1584	5100 × 6600
	ANSI-D	22 × 34	559 × 864	1584 × 2448	6600 × 10200
	ANSI-E	34 × 44	864 × 1118	2448 × 3168	10200 × 13200

        See @paper-sizes.json for a good machine-readable format that we can integrate as a C++ compiled resource header. 

	
	Layouts:
	
	We will need two basic layouts:
	
	a) a scaled layout, where the image (which will be an arbitrary rectangle) will fit within the page's media box with room to spare. We want the image centered on the page.
	   The scaled layout will have to accomodate for the gutter, adding this value to the centering so that the image is centered with awareness of the gutter.
	
	b) a bleed layout, where the image is expected to fill the entire media box (bleed box). To achieve this, the image may need to be uniformly scaled in both width and height.
	   The Bleed Layout will apply the gutter to avoid cropping the image too much by the binding. So the bleed is really just on the top, outer-side, and bottom.
	
Out of Scope:

The book cover
multi-image layouts

---

## Implementation

### Build System

CMake + pkg-config with system libraries (no vcpkg). All dependencies installed via `dnf`.

```bash
cmake -S . -B build
cmake --build build
```

### Dependencies

| Library | pkg-config name | Version on this system |
|---|---|---|
| PoDoFo | `libpodofo` | 0.9.8 (from EPEL) |
| Little CMS 2 | `lcms2` | 2.12 |
| libjpeg-turbo | `libjpeg` | 2.0.90 |

Install: `sudo dnf install podofo-devel lcms2-devel libjpeg-turbo-devel`

### CLI Flags (all implemented)

```
-f / --folder       input directory (natural sort order)
-o / --output       output PDF path
-p / --page-size    WxH in --units, or standard name (Letter, A4, etc.)
-u / --units        in | mm  (3dp precision for in, 2dp for mm)
-g / --gutter       spine margin
-m / --margin       outer margin for scaled layout (default 0.250 in = 18 pts)
-i / --icc-profile  CMYK ICC profile → PDF/X-3 OutputIntent
-l / --layout       scaled | bleed
```

Tilde expansion is handled in code, so quoted paths like `"~/profiles/foo.icc"` work.

### Key Implementation Decisions

**podofo 0.9.x API** (not 0.10.x): uses `painter.SetPage()`, `painter.FinishPage()`,
`img.LoadFromJpegData()`, `doc.CreatePage(PdfRect(left, bottom, width, height))`,
and `PdfRect` takes `(left, bottom, width, height)` — not `(llx, lly, urx, ury)`.

**CMYK conversion**: lcms2 transform uses `TYPE_CMYK_16_REV` (Adobe-inverted, 16-bit
intermediate) to match libjpeg's `JCS_CMYK` convention (255 = no ink).
Row-by-row streaming: 8-bit RGB → 16-bit RGB (×257) → 16-bit CMYK → 8-bit CMYK.

**Scaled layout**: live area = page minus gutter (spine) and margin (other 3 sides).
Image aspect-ratio preserved, centered in live area.

**Bleed layout**: `--page-size` is the *trim* size. MediaBox expands by `BLEED_PTS`
(9 pts = 0.125 in) on the 3 outer sides. TrimBox `[0 0 page_w page_h]` and
BleedBox (= MediaBox) are written to each page dictionary.

**Odd pages**: gutter left, bleed right/top/bottom.
**Even pages**: gutter right, bleed left/top/bottom.


	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	




