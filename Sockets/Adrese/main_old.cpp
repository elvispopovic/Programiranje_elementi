#define _WIN32_WINNT 0x0600

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <vector>

using namespace std;

int pokreniWinSock()
{
    WSADATA wsa;
    int iRes;
    iRes = WSAStartup(MAKEWORD(2,2),&wsa);
    if (iRes != 0)
    {
        switch(iRes)
        {
            case WSASYSNOTREADY:
                cout << "Mrezni sustav nije spreman za komunikaciju." << endl;
                break;
            case WSAVERNOTSUPPORTED:
                cout << "Verzija WinSock nije podrzana." << endl;
                break;
            case WSAEINPROGRESS:
                cout << "Pokrenuta je blokirajuca WinSock operacija." << endl;
                break;
            case WSAEPROCLIM:
                cout << "Dosegnut je maksimum zadataka za Winsock." << endl;
                break;
            case WSAEFAULT:
                cout << "Pridruzeni pokazivac nije ispravnog tipa." << endl;
                break;
        }
        cout << "Greska: " << WSAGetLastError() << endl;
        return iRes;
    }

    cout << "WSA opis: " << wsa.szDescription << endl;
    cout << "Najveca moguca verzija: " << int(LOBYTE(wsa.wHighVersion)) << "." << int(HIBYTE(wsa.wHighVersion)) << endl;
    cout << "Status: " << wsa.szSystemStatus << endl;
    return iRes;
}

bool dohvatiAdaptere()
{
    char hostname[256];
    int i, iRes;
    addrinfo upute={};
    addrinfo *info, *ptrInfo;
    void *adresa;
    char *ipverzija;
    sockaddr_in *ipv4;
    sockaddr_in6 *ipv6;
    char ipstr[INET6_ADDRSTRLEN];
    upute.ai_family = AF_UNSPEC;
    upute.ai_socktype = SOCK_STREAM;
    upute.ai_protocol = IPPROTO_TCP;
    gethostname(hostname,256);
    cout << "Ime domacina: " << hostname << endl;
    iRes = getaddrinfo(hostname, nullptr, &upute, &info);
    if(iRes!=0)
    {
        cout << "Nije uspjelo dobavljanje adresnih informacija (WSA: %d), " << WSAGetLastError() << endl;
        return false;
    }
    cout << "Adrese adaptera: " << endl;
    for(ptrInfo = info, i=1; ptrInfo != nullptr; ptrInfo = ptrInfo->ai_next, i++)
    {
        if (ptrInfo->ai_family == AF_INET)
        {
            ipv4 = (sockaddr_in *)ptrInfo->ai_addr;
            adresa = &(ipv4->sin_addr);
            ipverzija = (char *)"IPv4";
        }
        else
        {
            ipv6 = (sockaddr_in6 *)ptrInfo->ai_addr;
            adresa = &(ipv6->sin6_addr);
            ipverzija = (char *)"IPv6";
        }
        inet_ntop(ptrInfo->ai_family, adresa, (PSTR)ipstr, sizeof(ipstr));
        printf("Adapter %d (%s): %s\n", i, ipverzija, ipstr);
    }
    freeaddrinfo(info);
    return true;
}

/* prema RFC2553 sockaddr_storage je univerzalno skladište i za IPv4 i za IPv6 adresu, zajedno sa portom*/
sockaddr_storage upisiAdresu(const char* adresaUnos, short port)
{
    int iRes;
    sockaddr_storage adresa;
    memset(&adresa, 0, sizeof(adresa));
    iRes=inet_pton(AF_INET,adresaUnos,&((sockaddr_in *)&adresa)->sin_addr);
    if(iRes != 0)
    {
        adresa.ss_family=AF_INET;
        ((sockaddr_in*)&adresa)->sin_port=htons(port);
    }
    else
    {
        iRes=inet_pton(AF_INET6,adresaUnos,&((sockaddr_in6 *)&adresa)->sin6_addr);
        if(iRes != 0)
        {
            adresa.ss_family=AF_INET6;
            ((sockaddr_in6*)&adresa)->sin6_port=htons(port);
        }
    }
    return adresa;
}

