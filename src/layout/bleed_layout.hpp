#pragma once
#include "layout.hpp"

// Bleed layout: the image fills the full BleedBox (= MediaBox).
// The MediaBox extends BLEED_PTS beyond the trim on three outer sides
// (top, outer edge, bottom); the spine side is held at gutter_pts.
//
// Returns a BleedPlacement containing:
//   image  — draw rect in PDF user space (may have negative y origin)
//   media  — expanded MediaBox (left/bottom/width/height, podofo convention)
//   trim_urx/trim_ury — upper-right of the TrimBox at [0, 0, urx, ury]
BleedPlacement bleed_placement(
    double img_w, double img_h,
    double page_w, double page_h,
    double gutter_pts,
    PageSide side
);
