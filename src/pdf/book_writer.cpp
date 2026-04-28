#include "book_writer.hpp"
#include "page_builder.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

#include <podofo/podofo.h>

using namespace PoDoFo;

static std::vector<char> read_file(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary | std::ios::ate);
    if (!f) throw std::runtime_error("Cannot open file: " + p.string());
    auto sz = f.tellg();
    f.seekg(0);
    std::vector<char> buf(static_cast<size_t>(sz));
    f.read(buf.data(), sz);
    return buf;
}

static void add_output_intent(PdfMemDocument& doc,
                               const std::filesystem::path& icc_path)
{
    auto icc_bytes = read_file(icc_path);

    PdfObject* icc_obj = doc.GetObjects().CreateObject();
    icc_obj->GetDictionary().AddKey(PdfName("N"), PdfObject(static_cast<int64_t>(4)));
    PdfMemoryInputStream icc_stream(icc_bytes.data(),
                                    static_cast<pdf_long>(icc_bytes.size()));
    icc_obj->GetStream()->Set(&icc_stream);

    PdfDictionary oi_dict;
    oi_dict.AddKey(PdfName("Type"),    PdfObject(PdfName("OutputIntent")));
    oi_dict.AddKey(PdfName("S"),       PdfObject(PdfName("GTS_PDFX")));
    oi_dict.AddKey(PdfName("OutputConditionIdentifier"),
                   PdfObject(PdfString("Custom")));
    oi_dict.AddKey(PdfName("Info"),    PdfObject(PdfString("CMYK ICC Profile")));
    oi_dict.AddKey(PdfName("RegistryName"), PdfObject(PdfString("")));
    oi_dict.AddKey(PdfName("DestOutputProfile"),
                   PdfObject(icc_obj->Reference()));

    PdfArray oi_array;
    oi_array.push_back(PdfObject(oi_dict));
    doc.GetCatalog()->GetDictionary().AddKey(
        PdfName("OutputIntents"), PdfObject(oi_array));
}

static void set_metadata(PdfMemDocument& doc) {
    doc.GetInfo()->SetCreator(PdfString("imgbook 0.1.0"));
    doc.GetInfo()->SetProducer(PdfString("imgbook 0.1.0 / PoDoFo 0.9"));
    doc.GetCatalog()->GetDictionary().AddKey(
        PdfName("GTS_PDFXVersion"), PdfObject(PdfString("PDF/X-3:2002")));
}

// Adds a PDF rectangle array [llx lly urx ury] as a page box key.
static void set_page_box(PdfPage* page, const char* key,
                         double llx, double lly, double urx, double ury)
{
    PdfArray box;
    box.push_back(PdfObject(llx));
    box.push_back(PdfObject(lly));
    box.push_back(PdfObject(urx));
    box.push_back(PdfObject(ury));
    page->GetObject()->GetDictionary().AddKey(PdfName(key), PdfObject(box));
}

void write_book(
    const std::filesystem::path& output_path,
    const std::filesystem::path& icc_profile,
    const std::vector<PageSpec>& pages)
{
    PdfMemDocument doc;
    doc.SetPdfVersion(ePdfVersion_1_3);

    set_metadata(doc);
    add_output_intent(doc, icc_profile);

    for (const auto& spec : pages) {
        const auto& m = spec.media;
        PdfPage* page = doc.CreatePage(
            PdfRect(m.left, m.bottom, m.width, m.height));
        if (!page)
            throw std::runtime_error("PoDoFo: failed to create page "
                                     + std::to_string(spec.page_number));

        if (spec.has_trim_box) {
            // TrimBox: the stated (trim) page dimensions at [0,0]
            set_page_box(page, "TrimBox", 0.0, 0.0, spec.trim_urx, spec.trim_ury);
            // BleedBox = MediaBox (already the full bleed extent)
            set_page_box(page, "BleedBox",
                         m.left, m.bottom,
                         m.left + m.width, m.bottom + m.height);
        }

        add_page_image(doc, *page, spec);
    }

    doc.Write(output_path.string().c_str());
}
