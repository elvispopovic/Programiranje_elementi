#ifndef __RADGUI__
#define __RADGUI__

#include <cmath>

#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif

class IGUIFunkcije
{
public:
	virtual float __stdcall Test(float, float)=0;
    virtual void  __stdcall PokreniFormu1(const char*)=0;
};

typedef void*(*vpFunc)();
typedef void (*vFuncIGp)(IGUIFunkcije *);

#endif __RADGUI__
