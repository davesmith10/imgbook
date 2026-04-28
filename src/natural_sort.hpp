#pragma once
#include <string>
#include <filesystem>

// Splits a string into alternating alpha and numeric chunks, then compares
// them so that numeric segments are ordered by value rather than lexically.
// e.g. "img2.jpg" < "img10.jpg" < "img20.jpg"
inline bool natural_less(const std::filesystem::path& a, const std::filesystem::path& b) {
    const std::string sa = a.filename().string();
    const std::string sb = b.filename().string();

    size_t i = 0, j = 0;
    while (i < sa.size() && j < sb.size()) {
        if (isdigit(static_cast<unsigned char>(sa[i])) &&
            isdigit(static_cast<unsigned char>(sb[j]))) {
            // Compare numeric run
            size_t ni = i, nj = j;
            while (ni < sa.size() && isdigit(static_cast<unsigned char>(sa[ni]))) ++ni;
            while (nj < sb.size() && isdigit(static_cast<unsigned char>(sb[nj]))) ++nj;
            // Compare by numeric value (strip leading zeros by length then lex)
            size_t lenA = ni - i, lenB = nj - j;
            if (lenA != lenB) return lenA < lenB;
            int cmp = sa.compare(i, lenA, sb, j, lenB);
            if (cmp != 0) return cmp < 0;
            i = ni; j = nj;
        } else {
            unsigned char ca = static_cast<unsigned char>(tolower(static_cast<unsigned char>(sa[i])));
            unsigned char cb = static_cast<unsigned char>(tolower(static_cast<unsigned char>(sb[j])));
            if (ca != cb) return ca < cb;
            ++i; ++j;
        }
    }
    return sa.size() < sb.size();
}
