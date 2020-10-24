#include <iostream>
#include "Dinamicka.h"

using namespace std;

/* ovo moze biti u posebnom headeru ali je zbog jednostavnosti i manjeg broja datoteka stavljeno ovdje */
class KlijentKlasa : public IKlijentKlasa
{
public:
    KlijentKlasa(const char* ulazniTekst, double ulazniBroj)
    {
        cout << "U konstruktor klijent klase proslijedjen je tekst: \"" << ulazniTekst << "\", i broj " << ulazniBroj << endl;
    }
    void MetodaX(const char* tekst)
    {
        cout << "U metodu \"MetodaX\" klijent klase proslijedjen je tekst: \"" << tekst << "\"." << endl;
    }

};



int main()
{
    IDllKlasa* dllObjekt = nullptr;
    IKlijentKlasa* klijentObjekt;
    pfInit1 DohvatiObjekt;
    pfInit2 PosaljiObjekt;
    cout << "Pokretanje aplikacije..." << endl;
    HINSTANCE hInst = LoadLibrary("dinamicka.dll");
    if(!hInst)
    {
        cout << "Nije uspjelo dobavljanje instance biblioteke." << endl;
        return 1;
    }
    DohvatiObjekt = (pfInit1)GetProcAddress(hInst, "InicijalizirajIDohvatiObjekt");
    if(!DohvatiObjekt)
    {
        FreeLibrary(hInst);
        cout << "Nije uspjelo kreiranje inicijalizatora i dobavljaca objekta dll klase." << endl;
        return 1;
    }
    PosaljiObjekt = (pfInit2)GetProcAddress(hInst, "PosaljiObjekt");
    if(!PosaljiObjekt)
    {
        FreeLibrary(hInst);
        cout << "Nije uspjelo kreiranje inicijalizatora objekta za slanje klijent klase." << endl;
        return 1;
    }

    dllObjekt = DohvatiObjekt("\"Ovo je ulazni tekst.\"",101);
    if(!dllObjekt)
    {
        FreeLibrary(hInst);
        cout << "Nije uspjela inicijalizacija i dohvacanje objekta dll-a." << endl;
        return 1;
    }

    klijentObjekt = new KlijentKlasa("Ulazni tekst klijent klase.", 505.5);
    PosaljiObjekt(klijentObjekt);

    /* pozivanje metode iz dll klase, pozvat æe i obrnuto metodu klijent klase ako je objekt klijent klase uspjesno poslan*/
    dllObjekt->Metoda("Ovo je tekst za metodu.");

    /* odgovornost klijenta za unistavanje objekta kreiranog u dll */
    if(dllObjekt != nullptr)
        delete dllObjekt;
    FreeLibrary(hInst);
    delete klijentObjekt;
    return 0;
}
