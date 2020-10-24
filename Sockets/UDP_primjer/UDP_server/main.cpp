#define _WIN32_WINNT 0x0600
#include <iostream>
#include <thread>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctime>

/* 
	dodati statièku biblioteku libws2_32.a u projekt 
	ova se biblioteka nalazi u lib direktoriju C++ kompilera
*/

using namespace std;

void obradiUticnicu(int& uticnica, int& threadClose, char* hostname);

int main(int argc, char** argv) 
{
	WSADATA wsa;
	int i, j, k, n, port;
	char ch;
	int winsock, rezultat;
	char hostname[256], adresaIspis[64];
	void *pAdresa1, *pAdresa2;
	addrinfo upute, *info, *pInfo;
	sockaddr_storage odabranaAdresa;
	int uticnica;
	thread* dretvaUticnice;
	int threadClose;
	
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
	/* Dohvacanje imena hosta */
	gethostname(hostname, 256);
	cout << "Domacin (host): " << hostname << endl;
	
	/* dohvacanje adresa adaptera */
	memset(&upute, 0, sizeof(upute));
	upute.ai_family = AF_UNSPEC;
	upute.ai_socktype = SOCK_DGRAM;
	if((rezultat = getaddrinfo(hostname, nullptr, &upute, &info) != 0))
	{
		cout << "Nije uspjelo dohvacanje adresa adaptera: " << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	cout << "Adrese adaptera:" << endl;
	for(i=1, pInfo = info; pInfo != nullptr; pInfo = pInfo->ai_next)
	{
		pAdresa1 = pInfo->ai_addr;
		if(pInfo->ai_family == PF_INET)
		{
			pAdresa2 = &(((sockaddr_in*)pAdresa1)->sin_addr);
			inet_ntop(PF_INET, pAdresa2, (PSTR)adresaIspis, sizeof(adresaIspis));
			cout << i << ": " << adresaIspis << endl;
			i++;
		}
		else if(pInfo->ai_family == PF_INET6)
		{
			pAdresa2 = &(((sockaddr_in6*)pAdresa1)->sin6_addr);
			inet_ntop(PF_INET6, pAdresa2, (PSTR)adresaIspis, sizeof(adresaIspis));
			cout << i << ": " << adresaIspis << endl;
			i++;
		}
	}
	
	/* odabir adaptera */
	do
	{
		cout << "Unesi broj (od 1 do " << i-1 << ")!" << endl;
		cin >> j;
		if(cin.fail())
        {
        	cin.clear();
        	cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
	}while (j<1 || j>=i);
	/*  unos porta */
	do
	{
		cout << "Unesi port (od 10000 do 40000)!" << endl;
		cin >> k;
		if(cin.fail())
        {
        	cin.clear();
        	cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
	}while (k<10000 || k>40000);
	
	
	/* pretrazivanje adresa i upis u sockaddr_storage */
	for(i=1, pInfo = info; pInfo != nullptr; pInfo = pInfo->ai_next)
	{
		if(i == j)
		{
			memset(&odabranaAdresa, 0, sizeof(odabranaAdresa));
			memcpy(&odabranaAdresa, (sockaddr_storage*)pInfo->ai_addr, sizeof(sockaddr_storage));
			if(odabranaAdresa.ss_family == PF_INET)
				((sockaddr_in*)&odabranaAdresa)->sin_port=htons(k);
			else if(odabranaAdresa.ss_family == PF_INET6)
				((sockaddr_in6*)&odabranaAdresa)->sin6_port=htons(k);
			break;
		}
		if(pInfo->ai_family == PF_INET || pInfo->ai_family == PF_INET6)
			i++;
	}
	freeaddrinfo(info);	
	/* ispis iz sockaddr_storage strukture kao test */
	if(odabranaAdresa.ss_family == PF_INET)
	{
		inet_ntop(odabranaAdresa.ss_family, &(((sockaddr_in*)&odabranaAdresa)->sin_addr), (PSTR)adresaIspis, sizeof(adresaIspis));
		port = ntohs(((sockaddr_in*)&odabranaAdresa)->sin_port);
		cout << "Odabrana je IPv4 adresa " << adresaIspis << ":" << port << endl;
	}
	else
	{
		inet_ntop(odabranaAdresa.ss_family, &(((sockaddr_in6*)&odabranaAdresa)->sin6_addr), (PSTR)adresaIspis, sizeof(adresaIspis));
		port = ntohs(((sockaddr_in6*)&odabranaAdresa)->sin6_port);
		cout << "Odabrana je IPv6 adresa [" << adresaIspis << "]:" << port << endl;
	}

	
	/* kreiramo i vezemo (bind) uticnicu */
	uticnica = socket(odabranaAdresa.ss_family, SOCK_DGRAM, 0);
	if(bind(uticnica, (sockaddr*)&odabranaAdresa, sizeof(sockaddr_storage)) != 0)
	{
		cout << "Nije uspjelo vezanje uticnice." << endl;
		WSACleanup();
		return 0;
	}
	cout << "Uticnica " << uticnica << " je stvorena i vezana." << endl;

	
	/* pokretanje dretve koja obradjuje poruke na uticnici */
	threadClose = 0;
	dretvaUticnice = new thread(&obradiUticnicu, ref(uticnica), ref(threadClose), hostname);
	
	/* cekamo ESC sa konzole, a onda saljemo praznu poruku sami sebi sto prekida petlju u radnoj dretvi */
	while(ch != 27)
        ch = _getch();
        
    /* zatvaranje dretve */
    threadClose = 1;
	closesocket(uticnica); //salje praznu poruku i time oslobadja uticnicu iz recvfrom cekanja

	dretvaUticnice->join();
    delete dretvaUticnice;
	cout << "Uticnica " << uticnica << " je zatvorena." << endl;
	WSACleanup();
	cout << "Winsock je zaustavljen." << endl;
	return 0;
}

/* 	radna dretva - uticnica se obradjuje u posebnoj dretvi unutar beskonacne petlje 
	tako da je glavna dretva rezervirana za provjeru tipkovnice tj pritiska tipke ESC  
*/
void obradiUticnicu(int& uticnica, int& threadClose, char* hostname)
{
	int n, velicinaAdrese;
	time_t vrijeme;
	sockaddr_storage adresaKlijenta;
	char bufferRecv[1024], bufferSend[1024];
	cout << "--------" << endl << "Dretva slusanja je pokrenuta. Cekam poruke klijenata. " << endl;
	cout << "Pritisni ESC za zatvaranje uticnice i kraj rada." << endl;
	while(1)
	{
		memset(bufferRecv, 0, sizeof(bufferRecv));
		n = recvfrom(uticnica, bufferRecv, 1024, 0, (sockaddr *)&adresaKlijenta, &velicinaAdrese);
		if(n>0)
		{
			vrijeme = time(0);
			cout << "Primljena je poruka u " << ctime(&vrijeme);
			cout << "Sadrzaj poruke: \"" << bufferRecv << "\"" << endl;			
			cout << "Saljem poruku potvrde." << endl;
			sprintf(bufferSend, "Posluzitelj %s je primio poruku u %s", hostname, ctime(&vrijeme));
			n = sendto(uticnica, bufferSend, strlen(bufferSend), 0, (sockaddr *)&adresaKlijenta, velicinaAdrese);
		}
		else if(threadClose == 0)
			cout << "Primljena je prazna poruka." << endl;
		else
			break;
		cout << "Za izlaz iz programa pritisnite tipku ESC." << endl << endl;
	}
	cout << "Zaustavljena je dretva slusanja uticnice." << endl;	
}



