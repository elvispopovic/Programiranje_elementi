//---------------------------------------------------------------------------

#ifndef KlijentH
#define KlijentH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>

#include <winsock2.h>
#include <Processthreadsapi.h>
#include <ws2tcpip.h>
#include "Dretva.h"
#include <Vcl.Menus.hpp>
#include <Vcl.Tabs.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class Dretva;
//---------------------------------------------------------------------------

class TForm2 : public TForm
{
__published:	// IDE-managed Components
	TMemo *memIspis;
	TButton *Button1;
	TButton *Button2;
	TMainMenu *MainMenu1;
	TMenuItem *Server1;
	TMenuItem *Pokreni1;
	TMenuItem *Povei1;
	TMenuItem *Zaustavi1;
	TGroupBox *GroupBox1;
	TRadioGroup *RGTest;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall RGTestClick(TObject *Sender);
private:	// User declarations
	WSAData wsaData;
	STARTUPINFO StartupInfo;

	Dretva *primanje;
	void __fastcall IspisiAdresu(ADDRINFO *info);
	void __fastcall Slanje(SOCKET server);
    void __fastcall Primanje(SOCKET server);

public:		// User declarations
	SOCKET server;
	SOCKET TForm2::PosaljiZahtijevZaVezu(const char *adresa, short port, int family);
	PROCESS_INFORMATION ProcessInfo;
	__fastcall TForm2(TComponent* Owner);
	bool __fastcall PokreniServer();
};
//---------------------------------------------------------------------------

extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
