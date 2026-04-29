#pragma once
#include <filesystem>
#include <vector>
#include <cstdint>
#include "../layout/layout.hpp"

struct PageSpec {
    std::vector<uint8_t> cmyk_jpeg;
    int img_pixel_w;
    int img_pixel_h;
    int page_number;                 // 1-based
    PagePlacement placement;
    // MediaBox (left, bottom, width, height) — origin may be negative for bleed pages
    MediaBox media;
    // TrimBox — only present for bleed layout
    bool   has_trim_box = false;
    double trim_urx     = 0.0;      // TrimBox upper-right x (lower-left is always 0,0)
    double trim_ury     = 0.0;      // TrimBox upper-right y
};

void write_book(
    const std::filesystem::path& output_path,
    const std::filesystem::path& icc_profile,
    const std::vector<PageSpec>& pages
);
