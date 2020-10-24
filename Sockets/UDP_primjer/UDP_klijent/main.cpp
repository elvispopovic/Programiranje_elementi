#define _WIN32_WINNT 0x0600
#include <iostream>
#include <cstring>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <ctime>

/* 
	dodati statièku biblioteku libws2_32.a u projekt 
	ova se biblioteka nalazi u lib direktoriju C++ kompilera
	i služi kao podrška Winsock-u
*/

using namespace std;

void obradiUticnicu(int& uticnica, sockaddr_storage adresa, int& threadClose);

int main(int argc, char** argv) 
{
	WSADATA wsa;
	char adresaUnos[INET6_ADDRSTRLEN]; //dovoljno mjesta za IPv6 adrese
	char adresaPrikaz[INET6_ADDRSTRLEN];
	char adresaIPort[INET6_ADDRSTRLEN+10]; //dovoljno za IPv6 adresu i port
	int i, portUnos, port;
	sockaddr_storage adresa;
	int winsock, rezultat;
	addrinfo upute, *info;
	void *pAdresa;
	int uticnica;
	thread* dretvaUticnice;
	int threadClose;
	string tekstUnos;
	
	/* aktiviranje WinSock-a */
	if((winsock = WSAStartup(MAKEWORD(2,2), &wsa))==-1) //neuspjeh
	{
		cout << "Winsock pogreska: " << WSAGetLastError() << endl;
		cout << "Program ce se zatvoriti." << endl;
		return 0;
	}
	else //uspjeh
	{
		cout << "Otvoren je Winsock" << 
		", v" << int(LOBYTE(wsa.wHighVersion)) << "." <<int(HIBYTE(wsa.wHighVersion)) << 
		", status: " << wsa.szSystemStatus << endl;
	}
	
	/* unos adrese */
	do
	{
		cout << "Unesi IP adresu posluzitelja (IPv4 ili IPv6)!" << endl;
		cin >> adresaUnos;
		memset(&adresa, 0, sizeof(adresa));
		rezultat = inet_pton(AF_INET, adresaUnos, &((sockaddr_in*)&adresa)->sin_addr);
		if(rezultat != 0) //unesena je IPv4 adresa
			adresa.ss_family = AF_INET;
		else
		{
			rezultat = inet_pton(AF_INET6, adresaUnos, &((sockaddr_in6*)&adresa)->sin6_addr);
			if(rezultat != 0)
				adresa.ss_family = AF_INET6;
		}
	}while(rezultat == 0);
	/* unos porta */
	do
	{
		cout << "Unesi port!" << endl;
		cin >> portUnos;
		if(cin.fail())
        {
        	cin.clear();
        	cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
	}while(portUnos < 10000 || portUnos > 40000);
	if(adresa.ss_family == AF_INET)
		((sockaddr_in*)&adresa)->sin_port=htons(portUnos);
	else
		((sockaddr_in6*)&adresa)->sin6_port=htons(portUnos);
		
	/* prikaz adrese i porta za test */
	if (adresa.ss_family == AF_INET)
    {
        port = ntohs(((sockaddr_in *)&adresa)->sin_port);
        pAdresa = &(((sockaddr_in *)&adresa)->sin_addr);
    }
    else if(adresa.ss_family == AF_INET6)
    {
        port = ntohs(((sockaddr_in6 *)&adresa)->sin6_port);
        pAdresa = &(((sockaddr_in6 *)&adresa)->sin6_addr);
    }
    if(pAdresa == nullptr)
        strcpy(adresaPrikaz,"");
	else
	{
    	inet_ntop(adresa.ss_family, pAdresa, (PSTR)adresaPrikaz, INET6_ADDRSTRLEN);
    	if(adresa.ss_family == AF_INET)
    	{
    		cout << "Odabrana je IPv4 adresa" << endl;
    		cout << adresaPrikaz << ":" << port << endl;
        }
        else
        {
        	cout << "Odabrana je IPv6 adresa" << endl;
        	cout << "[" << adresaPrikaz << "]:" << port << endl;
        }
	}
	
	/* kreiranje uticnice */
	if((uticnica = socket(adresa.ss_family, SOCK_DGRAM, 0)) < 0)
	{
		cout << "Nije uspjelo kreiranje uticnice. Greska " << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	/* slanje pozdravne poruke poruke */
	sendto(uticnica, "Pozdrav od klijenta.", 21, 0, (sockaddr*)&adresa, sizeof(adresa));
	
	/* pokretanje dretve primanja */
	threadClose = 0;
	dretvaUticnice = new thread(&obradiUticnicu, ref(uticnica), adresa, ref(threadClose));
	
	/* petlja slanja */
	do
	{
		getline(cin, tekstUnos);
		rezultat = sendto(uticnica, tekstUnos.c_str(), tekstUnos.size(), 0, (sockaddr*)&adresa, sizeof(adresa));
		if(rezultat == -1)
			cout << "Greska prilikom slanja: " << WSAGetLastError() << endl;
	}while(tekstUnos.compare("q") != 0);
	
	/* gasenje dretve i zatvaranje uticnice */
	threadClose = 1;
	closesocket(uticnica); //salje praznu poruku i oslobadja uticnicu iz recvfrom cekanja
	
	/* zatvaranje dretve */
	dretvaUticnice->join();
    delete dretvaUticnice;
	cout << "Uticnica " << uticnica << " je zatvorena." << endl;
	WSACleanup();
	cout << "Winsock je zaustavljen." << endl;
	return 0;
}

void obradiUticnicu(int& uticnica, sockaddr_storage adresa, int& threadClose)
{
	int n, velicinaAdrese;
	time_t vrijeme;
	char bufferRecv[1024];
	cout << "Pokrenuta je dretva za slusanje uticnice." << endl;
	velicinaAdrese = sizeof(adresa);
	while(1)
	{
		memset(bufferRecv,0,sizeof(bufferRecv));
		n = recvfrom(uticnica, bufferRecv, 1024, 0, (sockaddr*)&adresa, &velicinaAdrese);
		if(n > 0)
		{
			vrijeme = time(0);
			cout << "Primljena je poruka sa posluzitelja u " << ctime(&vrijeme);
			cout << "Sadrzaj: \"" << bufferRecv << "\"" << endl;
		}
		else if(threadClose == 0)
		{
			cout << "Primljena je prazna poruka. Greska " << WSAGetLastError() << endl;
		}
		else
			break;
		cout << "Za zaustavljanje klijenta unesite q i pritisnite ENTER." << endl << endl;
	}
	cout << "Zatvorena je dretva slusanja." << endl;
}
