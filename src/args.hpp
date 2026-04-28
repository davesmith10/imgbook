#pragma once
#include <filesystem>
#include <string>

enum class Layout { Scaled, Bleed };
enum class Units  { Inches, MM };

struct Config {
    std::filesystem::path folder;
    std::filesystem::path output;
    std::filesystem::path icc_profile;
    double page_width_pts;   // PDF user units (points)
    double page_height_pts;
    double gutter_pts;
    double margin_pts; // outer margin (top/bottom/non-spine) for scaled layout
    Layout layout;
    Units  units;
};

// Parses argv, validates all inputs, converts measurements to PDF points.
// Prints an error message and calls std::exit(1) on any validation failure.
Config parse_args(int argc, char** argv);
