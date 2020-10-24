#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>

// #pragma comment( lib, "Ws2_32.lib" )

using namespace std;

/// Vraæa pokazivaè na vezanu listu dobavljenih adresa
ADDRINFO** DobaviAdrese(int spec = AF_UNSPEC)
{
    ADDRINFO hints;
    ADDRINFO *info = new ADDRINFO();
    ADDRINFO *ptr;
    char ime[80];
    char *adresa;

    gethostname(ime,sizeof(ime));
    cout << "Ime hosta: " << ime << endl;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = spec; //ako tu postavimo AF_INET6, uzimamo samo IP6
    hints.ai_protocol = IPPROTO_TCP; //TCP
    hints.ai_socktype = SOCK_STREAM;
    char *port = new char[6];
    strcpy(port,"5000");
    if(getaddrinfo(ime,port,&hints, &info)==0) //ime moze i null
    for(ptr=info; ptr!=nullptr; ptr=ptr->ai_next)
    {
        if(ptr->ai_family==AF_INET)
        {
            adresa=new char[INET_ADDRSTRLEN];
            sockaddr_in *in = (sockaddr_in *)ptr->ai_addr;
            inet_ntop(AF_INET,&(in->sin_addr), adresa, INET_ADDRSTRLEN);
            cout << "IPv4 adresa: " << adresa << endl;
            delete[] adresa;
        }
        else if(ptr->ai_family==AF_INET6)
        {
            adresa=new char[INET6_ADDRSTRLEN];
            sockaddr_in6 *in = (sockaddr_in6 *)ptr->ai_addr;
            inet_ntop(AF_INET6,&(in->sin6_addr), adresa, INET6_ADDRSTRLEN);
            cout << "IPv6 adresa: " << adresa << endl;
            delete[] adresa;
        }
    }
    return &info;
}

int main()
{
    WSAData wsaData;
    //potrebno dodati libwsock32 u projekt
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;

    ADDRINFO** info = DobaviAdrese(AF_INET6);



    if(info!=nullptr) //nullprt zahtijeva C++11
        freeaddrinfo(*info);
    WSACleanup();
    system("PAUSE");
    return 0;
}

