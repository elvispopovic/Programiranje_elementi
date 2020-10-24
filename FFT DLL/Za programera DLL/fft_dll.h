#ifndef __FFTDLL_H__
#define __FFTDLL_H__
#include <windows.h>

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

class InterfaceFFT
{
	public:
        virtual ~InterfaceFFT(){};
        virtual void PromijeniSmjer(char)=0;
        virtual void TransDFT(double rex[], double imx[], double ref[], double imf[])=0;
        virtual void TransDST(double x[], double f[])=0;
        virtual void TransDCT(double x[], double f[])=0;
        virtual void Normiraj(double x[], double f[])=0;
};

typedef InterfaceFFT* (__cdecl *pfInit)(unsigned char, char);

#ifdef __cplusplus
extern "C"
{
#endif
DLL_EXPORT InterfaceFFT* __stdcall InicijalizirajFFT(unsigned char bitova, char smjer);
#ifdef __cplusplus
}
#endif

#endif // __FFTDLL_H__
