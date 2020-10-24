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
void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort=true);
SOCKET OtvoriUticnicu(int spec=AF_INET);
void Slanje(SOCKET s, sockaddr_storage adresa);

int main()
{
    WSAData wsaData;
    hostent *host;
    SOCKET uticnica;
    sockaddr_storage adresa={};

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;

    /*
    host = gethostbyname("VioletStormII");
    if(host==NULL)
    {
        cout << "Ne mogu pronaci host servera." << endl;
        return 0;
    }
    cout << "Host je pronadjen." << endl;
    */

    /*
    uticnica = OtvoriUticnicu(AF_INET);
    ((sockaddr_in *)&adresa)->sin_family = AF_INET;
    inet_pton(AF_INET,"192.168.8.101",&(((sockaddr_in *)&adresa)->sin_addr));
    ((sockaddr_in *)&adresa)->sin_port=htons(5000);
    */
    uticnica = OtvoriUticnicu(AF_INET6);
    ((sockaddr_in6 *)&adresa)->sin6_family = AF_INET6;
    inet_pton(AF_INET6,"fe80::d5f8:192d:2833:71ad",&(((sockaddr_in6 *)&adresa)->sin6_addr));
    ((sockaddr_in6 *)&adresa)->sin6_port=htons(5000);
    IspisiAdresu(adresa);
    Slanje(uticnica,adresa);


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
        if(ispisiPort==false)
            cout << ispisAdrese << endl;
        else
            cout << ispisAdrese << " , port " << port << endl;
    }
}


SOCKET OtvoriUticnicu(int spec)
{
    SOCKET uticnica;
    if((uticnica=socket(spec, SOCK_DGRAM, IPPROTO_UDP))== INVALID_SOCKET)
        cout << "Nije uspjelo kreiranje UDP uticnice." << endl;
    cout << "Uticnica je kreirana." << endl;
    return uticnica;
}

void Slanje(SOCKET s, sockaddr_storage adresa)
{
    int iRes, brojac;
    char *buffer;
    paket *pak = (paket *)new char[MAX_MSG];
    buffer = new char[5000000];
    char *ptr;
    int velPodataka = MAX_MSG-sizeof(paket);
    for(brojac=0, ptr=&buffer[0];ptr<&buffer[5000000];ptr+=velPodataka,brojac++)
    {
        pak->redoslijed=brojac;
        pak->velPodataka=(5000000-brojac*velPodataka)<velPodataka?(5000000-brojac*velPodataka):velPodataka;
        memcpy(pak->podaci,ptr,pak->velPodataka);
        iRes=sendto(s,(char *)pak,MAX_MSG,0,(struct sockaddr *)&adresa,sizeof(adresa));
        cout << "Poslano " << iRes << " bajtova." << endl;
    }

    delete[] pak;
    delete[] buffer;
}


