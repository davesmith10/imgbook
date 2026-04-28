#include "cmyk_convert.hpp"

#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <lcms2.h>
#include <jpeglib.h>

namespace {

struct JpegErrorMgr {
    struct jpeg_error_mgr pub; // MUST be first
    jmp_buf jmpbuf;
    char msg[JMSG_LENGTH_MAX];
};

static void on_jpeg_error(j_common_ptr cinfo) {
    auto* e = reinterpret_cast<JpegErrorMgr*>(cinfo->err);
    (*cinfo->err->format_message)(cinfo, e->msg);
    longjmp(e->jmpbuf, 1);
}

struct CompressGuard {
    jpeg_compress_struct cinfo{};
    bool created = false;
    ~CompressGuard() { if (created) jpeg_destroy_compress(&cinfo); }
};

struct CmsProfileGuard {
    cmsHPROFILE handle = nullptr;
    ~CmsProfileGuard() { if (handle) cmsCloseProfile(handle); }
};

struct CmsTransformGuard {
    cmsHTRANSFORM handle = nullptr;
    ~CmsTransformGuard() { if (handle) cmsDeleteTransform(handle); }
};

static void lcms_error_fn(cmsContext, cmsUInt32Number, const char* text) {
    throw std::runtime_error(std::string("lcms2: ") + text);
}

} // namespace

std::vector<uint8_t> rgb_to_cmyk_jpeg(
    const RgbImage& src,
    const std::filesystem::path& icc_profile,
    int quality)
{
    cmsSetLogErrorHandler(lcms_error_fn);

    // --- Build sRGB → CMYK transform (16-bit for higher precision) ---
    CmsProfileGuard srgb_prof, cmyk_prof;
    srgb_prof.handle = cmsCreate_sRGBProfile();
    if (!srgb_prof.handle)
        throw std::runtime_error("lcms2: cannot create built-in sRGB profile");

    cmyk_prof.handle = cmsOpenProfileFromFile(icc_profile.string().c_str(), "r");
    if (!cmyk_prof.handle)
        throw std::runtime_error("lcms2: cannot open ICC profile: " + icc_profile.string());

    if (cmsGetColorSpace(cmyk_prof.handle) != cmsSigCmykData)
        throw std::runtime_error("ICC profile does not describe a CMYK color space: "
                                 + icc_profile.string());

    CmsTransformGuard xform;
    // TYPE_CMYK_16_REV: 16-bit, Adobe-inverted (65535 = no ink), which matches
    // libjpeg's JCS_CMYK convention (255 = no ink after downshift to 8-bit).
    xform.handle = cmsCreateTransform(
        srgb_prof.handle, TYPE_RGB_16,
        cmyk_prof.handle, TYPE_CMYK_16_REV,
        INTENT_PERCEPTUAL,
        cmsFLAGS_BLACKPOINTCOMPENSATION);
    if (!xform.handle)
        throw std::runtime_error("lcms2: failed to create color transform");

    uint32_t width  = static_cast<uint32_t>(src.width);
    uint32_t height = static_cast<uint32_t>(src.height);

    // --- Compress CMYK → JPEG in memory ---
    CompressGuard cctx;
    JpegErrorMgr  jerr;
    cctx.cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = on_jpeg_error;

    unsigned char* out_buf  = nullptr;
    unsigned long  out_size = 0;

    if (setjmp(jerr.jmpbuf)) {
        if (out_buf) free(out_buf);
        throw std::runtime_error(std::string("JPEG compress error: ") + jerr.msg);
    }

    jpeg_create_compress(&cctx.cinfo);
    cctx.created = true;
    jpeg_mem_dest(&cctx.cinfo, &out_buf, &out_size);

    cctx.cinfo.image_width      = width;
    cctx.cinfo.image_height     = height;
    cctx.cinfo.input_components = 4;
    cctx.cinfo.in_color_space   = JCS_CMYK;

    jpeg_set_defaults(&cctx.cinfo);
    jpeg_set_colorspace(&cctx.cinfo, JCS_CMYK);
    jpeg_set_quality(&cctx.cinfo, quality, TRUE);
    cctx.cinfo.optimize_coding = TRUE;

    jpeg_start_compress(&cctx.cinfo, TRUE);

    // Row-by-row: 8-bit RGB → 16-bit RGB → 16-bit CMYK (inverted) → 8-bit CMYK
    std::vector<uint16_t> rgb_row_16(width * 3);
    std::vector<uint16_t> cmyk_row_16(width * 4);
    std::vector<uint8_t>  cmyk_row(width * 4);
    JSAMPROW cmyk_ptr = cmyk_row.data();

    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* rgb8 = &src.pixels[static_cast<size_t>(y) * width * 3];

        // Upsample 8-bit → 16-bit (×257 = 65535/255, lossless round-trip)
        for (uint32_t i = 0; i < width * 3; ++i)
            rgb_row_16[i] = static_cast<uint16_t>(rgb8[i] * 257u);

        cmsDoTransform(xform.handle, rgb_row_16.data(), cmyk_row_16.data(), width);

        // Downsample 16-bit → 8-bit
        for (uint32_t i = 0; i < width * 4; ++i)
            cmyk_row[i] = static_cast<uint8_t>(cmyk_row_16[i] >> 8);

        jpeg_write_scanlines(&cctx.cinfo, &cmyk_ptr, 1);
    }

    jpeg_finish_compress(&cctx.cinfo);

    std::vector<uint8_t> result(out_buf, out_buf + out_size);
    free(out_buf);
    return result;
}
