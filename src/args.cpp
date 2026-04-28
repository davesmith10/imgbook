#include "args.hpp"
#include "paper_sizes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <pwd.h>
#include <string>
#include <unistd.h>

static std::string expand_tilde(const std::string& p) {
    if (p.empty() || p[0] != '~') return p;
    if (p.size() > 1 && p[1] != '/') return p;
    const char* home = getenv("HOME");
    if (!home) {
        if (const struct passwd* pw = getpwuid(getuid()))
            home = pw->pw_dir;
    }
    if (!home) return p;
    return std::string(home) + p.substr(1);
}

static constexpr double PTS_PER_INCH = 72.0;
static constexpr double PTS_PER_MM   = 72.0 / 25.4;
// Default outer margin (KDP minimum for no-bleed): 0.250 in = 18 pts
static constexpr double DEFAULT_MARGIN_PTS = 18.0;

static int decimal_places(const std::string& s) {
    auto dot = s.find('.');
    if (dot == std::string::npos) return 0;
    return static_cast<int>(s.size() - dot - 1);
}

static bool parse_dimension_string(const std::string& raw,
                                   double& w, double& h,
                                   std::string& raw_w, std::string& raw_h) {
    auto pos = raw.find('x');
    if (pos == std::string::npos) return false;
    raw_w = raw.substr(0, pos);
    raw_h = raw.substr(pos + 1);
    try {
        size_t iw, ih;
        w = std::stod(raw_w, &iw);
        if (iw != raw_w.size()) return false;
        h = std::stod(raw_h, &ih);
        if (ih != raw_h.size()) return false;
    } catch (...) {
        return false;
    }
    return true;
}

static void usage(const char* prog) {
    std::cerr <<
        "Usage: " << prog << "\n"
        "  -f, --folder      <path>          Input directory of JPEG images\n"
        "  -o, --output      <file.pdf>       Output PDF path\n"
        "  -p, --page-size   <WxH>|<name>    Page size in --units or standard name (Letter, A4 ...)\n"
        "  -u, --units       in|mm           Unit system for all measurements\n"
        "  -g, --gutter      <amount>         Spine gutter margin in --units\n"
        "  -m, --margin      <amount>         Outer margin (top/bottom/edge) for scaled layout\n"
        "                                     Default: 0.250 in / 6.35 mm\n"
        "  -i, --icc-profile <path>           CMYK ICC profile (embedded as PDF/X-3 OutputIntent)\n"
        "  -l, --layout      scaled|bleed     Image placement mode\n"
        "\n"
        "Example:\n"
        "  imgbook -f ./photos -p Letter -u in -g 0.500 -m 0.250 -l scaled -i CGATS.icc -o book.pdf\n";
}

// Parse and validate a measurement string; return value in user units.
static double parse_measurement(const std::string& val, const char* flag_name,
                                int required_dp, const std::string& units_str) {
    size_t idx;
    double v;
    try { v = std::stod(val, &idx); }
    catch (...) {
        std::cerr << "Error: " << flag_name << " value '" << val
                  << "' is not a valid number.\n";
        std::exit(1);
    }
    if (idx != val.size()) {
        std::cerr << "Error: " << flag_name << " value '" << val
                  << "' is not a valid number.\n";
        std::exit(1);
    }
    if (decimal_places(val) != required_dp) {
        std::cerr << "Error: " << flag_name << " must use " << required_dp
                  << " decimal place(s) for --units " << units_str
                  << " (got '" << val << "')\n";
        std::exit(1);
    }
    if (v < 0) {
        std::cerr << "Error: " << flag_name << " must be non-negative.\n";
        std::exit(1);
    }
    return v;
}

