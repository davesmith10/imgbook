#include "page_builder.hpp"

#include <stdexcept>

using namespace PoDoFo;

void add_page_image(
    PdfMemDocument& doc,
    PdfPage& page,
    const PageSpec& spec)
{
    PdfImage img(&doc);
    img.LoadFromJpegData(
        reinterpret_cast<const unsigned char*>(spec.cmyk_jpeg.data()),
        static_cast<pdf_long>(spec.cmyk_jpeg.size()));

    // img.GetWidth()/GetHeight() returns the image's bounding box size in user units,
    // which equals the pixel dimensions after LoadFromJpegData.
    // DrawImage scale = desired_pts / natural_units.
    const auto& p = spec.placement;
    double scale_x = p.w / img.GetWidth();
    double scale_y = p.h / img.GetHeight();

    PdfPainter painter;
    painter.SetPage(&page);
    painter.DrawImage(p.x, p.y, &img, scale_x, scale_y);
    painter.FinishPage();
}
