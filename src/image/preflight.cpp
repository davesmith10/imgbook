#include "preflight.hpp"

#include <csetjmp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}

bool has_jpeg_extension(const std::filesystem::path& p) {
    std::string ext = p.extension().string();
    // Lowercase comparison
    for (auto& c : ext) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return ext == ".jpg" || ext == ".jpeg";
}

bool has_jpeg_magic(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    unsigned char buf[3] = {};
    f.read(reinterpret_cast<char*>(buf), 3);
    return buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF;
}

// Silent libjpeg error handler — we just want success/failure, no stderr noise.
struct SilentErrorMgr {
    struct jpeg_error_mgr pub;
    jmp_buf jmpbuf;
};

static void silent_error_exit(j_common_ptr cinfo) {
    auto* myerr = reinterpret_cast<SilentErrorMgr*>(cinfo->err);
    longjmp(myerr->jmpbuf, 1);
}

bool read_jpeg_header(const std::filesystem::path& p, int& width, int& height) {
    FILE* fp = fopen(p.string().c_str(), "rb");
    if (!fp) return false;

    struct jpeg_decompress_struct cinfo;
    SilentErrorMgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = silent_error_exit;

    if (setjmp(jerr.jmpbuf)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);

    width  = static_cast<int>(cinfo.image_width);
    height = static_cast<int>(cinfo.image_height);

    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    return true;
}

bool preflight_jpeg(const std::filesystem::path& p, ImageInfo& out) {
    if (!has_jpeg_extension(p)) {
        std::cerr << "Error: '" << p.filename().string()
                  << "' does not have a .jpg/.jpeg extension.\n";
        return false;
    }
    if (!has_jpeg_magic(p)) {
        std::cerr << "Error: '" << p.filename().string()
                  << "' is not a valid JPEG (bad magic bytes).\n";
        return false;
    }
    int w = 0, h = 0;
    if (!read_jpeg_header(p, w, h)) {
        std::cerr << "Error: '" << p.filename().string()
                  << "' could not be read as a JPEG image.\n";
        return false;
    }
    out = ImageInfo{ w, h, p };
    return true;
}
