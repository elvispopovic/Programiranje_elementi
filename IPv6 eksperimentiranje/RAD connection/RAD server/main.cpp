#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>
#include <Mstcpip.h>
#include <thread>
#include "podaci.h"
// #pragma comment( lib, "Ws2_32.lib" )

using namespace std;

void IspisiAdresu(ADDRINFO *info);
SOCKET OtvoriPortZaSlusanje(int family, short port);
SOCKET CekajVezu(SOCKET slusanje);

void Primanje(SOCKET klijent)
{
    int readed, rezultatSlanja;
    char *recvbuffer;
    int recvbufferlen = 1024;
    recvbuffer = new char[recvbufferlen];
    do
    {
        ZeroMemory(recvbuffer,sizeof(recvbuffer));
        readed = recv(klijent, recvbuffer, recvbufferlen, 0);
        if(readed>0)
        {
            cout << "naziv: " << ((GUINaredba *)recvbuffer)->naziv << endl;
            cout << "int parametri: " << ((GUINaredba *)recvbuffer)->iParametri[0] << ", ";
            cout << ((GUINaredba *)recvbuffer)->iParametri[1] << ", ";
            cout << ((GUINaredba *)recvbuffer)->iParametri[2] << ", ";
            cout << ((GUINaredba *)recvbuffer)->iParametri[3] << endl;
            cout <<  "double parametri: " <<((GUINaredba *)recvbuffer)->dParametri[0] << ", ";
            cout << ((GUINaredba *)recvbuffer)->iParametri[1] << endl;
            cout << "tekst: " << ((GUINaredba *)recvbuffer)->tekst << endl;
        }
        else
            cout << "Veza je prekinuta." << endl;
    }while(readed>0);
	delete[] recvbuffer;
}

void Slanje(SOCKET klijent)
{
    int recvbufferlen = 1024;
    int sent;
    char *sendbuffer = new char[recvbufferlen];
    do
    {
        ZeroMemory(sendbuffer,sizeof(sendbuffer));
        cin >> sendbuffer;
        sent=send(klijent,sendbuffer,strlen(sendbuffer),0);
    }while(strcmp(sendbuffer,"#q")!=0&&(sent!=-1)&&klijent!=INVALID_SOCKET);
    closesocket(klijent);
    delete[] sendbuffer;
}


int main()
{
    WSAData wsaData;
    SOCKET slusanje;
    SOCKET klijent;
    //potrebno dodati libwsock32 u projekt
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;
    slusanje = OtvoriPortZaSlusanje(AF_INET6,5000);
    if(slusanje==INVALID_SOCKET)
    {
        cout << "Nije uspjelo kreiranje porta za slusanje." << endl;
        WSACleanup();
        return 0;
    }
    while(1)
    {
    cout << "Cekam uspostavu veze na uticnici na portu 5000." << endl;
    klijent=CekajVezu(slusanje);
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
    //thread t1(Slanje,klijent);
    thread t2(Primanje,klijent);

    //t1.join();
    t2.join();
    }
    closesocket(slusanje);
    WSACleanup();
    system("PAUSE");
    return 0;
}

void IspisiAdresu(ADDRINFO *info)
{
    char *adresa;
    if(info==nullptr)
        return;
    if(info->ai_family==AF_INET)
    {
        adresa=new char[INET_ADDRSTRLEN];
        sockaddr_in *in = (sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET,&(in->sin_addr), adresa, INET_ADDRSTRLEN);
        cout << "IPv4 adresa: " << adresa << ", port: " << ntohs(in->sin_port) << endl;
        delete[] adresa;
    }
    else if(info->ai_family==AF_INET6)
    {
        adresa=new char[INET6_ADDRSTRLEN];
        sockaddr_in6 *in = (sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6,&(in->sin6_addr), adresa, INET6_ADDRSTRLEN);
        cout << "IPv6 adresa: " << adresa << ", port: " << ntohs(in->sin6_port) << endl;
        delete[] adresa;
    }
}

SOCKET OtvoriPortZaSlusanje(int family, short port)
{
    ADDRINFO hints;
    ADDRINFO *info, *ptr;
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
            IspisiAdresu(ptr);
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

SOCKET CekajVezu(SOCKET slusanje)
{
    SOCKET klijent;
    if(listen(slusanje, SOMAXCONN)==SOCKET_ERROR)
        return INVALID_SOCKET;
    if((klijent = accept(slusanje, NULL, NULL))==INVALID_SOCKET)
        return INVALID_SOCKET;
    return klijent;
}

