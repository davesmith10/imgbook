#pragma once
#include <optional>
#include <string_view>

struct SizePoints {
    double w;
    double h;
};

// Returns page dimensions in PDF points (1 point = 1/72 inch).
// Accepts standard names (case-insensitive): A0-A10, Letter, Legal, Ledger,
// Executive, Statement, Folio, Quarto, ANSI-A through ANSI-E.
inline std::optional<SizePoints> lookup_paper_size(std::string_view name) {
    // Normalise to upper-case for comparison
    std::string upper;
    upper.reserve(name.size());
    for (char c : name) upper += static_cast<char>(toupper(static_cast<unsigned char>(c)));

    struct Entry { const char* id; double w; double h; };
    static constexpr Entry table[] = {
        // ISO 216 A-series
        { "A0",      2384, 3371 },
        { "A1",      1684, 2384 },
        { "A2",      1191, 1684 },
        { "A3",       842, 1191 },
        { "A4",       595,  842 },
        { "A5",       420,  595 },
        { "A6",       298,  420 },
        { "A7",       210,  298 },
        { "A8",       148,  210 },
        { "A9",       105,  148 },
        { "A10",       74,  105 },
        // American standard
        { "LETTER",   612,  792 },
        { "LEGAL",    612, 1008 },
        { "LEDGER",   792, 1224 },
        { "EXECUTIVE",522,  756 },
        { "STATEMENT",396,  612 },
        { "FOLIO",    612,  936 },
        { "QUARTO",   612,  780 },
        { "ANSI-A",   612,  792 },
        { "ANSI-B",   792, 1224 },
        { "ANSI-C",  1224, 1584 },
        { "ANSI-D",  1584, 2448 },
        { "ANSI-E",  2448, 3168 },
    };

    for (const auto& e : table) {
        if (upper == e.id) return SizePoints{ e.w, e.h };
    }
    return std::nullopt;
}
