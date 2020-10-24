#ifndef _DLL_H_
#define _DLL_H_

#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif

//interface klase u dll-u. Ne moramo znati kako klasa izgleda i sto joj rade metode
//ali ove metode sigurno ima i mozemo ih koristiti
class IKlasa
{
	public:
		virtual void  __stdcall f1()=0;
		virtual int   __stdcall f2(int)=0;
		virtual float __stdcall f3(float)=0;
};

//tip pokazivaca na funkciju u glavnom programu preko kojega dll moze pozivati funkciju u programu
typedef int(*IntFuncChar)(const char *);
//tip pokazivaca na funkciju koja dobavlja objekt
typedef void* (*VPFunc1)();
typedef void  (*VFunc1)(IKlasa *);
typedef void* (*VPFunc2)(IntFuncChar);

#endif
