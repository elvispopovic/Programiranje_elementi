#include <iostream>
#include <windows.h>
#include "..\\..\\RADGui.h"
#include "test.h"

using namespace std;

class TestFunkcije : public ITestFunkcije
{
public:
    int __stdcall Test(const char *t)
    {
        cout << "Test: " << t << endl;
        return strlen(t);
    }
    void __stdcall Click(const char *gumb)
    {
        cout << "Kliknut je " << gumb << endl;
    }
};

int main()
{
    HINSTANCE hInst;
    TestFunkcije *TestObjekt;
    hInst = LoadLibrary("..\\..\\..\\..\\Win32\\Debug\\RADGUI.dll");
    if(!hInst)
    {
        cout << "Nije uspjelo ucitavanje dll-a." << endl;
        return 1;
    }
    cout << "Dll je ucitan." << endl;
    vpFunc DobaviGUIFunkcije =  reinterpret_cast<vpFunc> (GetProcAddress(hInst,"DobaviGUIObjekt"));
    vFuncIGp ObrisiGUIObjekt =  reinterpret_cast<vFuncIGp>(GetProcAddress(hInst,"ObrisiGUIObjekt"));
    vFuncITF DobaviTestObjekt = reinterpret_cast<vFuncITF>(GetProcAddress(hInst,"DobaviTestObjekt"));
    if(!DobaviGUIFunkcije||!ObrisiGUIObjekt ||!DobaviTestObjekt)
    {
        cout << "Nije uspjelo dobavljanje funkcija za dobavljanje i brisanje GUI objekta." << endl;
        FreeLibrary(hInst);
        return 1;
    }
    cout << "Dobavljena je funkcija za dobavljanje i brisanje GUI objekta." << endl;

    TestObjekt = new TestFunkcije();
    cout << "Proslijedjujem objekt dll-u preko cijih metoda ce dll zvati testni program." << endl;
    DobaviTestObjekt(static_cast<ITestFunkcije *>(TestObjekt));

    IGUIFunkcije *GUIObjekt = static_cast<IGUIFunkcije *>(DobaviGUIFunkcije());
    if(!GUIObjekt)
    {
        cout << "Nije uspjelo dobavljanje objekta sa GUI funkcijama." << endl;
        FreeLibrary(hInst);
        return 1;
    }
    cout << "Testiranje objekta sa GUI funkcijama..." << endl;
    cout << "f1(3.0,4.0)=" << GUIObjekt->Test(3.0,4.0) << endl;

    if(TestObjekt!=NULL)
        GUIObjekt->PokreniFormu1("GUI prozor");


    delete TestObjekt;
    ObrisiGUIObjekt(GUIObjekt);
    if(FreeLibrary(hInst))
        cout << "Dll biblioteka je oslobodjena." << endl;
    return 0;
}
