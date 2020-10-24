#include "uticnice.h"

using namespace std;

int main()
{
    const internet_protokoli intProtokol = internet_protokoli::TCP;
    char podaci[1024];
    sockaddr_storage adresa;
    int winsock;
    int uticnica;
    if((winsock = pokreniWinSock()) == -1)
        return 1;
    uticnica = kreirajUticnicu(intProtokol,IP_protokoli::IPv6);

    strcpy(podaci, "test");
    adresa = upisiAdresu("fe80::9e1:a78f:f313:1e20", 10000);
    cout << "Adresa: " << prikaziAdresu(adresa) << endl;
    posaljiPodatke(uticnica,intProtokol, podaci, 1024, adresa);


    WSACleanup();
    cout << "Winsock je oslobodjen." << endl;
    return 0;
}

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

char* prikaziAdresu(sockaddr_storage adresa, bool ispisiPort)
{
    static char strRezultat[INET6_ADDRSTRLEN+10];
    char ipstr[INET6_ADDRSTRLEN];
    short port;
    void* pAdresa = nullptr;
    if (adresa.ss_family == PF_INET)
    {
        port = ntohs(((sockaddr_in *)&adresa)->sin_port);
        pAdresa = &(((sockaddr_in *)&adresa)->sin_addr);
    }
    else if(adresa.ss_family == PF_INET6)
    {
        port = ntohs(((sockaddr_in6 *)&adresa)->sin6_port);
        pAdresa = &(((sockaddr_in6 *)&adresa)->sin6_addr);
    }
    if(pAdresa == nullptr)
    {
        strcpy(strRezultat,"");
        return strRezultat;
    }
    inet_ntop(adresa.ss_family, pAdresa, (PSTR)ipstr, sizeof(ipstr));
    if(ispisiPort)
        if (adresa.ss_family == AF_INET)
            sprintf(strRezultat,"%s:%d",ipstr,port);
        else
            sprintf(strRezultat,"[%s]:%d",ipstr,port);
    else
        if (adresa.ss_family == AF_INET)
            sprintf(strRezultat,"%s",ipstr);
        else
            sprintf(strRezultat,"%s",ipstr);
    return strRezultat;
}

int kreirajUticnicu(internet_protokoli iproto, IP_protokoli ipp)
{
    int uticnica;
    addrinfo upute;
    if(ipp == IP_protokoli::IPv6)
        upute.ai_family   = PF_INET6;
    else
        upute.ai_family   = PF_INET;
    if(iproto == internet_protokoli::UDP)
        upute.ai_socktype = SOCK_DGRAM;
    else
        upute.ai_socktype = SOCK_STREAM;

    uticnica = socket(upute.ai_family, upute.ai_socktype, 0);
    if (uticnica < 0)
    {
        cout << "Nije uspjelo kreiranje uticnice." << endl;
        return -1;
    }
    else
        cout << "Kreirana je uticnica " << uticnica << endl;
    return uticnica;
}




void posaljiPodatke(int uticnica, internet_protokoli iproto, char* buffer, int velicina, sockaddr_storage adresa)
{
    char bufferRcv[16384];
    int adresaVelicina, n;

    adresaVelicina = sizeof(adresa);
    if(iproto == internet_protokoli::UDP)
    {
        n = sendto(uticnica, buffer, velicina, 0, (sockaddr *)&adresa, adresaVelicina);
        if(n >= 0)
        {
            cout << "Poslano " << n << " bajtova zahtjeva." << endl;
            n = recvfrom(uticnica, bufferRcv, 16384, 0,(sockaddr *)&adresa, &adresaVelicina);
            if(n>=0)
                cout << "Primljeno je " << n << " bajtova odgovora." << endl;
            else
                cout << "Greska u primanju odgovora: " << WSAGetLastError() << endl;
        }

        else
            cout << "Greska u slanju: " << WSAGetLastError() << endl;
    }
    else
    {
        n = connect(uticnica, (sockaddr *)&adresa, adresaVelicina);
        if(n>=0)
        {
            cout << "Uspostavljena je veza sa serverom na adresi " << prikaziAdresu(adresa) << endl;
            n = send(uticnica, buffer, velicina, 0);
            if(n>=0)
            {
                cout << "Poslano je " << n << " bajtova zahtjeva." << endl;
                n = recv(uticnica, bufferRcv, 16384, 0);
                if(n>=0)
                    cout << "Primljeno je " << n << " bajtova odgovora." << endl;
                else
                    cout << "Greska u primanju odgovora: " << WSAGetLastError() << endl;
            }
            else
                cout << "Greska u slanju zahtjeva: " << WSAGetLastError() << endl;
            closesocket(uticnica);
        }
        else
            cout << "Greska u uspostavljanju veze: " << WSAGetLastError() << endl;

    }
}

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


