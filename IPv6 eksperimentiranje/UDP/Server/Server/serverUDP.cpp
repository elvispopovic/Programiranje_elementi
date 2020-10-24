#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>
#include <Mstcpip.h>
#include <thread>
#define MAX_MSG 8192

struct paket
{
    int redoslijed;
    int velPodataka;
    char podaci[1];
};

using namespace std;
ADDRINFO* DobaviAdrese(int spec = AF_UNSPEC);
void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort=true);
SOCKET OtvoriUticnicu(ADDRINFO *adrese, unsigned short port);
void Primanje(SOCKET s);

int main()
{
    WSAData wsaData;
    ADDRINFO *adrese;
    SOCKET slusanje;
    char unos[6];
    int tip;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        cout << "Greska: Winsock start." << endl;
        return 0;
    }

    cout << "Unesi 6 za IPv6 adrese." << endl;
    cin >> tip;
    cin.clear();
    cin.ignore();

    if(tip==6)
        adrese=DobaviAdrese(AF_INET6);
    else
        adrese=DobaviAdrese(AF_INET);

    if(adrese!=nullptr)
    {
        slusanje = OtvoriUticnicu(adrese, 5000);
        freeaddrinfo(adrese);
    }
    else
    {
        cout << "Greska pri otvaranju uticnice." << endl;
        return 0;

    }
    if(slusanje==INVALID_SOCKET)
    {
        cout << "Nije uspjelo kreiranje uticnice za slusanje." << endl;
        WSACleanup();
        return 0;
    }

    thread t1(Primanje,slusanje);
    cout << "Dretva za primanje je pokrenuta. Za kraj pritisni enter." << endl;
    cin.getline(unos,6);

    t1.detach();

    WSACleanup();
    return 0;
}

ADDRINFO* DobaviAdrese(int spec)
{
    ADDRINFO sugestije;
    ADDRINFO *adrese, *pa;
    sockaddr_storage addr = {};
    char domacinIme[80];
    gethostname(domacinIme,80);
    cout << "Ime domacina: " << domacinIme << endl;
    memset(&sugestije, 0, sizeof(sugestije));
    sugestije.ai_family = spec; //uzimamo samo onu vrstu koja je specificirana
    sugestije.ai_protocol = IPPROTO_UDP;
    sugestije.ai_socktype = SOCK_DGRAM;
    if(getaddrinfo(domacinIme,"5000",&sugestije, &adrese)==0)
    {
        cout << "Adrese adaptera:" << endl;
        for(pa=adrese; pa!=nullptr; pa=pa->ai_next) //idemo po vezanoj listi adresa; nullprt zahtijeva C++11
        {
            memcpy(&addr,pa->ai_addr,pa->ai_addrlen);
            IspisiAdresu(addr, false);
        }
        return adrese;
    }
    return nullptr;
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
        cout << ispisAdrese << endl;
    else
        cout << ispisAdrese << " , port " << port << endl;
}

SOCKET OtvoriUticnicu(ADDRINFO *adrese, unsigned short port)
{
    ADDRINFO *pa;
    SOCKET uticnica;
    sockaddr_storage addr = {};
    char ispisAdrese[INET6_ADDRSTRLEN];
    socklen_t addrLen = sizeof(sockaddr_storage);
    for(pa=adrese; pa!=nullptr; pa=pa->ai_next)
    {
        if((uticnica=socket(pa->ai_family, SOCK_DGRAM, IPPROTO_UDP))== INVALID_SOCKET)
                continue;
        if(pa->ai_family==AF_INET)
            ((sockaddr_in *)pa->ai_addr)->sin_port = htons(port);
        else if(pa->ai_family==AF_INET6)
            ((sockaddr_in6 *)pa->ai_addr)->sin6_port = htons(port);
        memcpy(&addr,pa->ai_addr,pa->ai_addrlen);
        if(bind(uticnica, (sockaddr *)&addr, addrLen)==INVALID_SOCKET)
        {
            closesocket(uticnica);
            return INVALID_SOCKET;
        }
        else
        {
            cout << "Otvorena je uticnica na ";
            IspisiAdresu(addr);
            return uticnica;
        }

    }
}

void Primanje(SOCKET s)
{
    paket *pak;
    int pakLen = MAX_MSG;
    sockaddr_storage adresa;
    socklen_t addrLen = sizeof(adresa);
    int procitano;

    pak = (paket *)new char[MAX_MSG];
    do
    {
        ZeroMemory(pak,pakLen);
        procitano=recvfrom(s,(char *)pak, pakLen, 0,(sockaddr *)&adresa, &addrLen);
        if(procitano>0)
        {
            IspisiAdresu(adresa);
            cout << pak->redoslijed << ": velicina: " << procitano << endl;
        }
        else
            cout << "Greska na socketu." << endl;

    }while(1);
}


