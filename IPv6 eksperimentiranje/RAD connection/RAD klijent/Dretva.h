//---------------------------------------------------------------------------

#ifndef DretvaH
#define DretvaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.StdCtrls.hpp>
#include "Klijent.h"
//---------------------------------------------------------------------------
class TForm2;
//---------------------------------------------------------------------------
class Dretva : public TThread
{
private:
	volatile long dStop;
	SOCKET server;
	TForm2 *form;
	int recvbufferlen;
	wchar_t *ispis;
	char *recvbuffer;
	wchar_t *primljeno;
	void __fastcall Ispisi();
	void __fastcall OsvjeziProces();
public:
	__fastcall Dretva(TForm2* f, SOCKET s);
	__fastcall ~Dretva();
	void __fastcall Dretva::Execute();
	void __fastcall Dretva::Stop();


};

//---------------------------------------------------------------------------
#endif