Config parse_args(int argc, char** argv) {
    if (argc == 1) { usage(argv[0]); std::exit(0); }

    std::string folder_str, output_str, page_size_str, units_str,
                gutter_str, margin_str, icc_str, layout_str;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto take_next = [&](const char* flag_name) -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << flag_name << " requires an argument.\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if      (arg == "-f" || arg == "--folder")      folder_str    = take_next("--folder");
        else if (arg == "-o" || arg == "--output")      output_str    = take_next("--output");
        else if (arg == "-p" || arg == "--page-size")   page_size_str = take_next("--page-size");
        else if (arg == "-u" || arg == "--units")       units_str     = take_next("--units");
        else if (arg == "-g" || arg == "--gutter")      gutter_str    = take_next("--gutter");
        else if (arg == "-m" || arg == "--margin")      margin_str    = take_next("--margin");
        else if (arg == "-i" || arg == "--icc-profile") icc_str       = take_next("--icc-profile");
        else if (arg == "-l" || arg == "--layout")      layout_str    = take_next("--layout");
        else if (arg == "-h" || arg == "--help") { usage(argv[0]); std::exit(0); }
        else {
            std::cerr << "Error: unknown argument '" << arg << "'\n";
            usage(argv[0]);
            std::exit(1);
        }
    }

    bool missing = false;
    auto require = [&](const std::string& val, const char* name) {
        if (val.empty()) {
            std::cerr << "Error: " << name << " is required.\n";
            missing = true;
        }
    };
    require(folder_str,    "--folder");
    require(output_str,    "--output");
    require(page_size_str, "--page-size");
    require(units_str,     "--units");
    require(gutter_str,    "--gutter");
    require(icc_str,       "--icc-profile");
    require(layout_str,    "--layout");
    if (missing) { usage(argv[0]); std::exit(1); }

    // --- units ---
    Units units;
    if      (units_str == "in") units = Units::Inches;
    else if (units_str == "mm") units = Units::MM;
    else {
        std::cerr << "Error: --units must be 'in' or 'mm' (got '" << units_str << "')\n";
        std::exit(1);
    }
    int    required_dp  = (units == Units::Inches) ? 3 : 2;
    double pts_per_unit = (units == Units::Inches) ? PTS_PER_INCH : PTS_PER_MM;

    // --- layout ---
    Layout layout;
    if      (layout_str == "scaled") layout = Layout::Scaled;
    else if (layout_str == "bleed")  layout = Layout::Bleed;
    else {
        std::cerr << "Error: --layout must be 'scaled' or 'bleed' (got '" << layout_str << "')\n";
        std::exit(1);
    }

    // --- page size ---
    double pw_user = 0, ph_user = 0;
    std::string raw_w, raw_h;
    if (parse_dimension_string(page_size_str, pw_user, ph_user, raw_w, raw_h)) {
        if (decimal_places(raw_w) != required_dp || decimal_places(raw_h) != required_dp) {
            std::cerr << "Error: --page-size dimensions must use " << required_dp
                      << " decimal place(s) for --units " << units_str
                      << " (got '" << page_size_str << "')\n";
            std::exit(1);
        }
    } else {
        auto sz = lookup_paper_size(page_size_str);
        if (!sz) {
            std::cerr << "Error: unknown page size '" << page_size_str
                      << "'. Use WxH in your chosen units, or a standard name (Letter, A4, etc.).\n";
            std::exit(1);
        }
        pw_user = sz->w / pts_per_unit;
        ph_user = sz->h / pts_per_unit;
    }
    if (pw_user <= 0 || ph_user <= 0) {
        std::cerr << "Error: page dimensions must be positive.\n";
        std::exit(1);
    }

    // --- gutter ---
    double gutter_user = parse_measurement(gutter_str, "--gutter", required_dp, units_str);

    // --- margin (optional, default = 0.250 in = 18 pts) ---
    double margin_pts;
    if (margin_str.empty()) {
        margin_pts = DEFAULT_MARGIN_PTS;
    } else {
        double margin_user = parse_measurement(margin_str, "--margin", required_dp, units_str);
        margin_pts = margin_user * pts_per_unit;
    }

    // --- paths ---
    folder_str = expand_tilde(folder_str);
    output_str = expand_tilde(output_str);
    icc_str    = expand_tilde(icc_str);

    std::filesystem::path folder = folder_str;
    if (!std::filesystem::is_directory(folder)) {
        std::cerr << "Error: input folder not found or not a directory: " << folder << "\n";
        std::exit(1);
    }

    std::filesystem::path icc_path = icc_str;
    if (!std::filesystem::exists(icc_path)) {
        std::cerr << "Error: ICC profile not found: " << icc_path << "\n";
        std::exit(1);
    }

    return Config{
        folder,
        std::filesystem::path(output_str),
        icc_path,
        pw_user * pts_per_unit,
        ph_user * pts_per_unit,
        gutter_user * pts_per_unit,
        margin_pts,
        layout,
        units
    };
}
