#include "bleed_layout.hpp"
#include <algorithm>

BleedPlacement bleed_placement(
    double img_w, double img_h,
    double page_w, double page_h,
    double gutter_pts,
    PageSide side)
{
    const double bleed = BLEED_PTS;

    // MediaBox: expand on 3 outer sides; spine side stays at its trim edge.
    // podofo PdfRect(left, bottom, width, height).
    MediaBox media;
    if (side == PageSide::Odd) {   // spine left, outer right
        media.left   =  0.0;
        media.bottom = -bleed;
        media.width  =  page_w + bleed;
        media.height =  page_h + 2.0 * bleed;
    } else {                       // spine right, outer left
        media.left   = -bleed;
        media.bottom = -bleed;
        media.width  =  page_w + bleed;
        media.height =  page_h + 2.0 * bleed;
    }

    // Image fills the full bleed area, with gutter clearance on the spine side.
    // area origin (in PDF user-space) and dimensions:
    double area_x = (side == PageSide::Odd) ? gutter_pts : (media.left);
    double area_y = media.bottom;
    double area_w = (side == PageSide::Odd)
                    ? (media.left + media.width - gutter_pts)   // right bleed edge − gutter
                    : (page_w - gutter_pts - media.left);        // trim right − gutter + left bleed
    double area_h = media.height;

    // Cover-scale: fill the area entirely (image may be cropped on one axis)
    double scale  = std::max(area_w / img_w, area_h / img_h);
    double draw_w = img_w * scale;
    double draw_h = img_h * scale;

    // Center within the bleed area
    double cx = area_x + area_w / 2.0;
    double cy = area_y + area_h / 2.0;

    PagePlacement image{
        cx - draw_w / 2.0,
        cy - draw_h / 2.0,
        draw_w,
        draw_h
    };

    // TrimBox lower-left is always (0,0); upper-right is the trim page size
    return BleedPlacement{ image, media, page_w, page_h };
}
