#pragma once

// All coordinates are in PDF points (1/72 inch), origin at lower-left.

static constexpr double BLEED_PTS = 9.0; // 0.125 in — KDP standard bleed extension

enum class PageSide { Odd, Even }; // Odd = gutter left, Even = gutter right

struct PagePlacement {
    double x; // lower-left x of image draw rect
    double y; // lower-left y of image draw rect
    double w; // draw width  in points
    double h; // draw height in points
};

// Media box: origin (left, bottom) and size (width, height), all in PDF points.
// For scaled layout this is (0, 0, page_w, page_h).
// For bleed layout the origin may be negative to denote bleed extension beyond the trim edge.
struct MediaBox {
    double left   = 0.0;
    double bottom = 0.0;
    double width  = 0.0;
    double height = 0.0;
};

struct BleedPlacement {
    PagePlacement image;
    MediaBox      media;
    // TrimBox corners in PDF user space (lower-left is always 0,0)
    double trim_urx = 0.0;
    double trim_ury = 0.0;
};

inline PageSide side_for_page(int one_based_page) {
    return (one_based_page % 2 == 1) ? PageSide::Odd : PageSide::Even;
}
