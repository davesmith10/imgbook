#pragma once
#include <filesystem>
#include <vector>
#include <cstdint>
#include "jpeg_loader.hpp"

// Converts an sRGB image to CMYK using the given ICC profile (lcms2),
// then re-encodes the CMYK pixel data as a JPEG stored in memory.
// quality: 1-100 (recommend 90 for print).
// Throws std::runtime_error on failure.
std::vector<uint8_t> rgb_to_cmyk_jpeg(
    const RgbImage& src,
    const std::filesystem::path& icc_profile,
    int quality = 90
);
