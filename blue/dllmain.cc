#include "stdafx.hh"

#if blueplatform_windows()

extern auto __stdcall blue_gamesystem_send_process_attach(void *a) -> u32;

__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule,
                                            DWORD   reason,
                                            LPVOID  lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)&blue_gamesystem_send_process_attach, hModule, 0, nullptr);
        break;
    }

    return true;
}
#else
void __attribute__((constructor)) startup() {}

void __attribute__((destructor)) shutdown() {}
#endif
