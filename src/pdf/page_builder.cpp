#include "page_builder.hpp"
#include <stdexcept>

void add_page_image(HPDF_Doc pdf, HPDF_Page page,
                    const PageSpec& spec, double shift_x, double shift_y)
{
    HPDF_Image img = HPDF_LoadJpegImageFromMem(
        pdf,
        reinterpret_cast<const HPDF_BYTE*>(spec.cmyk_jpeg.data()),
        static_cast<HPDF_UINT>(spec.cmyk_jpeg.size()));
    if (!img)
        throw std::runtime_error("libharu: failed to load JPEG for page "
                                 + std::to_string(spec.page_number));

    const auto& p = spec.placement;
    HPDF_Page_DrawImage(page, img,
        static_cast<HPDF_REAL>(p.x + shift_x),
        static_cast<HPDF_REAL>(p.y + shift_y),
        static_cast<HPDF_REAL>(p.w),
        static_cast<HPDF_REAL>(p.h));
}
