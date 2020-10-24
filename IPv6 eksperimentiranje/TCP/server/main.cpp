#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>
#include <Mstcpip.h>
#include <thread>
#include <unordered_map>

// #pragma comment( lib, "Ws2_32.lib" )

using namespace std;

void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort=true);
SOCKET OtvoriPortZaSlusanje(int family, short port);
sockaddr_storage CekajVezu(SOCKET slusanje, SOCKET &klijent);

void Primanje(SOCKET klijent, unordered_map<SOCKET,sockaddr_storage> *tablica)
{
    int readed, rezultatSlanja;
    char *recvbuffer;
    int recvbufferlen = 1024;
    recvbuffer = new char[recvbufferlen];
    do
    {
        ZeroMemory(recvbuffer,sizeof(recvbuffer));
        readed = recv(klijent, recvbuffer, recvbufferlen, 0);
        auto it = tablica->find(klijent);
        if(it!=tablica->end())
            IspisiAdresu(it->second);
        if(readed>0)
            cout << " sadrzaj: " << recvbuffer << endl;
        else
            cout << " veza je prekinuta." << endl;
    }while(readed>0);
	delete[] recvbuffer;
	tablica->erase(klijent);
}

void Slanje(SOCKET klijent, unordered_map<SOCKET,sockaddr_storage> *tablica, bool *slanjeAktivno)
{
    int recvbufferlen = 1024;
    int sent;
    auto it=tablica->begin();
    char *sendbuffer = new char[recvbufferlen];
    cout << "Konzola za slanje aktivirana." << endl;
    do
    {
        sent=-1;
        ZeroMemory(sendbuffer,sizeof(sendbuffer));
        cin >> sendbuffer;
        cin.clear();
        cin.ignore();
        for(it=tablica->begin(); it!=tablica->end(); it++)
        {
            sent=send(it->first,sendbuffer,strlen(sendbuffer),0);
            if(sent==-1)
                break;
        }
    }while(strcmp(sendbuffer,"#q")!=0&&(sent!=-1)&&klijent!=INVALID_SOCKET);
    cout << "Konzola za slanje deaktivirana." << endl;
    closesocket(klijent);
    delete[] sendbuffer;
    *slanjeAktivno=false;
}


int main()
{
    WSAData wsaData;
    SOCKET slusanje;
    SOCKET klijent;
    hash<sockaddr_storage*> saHash;
    sockaddr_storage adresa;
    unordered_map <SOCKET,sockaddr_storage> tablicaKlijenata;
    socklen_t addrLen = sizeof(adresa);
    bool slanjeAktivno;
    int tip;
    thread *tSlanje;
    //potrebno dodati libwsock32 u projekt
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;
    cout << "Unesi 6 za IPv6 inace ce biti IPv4." << endl;
    cin >> tip;
    cin.clear();
    cin.ignore();
    if(tip==6)
        slusanje = OtvoriPortZaSlusanje(AF_INET6,5000);
    else
        slusanje = OtvoriPortZaSlusanje(AF_INET,5000);
    if(slusanje==INVALID_SOCKET)
    {
        cout << "Nije uspjelo kreiranje porta za slusanje." << endl;
        WSACleanup();
        return 0;
    }
    cout << "Cekam uspostavu veze na uticnici na portu 5000." << endl;
    slanjeAktivno=false;
    do
    {
    adresa=CekajVezu(slusanje, klijent);
    if(klijent == INVALID_SOCKET)
    {
        cout << "Nije uspjela uspostava veze." << endl;
        closesocket(slusanje);
        WSACleanup();
        return 0;
    }
    cout << "Uspostavljena je veza." << endl;
    cout << "Utipkani sadrzaj bit ce poslan klijentu." << endl << "Od klijenta primljeni sadrzaj biti ce ispisan." << endl;
    cout << "--------------------------" << endl;
    if(slanjeAktivno==false)
    {
        slanjeAktivno=true;
        thread t(Slanje,klijent, &tablicaKlijenata, &slanjeAktivno);
        t.detach();
    }
    tablicaKlijenata[klijent]=adresa;
    tSlanje= new thread(Primanje,klijent, &tablicaKlijenata);
    tSlanje->detach();
    }while(1);

    closesocket(slusanje);
    WSACleanup();
    system("PAUSE");
    return 0;
}

void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort)
{
    unsigned short port;
    char ispisAdrese[INET6_ADDRSTRLEN];
    if(adresa.ss_family==AF_INET)
    {
        inet_ntop(AF_INET,&(((sockaddr_in *)&adresa)->sin_addr), ispisAdrese, INET_ADDRSTRLEN);
        port=ntohs(((sockaddr_in *)&adresa)->sin_port);
    }
    else if(adresa.ss_family==AF_INET6)
    {
        inet_ntop(AF_INET6,&(((sockaddr_in6 *)&adresa)->sin6_addr), ispisAdrese, INET6_ADDRSTRLEN);
        port=ntohs(((sockaddr_in6 *)&adresa)->sin6_port);
    }
    if(ispisiPort==false)
        cout << ispisAdrese;
    else
        cout << ispisAdrese << ":" << port;
}

SOCKET OtvoriPortZaSlusanje(int family, short port)
{
    ADDRINFO hints;
    ADDRINFO *info, *ptr;
    sockaddr_storage addr = {};
    char ime[255];
    char strPort[6];
    SOCKET uticnica;
    gethostname(ime,sizeof(ime));
    cout << "Ime hosta: " << ime << endl;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = family;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    sprintf(strPort,"%d",port);
    if(getaddrinfo(ime,strPort,&hints,&info)!=0)
        return INVALID_SOCKET;
    else
        for(ptr=info; ptr!=nullptr; ptr=ptr->ai_next)
        {
            if((uticnica=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol))== INVALID_SOCKET)
                continue;
            memcpy(&addr,ptr->ai_addr,ptr->ai_addrlen);
            IspisiAdresu(addr);
            cout << endl;
            if(bind(uticnica, ptr->ai_addr,ptr->ai_addrlen) == SOCKET_ERROR)
            {
                closesocket(uticnica);
                freeaddrinfo(info);
                return INVALID_SOCKET;
            }
            else
            {
                freeaddrinfo(info);
                return uticnica;
            }
        }
}

sockaddr_storage CekajVezu(SOCKET slusanje, SOCKET &klijent)
{
    sockaddr_storage adresa;
    socklen_t addrLen=sizeof(adresa);
    if(listen(slusanje, SOMAXCONN)==SOCKET_ERROR)
        klijent=INVALID_SOCKET;
    else klijent = accept(slusanje, (sockaddr*)&adresa, &addrLen);
    return adresa;
}

