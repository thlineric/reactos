/* Automatically generated file; DO NOT EDIT!! */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STANDALONE
#include "wine/test.h"

extern void func_hlink(void);
extern void func_browse_ctx(void);

const struct test winetest_testlist[] =
{
    { "browse_ctx", func_browse_ctx },
    { "hlink", func_hlink },
    { 0, 0 }
};
