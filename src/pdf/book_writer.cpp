#include "book_writer.hpp"
#include "page_builder.hpp"

#include <csetjmp>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <hpdf.h>
#include <hpdf_pdfa.h>        // HPDF_PDFA_AppendOutputIntents
#include "hpdf_box_inject.h"  // hpdf_inject_box (TrimBox / BleedBox)

namespace fs = std::filesystem;

static thread_local jmp_buf s_jmpbuf;
static thread_local char    s_errmsg[256];

static void hpdf_err(HPDF_STATUS err, HPDF_STATUS detail, void*) {
    std::snprintf(s_errmsg, sizeof s_errmsg,
                  "libharu error 0x%04X detail 0x%04X",
                  static_cast<unsigned>(err), static_cast<unsigned>(detail));
    longjmp(s_jmpbuf, 1);
}

static void set_box(HPDF_Page page, const char* name,
                    double llx, double lly, double urx, double ury)
{
    hpdf_inject_box(page, name,
                    static_cast<float>(llx), static_cast<float>(lly),
                    static_cast<float>(urx), static_cast<float>(ury));
}

void write_book(
    const fs::path& output_path,
    const fs::path& icc_profile,
    const std::vector<PageSpec>& pages)
{
    HPDF_Doc pdf = HPDF_New(hpdf_err, nullptr);
    if (!pdf)
        throw std::runtime_error("libharu: HPDF_New failed");

    if (setjmp(s_jmpbuf)) {
        HPDF_Free(pdf);
        throw std::runtime_error(s_errmsg);
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    HPDF_SetInfoAttr(pdf, HPDF_INFO_CREATOR,  "imgbook 0.1.0");
    HPDF_SetInfoAttr(pdf, HPDF_INFO_PRODUCER, "imgbook 0.1.0 / libharu 2.3");

    // Embed CMYK ICC profile as PDF OutputIntent
    HPDF_OutputIntent intent =
        HPDF_LoadIccProfileFromFile(pdf, icc_profile.string().c_str(), 4);
    HPDF_PDFA_AppendOutputIntents(pdf, "Custom CMYK",
                                  reinterpret_cast<HPDF_Dict>(intent));

    for (const auto& spec : pages) {
        const auto& m = spec.media;

        // Shift so that all coordinates are non-negative (libharu requires (0,0) origin).
        // media.left and media.bottom may be negative for bleed pages.
        double shift_x = -m.left;
        double shift_y = -m.bottom;

        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth (page, static_cast<HPDF_REAL>(m.width));
        HPDF_Page_SetHeight(page, static_cast<HPDF_REAL>(m.height));

        if (spec.has_trim_box) {
            // TrimBox in shifted PDF user space: [shift_x, shift_y, shift_x+trim_w, shift_y+trim_h]
            set_box(page, "TrimBox",
                    shift_x,               shift_y,
                    shift_x + spec.trim_urx, shift_y + spec.trim_ury);
            // BleedBox equals the full MediaBox
            set_box(page, "BleedBox", 0.0, 0.0, m.width, m.height);
        }

        add_page_image(pdf, page, spec, shift_x, shift_y);
    }

    HPDF_SaveToFile(pdf, output_path.string().c_str());
    HPDF_Free(pdf);
}
