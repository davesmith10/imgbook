#include "args.hpp"
#include "natural_sort.hpp"
#include "image/preflight.hpp"
#include "image/jpeg_loader.hpp"
#include "image/cmyk_convert.hpp"
#include "layout/layout.hpp"
#include "layout/scaled_layout.hpp"
#include "layout/bleed_layout.hpp"
#include "pdf/book_writer.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    Config cfg = parse_args(argc, argv);

    // --- Collect and sort images ---
    std::vector<fs::path> image_paths;
    for (const auto& entry : fs::directory_iterator(cfg.folder)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        for (auto& c : ext) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        if (ext == ".jpg" || ext == ".jpeg")
            image_paths.push_back(entry.path());
    }
    std::sort(image_paths.begin(), image_paths.end(), natural_less);

    if (image_paths.empty()) {
        std::cerr << "Error: no JPEG images found in " << cfg.folder << "\n";
        return 1;
    }

    // --- Pre-flight ---
    std::vector<ImageInfo> infos;
    infos.reserve(image_paths.size());
    bool ok = true;
    for (const auto& p : image_paths) {
        ImageInfo info;
        if (!preflight_jpeg(p, info)) ok = false;
        else infos.push_back(info);
    }
    if (!ok) return 1;

    // --- Even page count ---
    if (infos.size() % 2 != 0) {
        std::cerr << "Error: " << infos.size()
                  << " image(s) found — a printed book requires an even page count.\n"
                  << "  Add or remove one image so the total is even.\n";
        return 1;
    }

    std::cout << "imgbook: " << infos.size() << " pages  |  "
              << cfg.page_width_pts << " x " << cfg.page_height_pts << " pts  |  "
              << "gutter " << cfg.gutter_pts << " pts  |  "
              << (cfg.layout == Layout::Scaled ? "scaled" : "bleed") << " layout";
    if (cfg.layout == Layout::Scaled)
        std::cout << "  |  margin " << cfg.margin_pts << " pts";
    std::cout << "\n";

    // --- Process each image ---
    std::vector<PageSpec> page_specs;
    page_specs.reserve(infos.size());

    for (int i = 0; i < static_cast<int>(infos.size()); ++i) {
        const auto& info = infos[i];
        int page_num = i + 1;
        PageSide side = side_for_page(page_num);

        std::cout << "  Page " << page_num
                  << " (" << info.path.filename().string() << ") ...";
        std::cout.flush();

        RgbImage rgb = load_jpeg_rgb(info.path);
        std::vector<uint8_t> cmyk_jpeg = rgb_to_cmyk_jpeg(rgb, cfg.icc_profile, 90);

        PageSpec spec;
        spec.cmyk_jpeg   = std::move(cmyk_jpeg);
        spec.img_pixel_w = info.width;
        spec.img_pixel_h = info.height;
        spec.page_number = page_num;

        if (cfg.layout == Layout::Scaled) {
            spec.placement = scaled_placement(
                info.width, info.height,
                cfg.page_width_pts, cfg.page_height_pts,
                cfg.gutter_pts, cfg.margin_pts, side);
            spec.media = MediaBox{ 0.0, 0.0,
                                   cfg.page_width_pts, cfg.page_height_pts };
            spec.has_trim_box = false;
        } else {
            BleedPlacement bp = bleed_placement(
                info.width, info.height,
                cfg.page_width_pts, cfg.page_height_pts,
                cfg.gutter_pts, side);
            spec.placement    = bp.image;
            spec.media        = bp.media;
            spec.has_trim_box = true;
            spec.trim_urx     = bp.trim_urx;
            spec.trim_ury     = bp.trim_ury;
        }

        page_specs.push_back(std::move(spec));
        std::cout << " done\n";
    }

    // --- Write PDF ---
    std::cout << "Writing " << cfg.output << " ...\n";
    try {
        write_book(cfg.output, cfg.icc_profile, page_specs);
    } catch (const std::exception& e) {
        std::cerr << "Error writing PDF: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Done. " << page_specs.size() << " pages written to "
              << cfg.output << "\n";
    return 0;
}