char* procitajAdresu(sockaddr_storage adresa, bool ispisiPort = true)
{
    static char strRes[INET6_ADDRSTRLEN+10];
    char ipstr[INET6_ADDRSTRLEN];
    int port;
    void* pAdresa = nullptr;
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
    {
        strcpy(strRes,"");
        return strRes;
    }
    inet_ntop(adresa.ss_family, pAdresa, (PSTR)ipstr, sizeof(ipstr));
    if(ispisiPort)
        if (adresa.ss_family == AF_INET)
            sprintf(strRes,"%s:%d",ipstr,port);
        else
            sprintf(strRes,"[%s]:%d",ipstr,port);
    else
        if (adresa.ss_family == AF_INET)
            sprintf(strRes,"%s",ipstr);
        else
            sprintf(strRes,"%s",ipstr);
    return strRes;
}

char* imeHosta(sockaddr_storage adresa)
{
    static char strRes[NI_MAXHOST+NI_MAXSERV+10];
    char host[NI_MAXHOST];
    char server[NI_MAXSERV];
    int iRes;
    iRes = getnameinfo((sockaddr *)&adresa, sizeof(adresa),host, sizeof(host), server, sizeof(server), 0);
    if(iRes == 0)
        sprintf(strRes,"%s, %s", host, server);
    else
        strcpy(strRes, "");
    return strRes;
}


vector<sockaddr_storage> adresaHosta(const char *host, const char *server, bool tcp = false)
{
    vector<sockaddr_storage> adrese;
    int iRes;
    addrinfo upute;
    addrinfo *rezultat, *ptrRezultat;
    memset(&upute, 0, sizeof(struct addrinfo));
    upute.ai_family = AF_UNSPEC; //IPv4 i IPv6
    if(tcp == false)
    {
        upute.ai_socktype = SOCK_DGRAM;
        upute.ai_protocol = IPPROTO_UDP;
    }
    else
    {
        upute.ai_socktype = SOCK_STREAM;
        upute.ai_protocol = IPPROTO_TCP;
    }

    iRes = getaddrinfo(host, server, &upute, &rezultat);
    if(iRes != 0)
    {
        cout << "Get addr info greska." << endl;
        return adrese;
    }

    ptrRezultat = rezultat;
    while(ptrRezultat != nullptr)
    {
        if(ptrRezultat->ai_family==AF_INET || ptrRezultat->ai_family==AF_INET6)
            adrese.push_back(*((sockaddr_storage*)ptrRezultat->ai_addr));
        ptrRezultat=ptrRezultat->ai_next;
    }

    freeaddrinfo(rezultat);
    return adrese;
}

int main()
{
    unsigned int brojJezgri;
    if(pokreniWinSock() != 0)
        return 1;
    if(dohvatiAdaptere() == false)
    {
        WSACleanup();
        return 1;
    }
    brojJezgri = thread::hardware_concurrency();
    cout << "Broj jezgri: " << brojJezgri << endl;

    sockaddr_storage adresa;
    adresa = upisiAdresu("192.168.56.1",80);
    cout << procitajAdresu(adresa) << endl;

    adresa = upisiAdresu("fe80::98d5:9938:b30b:2694",80);
    cout << procitajAdresu(adresa) << endl;
    cout << "Host: " << imeHosta(adresa) << endl;

    vector<sockaddr_storage> adrese;
    adrese = adresaHosta("VioletStorm3","");
    cout << "Broj adresa za VioletStorm3: " << adrese.size() << endl;
    for(auto it=adrese.begin(); it!=adrese.end(); it++)
        cout << procitajAdresu(*it, false) << endl;

    adrese = adresaHosta("www.google.com","",true);
    cout << "Broj adresa za www.google.com: " << adrese.size() << endl;
    for(auto it=adrese.begin(); it!=adrese.end(); it++)
        cout << procitajAdresu(*it, false) << endl;
    cout << "Host: " << imeHosta(*(adrese.begin())) << endl;

    WSACleanup();
    return 0;
}
