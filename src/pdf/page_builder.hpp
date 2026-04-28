#pragma once
#include "book_writer.hpp"
#include <podofo/podofo.h>

void add_page_image(
    PoDoFo::PdfMemDocument& doc,
    PoDoFo::PdfPage& page,
    const PageSpec& spec
);
