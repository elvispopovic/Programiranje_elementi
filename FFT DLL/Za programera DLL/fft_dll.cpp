#include "fft_dll.h"
#include "fastFourier.h"

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE; // u slucaju uspjeha vraca true
}

extern "C" DLL_EXPORT InterfaceFFT* __stdcall InicijalizirajFFT(unsigned char bitova, char smjer)
{
    return static_cast<InterfaceFFT *>(new Fft(bitova, smjer));
}
