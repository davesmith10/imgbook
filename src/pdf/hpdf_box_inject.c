/*
 * Do NOT include hpdf.h here. This file uses internal libharu types only
 * (HPDF_Dict, HPDF_Box, HPDF_Box_Array_New, HPDF_Dict_Add) to avoid the
 * HPDF_Dict typedef conflict that would arise if hpdf.h and hpdf_objects.h
 * were included in the same translation unit.
 */
#include <hpdf_pages.h>   /* pulls in hpdf_objects.h transitively */
#include "hpdf_box_inject.h"

void hpdf_inject_box(void* page_handle, const char* name,
                     float llx, float lly, float urx, float ury)
{
    HPDF_Page page = (HPDF_Page)page_handle;  /* HPDF_Page = HPDF_Dict = HPDF_Dict_Rec* */
    HPDF_Box box;
    HPDF_Array arr;

    box.left   = llx;
    box.bottom = lly;
    box.right  = urx;
    box.top    = ury;

    arr = HPDF_Box_Array_New(page->mmgr, box);
    if (arr)
        HPDF_Dict_Add((HPDF_Dict)page, name, arr);
}
