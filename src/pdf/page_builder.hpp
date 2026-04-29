#pragma once
#include "book_writer.hpp"
#include <hpdf.h>

void add_page_image(
    HPDF_Doc  pdf,
    HPDF_Page page,
    const PageSpec& spec,
    double shift_x,
    double shift_y
);
