//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be performing new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you

#include <fmx.h>
#include <iostream>
#pragma hdrstop
#pragma argsused
#include "RADGui.h"
#include "CodeBlocks_test\\Test\\test.h"
#include "Unit1.h"

ITestFunkcije *TestObjekt;

class GUIFunkcije : public IGUIFunkcije
{
private:
	TForm1 *form;
public:
	GUIFunkcije()
	{
		form = new TForm1(NULL,TestObjekt);
	}
	~GUIFunkcije()
	{
	}
	float __stdcall Test(float x, float y)
	{
		return(sqrt(x*x+y*y));
	}
	void  __stdcall PokreniFormu1(const char *caption)
	{
		if(form==NULL)
			return;
		form->Caption = caption;
		form->ShowModal();
	}
};

extern "C" int _libmain(unsigned long reason)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	return 1;
}

extern "C" __declspec(dllexport) void* __stdcall DobaviGUIObjekt()
{
    return static_cast<void *>(new GUIFunkcije());
}

extern "C" __declspec(dllexport) void __stdcall ObrisiGUIObjekt(IGUIFunkcije *objekt)
{
	delete static_cast<GUIFunkcije *>(objekt);
}

extern "C" __declspec(dllexport) void __stdcall DobaviTestObjekt(ITestFunkcije *objekt)
{
	int r;
	char ispis[80];
	TestObjekt = objekt;
	r=TestObjekt->Test("Pozivam testnu metodu iz dll-a i biljezim vraceni rezultat.");
	sprintf(ispis,"Prethodni poziv je vratio rezultat %d.",r);
	TestObjekt->Test(ispis);
}

