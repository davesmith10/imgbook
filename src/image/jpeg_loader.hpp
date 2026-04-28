#pragma once
#include <filesystem>
#include <vector>
#include <cstdint>

struct RgbImage {
    std::vector<uint8_t> pixels; // width * height * 3 bytes, row-major, R G B
    int width;
    int height;
};

// Decodes a JPEG file to raw 8-bit RGB pixel data.
// Throws std::runtime_error on failure.
RgbImage load_jpeg_rgb(const std::filesystem::path& p);
