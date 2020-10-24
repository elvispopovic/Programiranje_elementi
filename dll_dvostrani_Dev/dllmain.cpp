/* Replace "dll.h" with the name of your header */
#include "dll.h"
#include <windows.h>
#include <iostream>
#include <cmath>

using namespace std;

//klasa koja implementira interface
class Klasa : public IKlasa
{
public:
	Klasa()
	{
		cout << "Konstruktor: Objekt je kreiran." << endl;
	}
	~Klasa()
	{
		cout << "Destruktor: Objekt je unisten." << endl;
	}
	void f1()
	{
		cout << "Pozvana je f1 iz klase." << endl;
	}
	int f2(int x)
	{
		cout << "Pozvana je f2 iz klase sa parametrom " << x << "." << endl;
		return x*x;
	}
	
	float f3(float x)
	{
		cout << "Pozvana je f3 iz klase sa parametrom " << x << "." << endl;
		return sqrt(x);
	}
};

//pokazivac na funkciju u glavnom programu
IntFuncChar PozoviMe;

//pocetna metoda dll-a, pokrece se pri ucitavanju
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			cout << "Dll je zakvacen na proces." << endl;
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			cout << "Dll je otkvacen od procesa." << endl;
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			cout << "Dll je zakvacen na dretvu." << endl;
			break;
		}
		case DLL_THREAD_DETACH:
		{
			cout << "Dll je otkvacen od dretve." << endl;
			break;
		}
	}
	
	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}

//funkcija za kreiranje i prosljedjivanje objekta iz dll prema vanjskom programu
extern "C" __declspec(dllexport) void* __stdcall DobaviObjekt()
{
	if(PozoviMe!=NULL)
		PozoviMe("Ovo je tekst koji je poslan iz dll.");
	return static_cast<void *>(new Klasa());
}

extern "C"  __declspec(dllexport) void __stdcall PosaljiFunkciju(IntFuncChar f)
{
	PozoviMe = f;
}
