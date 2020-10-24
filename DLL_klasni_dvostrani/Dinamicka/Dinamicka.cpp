#include <iostream>
#include "Dinamicka.h"


IKlijentKlasa* klijentObjekt;
IDllKlasa* dllObjekt;

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            dllObjekt = nullptr;
            std::cout << "Prikljucen proces dll-a." << std::endl;
            break;

        case DLL_PROCESS_DETACH:
            if(dllObjekt != nullptr)
                delete dllObjekt;
            std::cout << "Odvojen proces dll-a." << std::endl;
            break;

        case DLL_THREAD_ATTACH:
            dllObjekt = nullptr;
            std::cout << "Prikljucena dretva dll-a." << std::endl;
            break;

        case DLL_THREAD_DETACH:
            if(dllObjekt != nullptr)
                delete dllObjekt;
            std::cout << "Odvojena dretva dll-a." << std::endl;
            break;
    }
    return TRUE; // succesful
}

class DllKlasa : public IDllKlasa
{
public:
    DllKlasa(const char* ulazniTekst, int ulazniBroj)
    {
        std::cout << "Konstruktoru je proslijedjen ulazni tekst: " << ulazniTekst;
        std::cout << " i ulazni broj " << ulazniBroj << std::endl;
    }
    void Metoda(const char* tekst)
    {
        std::cout << "Metodi je proslijedjen tekst: " << tekst << std::endl;
        if(klijentObjekt)
            klijentObjekt->MetodaX("Tekst prema klijent klasi.");
    }
private:
};

extern "C" DLL_EXPORT IDllKlasa* __stdcall InicijalizirajIDohvatiObjekt(const char* ulazniTekst, int ulazniBroj)
{
    if(dllObjekt == nullptr)
        dllObjekt = new DllKlasa(ulazniTekst, ulazniBroj);
    return static_cast<IDllKlasa *>(dllObjekt);
}

extern "C" DLL_EXPORT void __stdcall PosaljiObjekt(IKlijentKlasa* ko)
{
	klijentObjekt = ko;
}
