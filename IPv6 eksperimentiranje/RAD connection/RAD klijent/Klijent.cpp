//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Klijent.h"
#include "podaci.h"
//---------------------------------------------------------------------------
#pragma comment( lib, "Ws2_32.lib" )
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
	server = INVALID_SOCKET;
	if(PokreniServer())
		memIspis->Lines->Add(L"Server je pokrenut.");
	else
		memIspis->Lines->Add(L"Nije uspjelo pokretanje servera.");
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormCreate(TObject *Sender)
{
//
    memIspis->Lines->Clear();
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		return;
	memIspis->Lines->Add(L"WSA aktiviran");
	Button1Click(this->Button1);
	BringToFront();
	Button2->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormDestroy(TObject *Sender)
{
	WSACleanup();
}
//---------------------------------------------------------------------------

void __fastcall TForm2::IspisiAdresu(ADDRINFO *info)
{
	char *adresa;
	if(info==NULL)
		return;
    if(info->ai_family==AF_INET)
	{
		adresa=new char[INET_ADDRSTRLEN];
		sockaddr_in *in = (sockaddr_in *)info->ai_addr;
		inet_ntop(AF_INET,&(in->sin_addr), adresa, INET_ADDRSTRLEN);
		memIspis->Lines->Add(Format(L"IPv4 adresa: %s, port: %d", ARRAYOFCONST((adresa, ntohs(in->sin_port)))));
		delete[] adresa;
	}
    else if(info->ai_family==AF_INET6)
	{
		adresa=new char[INET6_ADDRSTRLEN];
		sockaddr_in6 *in = (sockaddr_in6 *)info->ai_addr;
		inet_ntop(AF_INET6,&(in->sin6_addr), adresa, INET6_ADDRSTRLEN);
		memIspis->Lines->Add(Format(L"IPv6 adresa: %s, port: %d", ARRAYOFCONST((adresa, ntohs(in->sin6_port)))));
		delete[] adresa;
	}
}
//---------------------------------------------------------------------------
SOCKET TForm2::PosaljiZahtijevZaVezu(const char *adresa, short port, int family)
{
	ADDRINFO hints;
    ADDRINFO *info, *ptr;
    SOCKET uticnica;
	char ime[255];
	char strPort[6];
	if(adresa==NULL)
		gethostname(ime,sizeof(ime)); //ide na isti host
    else
		strcpy(ime,adresa); //uzima adresu nekog hosta
	sprintf(strPort,"%d",port);
	ZeroMemory(&hints, sizeof(hints));
	memIspis->Lines->Add( Format(L"Adresa: %s, port: %s",ARRAYOFCONST((ime, strPort)) ));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
    if((getaddrinfo(ime,strPort,&hints,&info))==-1)
		return INVALID_SOCKET;
	for(ptr=info; ptr!=NULL; ptr=ptr->ai_next)
    {
        if((uticnica=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))== INVALID_SOCKET)
            continue;
        if (connect(uticnica, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            closesocket(uticnica);
            continue;
        }
        else
		{
			IspisiAdresu(ptr);
			freeaddrinfo(info);
			return uticnica;
        }
    }
    freeaddrinfo(info);
	return INVALID_SOCKET;
}

//---------------------------------------------------------------------------

void __fastcall TForm2::FormClose(TObject *Sender, TCloseAction &Action)
{
	DWORD exitCode;
	GetExitCodeProcess(ProcessInfo.hProcess, &exitCode);
	if(primanje!=NULL)
	{
		primanje->Stop();
		delete primanje;
	}
	if(exitCode==STILL_ACTIVE)
		TerminateProcess(ProcessInfo.hProcess,0);
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Button1Click(TObject *Sender)
{
	server = PosaljiZahtijevZaVezu(NULL,5000,AF_INET6);
	if(server == INVALID_SOCKET)
	{
		memIspis->Lines->Add(L"Nije uspjela uspostava veze.");
		return;
	}
	memIspis->Lines->Add(L"Uspostavljena je veza.");
	Button1->Enabled=false;
	Button2->Enabled=true;

	primanje = new Dretva(this, server);
	primanje->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Button2Click(TObject *Sender)
{
	primanje->Stop();
}
//---------------------------------------------------------------------------
bool __fastcall TForm2::PokreniServer()
{
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	memset(&ProcessInfo, 0, sizeof(ProcessInfo));
	if (!CreateProcess(L"server.exe", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, &StartupInfo, &ProcessInfo))
		return false;
	return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Button3Click(TObject *Sender)
{
    DWORD exitCode;
	GetExitCodeProcess(ProcessInfo.hProcess, &exitCode);
	if(exitCode!=STILL_ACTIVE)
		PokreniServer();
}
//---------------------------------------------------------------------------

void __fastcall TForm2::RGTestClick(TObject *Sender)
{

	int velicina = sizeof(GUINaredba)+26;
	char *buffer = new char[velicina];
	GUINaredba *n=(GUINaredba *)buffer;
	strcpy(n->naziv,"RB Test");
	strcpy(n->tekst,"Radio button kliknut.\0");
	n->tVelicina=strlen(n->tekst);
	n->iParametri[1]=100;
	n->iParametri[2]=200;
	n->iParametri[3]=0;
	n->dParametri[0]=3.5;
    n->dParametri[1]=2.5;
	switch(RGTest->ItemIndex)
	{
		case 0:
			memIspis->Lines->Add(L"Kliknut je prvi radio gumb.");
			n->iParametri[0]=1;
			send(server,buffer,velicina,0);
			break;
		case 1:
			memIspis->Lines->Add(L"Kliknut je drugi radio gumb.");
			n->iParametri[0]=2;
			send(server,buffer,velicina,0);
			break;
		case 2:
			memIspis->Lines->Add(L"Kliknut je tre�i radio gumb.");
			n->iParametri[0]=3;
			send(server,buffer,velicina,0);
			break;
		case 3:
			memIspis->Lines->Add(L"Kliknut je �etvrti radio gumb.");
			n->iParametri[0]=4;
			send(server,buffer,velicina,0);
			break;
	}
	delete[] buffer;
}
//---------------------------------------------------------------------------

