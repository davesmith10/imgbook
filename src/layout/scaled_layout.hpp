#pragma once
#include "layout.hpp"

// Scaled layout: fit the image within the live area, preserving aspect ratio,
// then center it.  The live area leaves gutter on the spine side and margin_pts
// on the other three sides (top, outer edge, bottom).
//
// img_w/img_h  : source pixel dimensions (aspect ratio only)
// page_w/page_h: page size in PDF points
// gutter_pts   : spine margin
// margin_pts   : outer margin applied to top, bottom, and non-spine edge
// side         : which side carries the spine gutter
PagePlacement scaled_placement(
    double img_w, double img_h,
    double page_w, double page_h,
    double gutter_pts,
    double margin_pts,
    PageSide side
);
