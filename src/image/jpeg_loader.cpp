#include "jpeg_loader.hpp"

#include <cstdio>
#include <cstring>
#include <setjmp.h>
#include <stdexcept>

extern "C" {
#include <jpeglib.h>
}

namespace {

struct ErrorMgr {
    struct jpeg_error_mgr pub;
    jmp_buf jmpbuf;
    char msg[JMSG_LENGTH_MAX];
};

static void on_error(j_common_ptr cinfo) {
    auto* e = reinterpret_cast<ErrorMgr*>(cinfo->err);
    (*cinfo->err->format_message)(cinfo, e->msg);
    longjmp(e->jmpbuf, 1);
}

} // namespace

RgbImage load_jpeg_rgb(const std::filesystem::path& p) {
    FILE* fp = fopen(p.string().c_str(), "rb");
    if (!fp) throw std::runtime_error("Cannot open file: " + p.string());

    struct jpeg_decompress_struct cinfo;
    ErrorMgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = on_error;

    if (setjmp(jerr.jmpbuf)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        throw std::runtime_error(std::string("JPEG decode error: ") + jerr.msg);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);

    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    int w = static_cast<int>(cinfo.output_width);
    int h = static_cast<int>(cinfo.output_height);
    int stride = w * 3;

    RgbImage img;
    img.width  = w;
    img.height = h;
    img.pixels.resize(static_cast<size_t>(h) * stride);

    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row = &img.pixels[cinfo.output_scanline * stride];
        jpeg_read_scanlines(&cinfo, &row, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return img;
}
