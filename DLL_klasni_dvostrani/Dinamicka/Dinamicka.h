#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

/* dva sucelja za Dll i Klijent klasu */
class IKlijentKlasa
{
public:
    virtual ~IKlijentKlasa(){};
    virtual void MetodaX(const char* tekst)=0;

};

class IDllKlasa
{
public:
    virtual ~IDllKlasa(){};
    virtual void Metoda(const char* tekst)=0;
};

/*  inicijalizator funkcije za dobavljanje objekta dll klase */
typedef IDllKlasa* (__cdecl *pfInit1)(const char*, int);

/*  inicijalizator funkcije za slanje objekta klijent klase */
typedef void (__cdecl *pfInit2)(IKlijentKlasa*);

#ifdef __cplusplus
extern "C"
{
#endif
DLL_EXPORT IDllKlasa* __stdcall InicijalizirajIDohvatiObjekt(const char* ulazniTekst, int ulazniBroj);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
DLL_EXPORT void __stdcall PosaljiObjekt(IKlijentKlasa* ko);
#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
