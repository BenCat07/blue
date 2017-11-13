#include "stdafx.h"

#if blueplatform_windows()
BOOL APIENTRY dllmain(HMODULE this_module, int reason, void *reserved) {
    return true;
}
#else
void __attribute__((constructor)) startup() {}

void __attribute__((destructor)) shutdown() {}
#endif