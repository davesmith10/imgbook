#pragma once
#include <filesystem>
#include <string>

struct ImageInfo {
    int width;
    int height;
    std::filesystem::path path;
};

// Returns true if the file has a .jpg/.jpeg extension (case-insensitive).
bool has_jpeg_extension(const std::filesystem::path& p);

// Checks that the file starts with FF D8 FF (JPEG magic bytes).
bool has_jpeg_magic(const std::filesystem::path& p);

// Reads the JPEG header with libjpeg and fills in width/height.
// Returns false if the file is not a valid JPEG.
bool read_jpeg_header(const std::filesystem::path& p, int& width, int& height);

// Full pre-flight check: extension + magic + valid header.
// On failure, prints an error message to stderr and returns false.
bool preflight_jpeg(const std::filesystem::path& p, ImageInfo& out);
