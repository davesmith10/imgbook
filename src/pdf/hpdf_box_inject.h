#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/* Injects a PDF page box (TrimBox, BleedBox, etc.) into a page dictionary.
 * Uses internal libharu dict API; compiled separately from hpdf.h to avoid
 * the HPDF_Dict typedef conflict between public and internal headers.
 * page_handle: opaque HPDF_Page handle (void* at the ABI level).
 */
void hpdf_inject_box(void* page_handle, const char* name,
                     float llx, float lly, float urx, float ury);

#ifdef __cplusplus
}
#endif
