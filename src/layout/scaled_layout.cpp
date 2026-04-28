#include "scaled_layout.hpp"
#include <algorithm>

PagePlacement scaled_placement(
    double img_w, double img_h,
    double page_w, double page_h,
    double gutter_pts,
    double margin_pts,
    PageSide side)
{
    // Live area: spine side gets gutter, other three sides get margin
    double live_x = (side == PageSide::Odd) ? gutter_pts : margin_pts;
    double live_y = margin_pts;
    double live_w = page_w - gutter_pts - margin_pts;
    double live_h = page_h - 2.0 * margin_pts;

    // Scale to fit inside live area (uniform scale, preserve aspect ratio)
    double scale  = std::min(live_w / img_w, live_h / img_h);
    double draw_w = img_w * scale;
    double draw_h = img_h * scale;

    // Center within live area
    double cx = live_x + live_w / 2.0;
    double cy = live_y + live_h / 2.0;

    return PagePlacement{
        cx - draw_w / 2.0,
        cy - draw_h / 2.0,
        draw_w,
        draw_h
    };
}
