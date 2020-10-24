#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>
#include <Mstcpip.h>
#include <thread>


using namespace std;

void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort);
SOCKET PosaljiZahtijevZaVezu(sockaddr_storage adresa);
bool UnesiAdresu(sockaddr_storage *adresa);

void Slanje(SOCKET server)
{
    int kolicina=1024; 
    int recvbufferlen = kolicina;
    int sent;
    char *sendbuffer = new char[kolicina];
    do
    {
        ZeroMemory(sendbuffer,sizeof(sendbuffer));
        cin >> sendbuffer;
        sent=send(server,sendbuffer,strlen(sendbuffer),0);
    }while(strcmp(sendbuffer,"#q")!=0&&(sent!=-1)&&server!=INVALID_SOCKET);
    closesocket(server);
    delete[] sendbuffer;
}

void Primanje(SOCKET server)
{
    int recvbufferlen = 1024;
    int readed;
    char *recvbuffer = new char[recvbufferlen];
    do
    {
        ZeroMemory(recvbuffer,recvbufferlen);
        readed=recv(server,recvbuffer,recvbufferlen,0);
        if(readed>0)
            cout << "Primljeno: " << recvbuffer << endl;
        else
            cout << "Veza je prekinuta." << endl;
    }while(readed>0);
    delete[] recvbuffer;
}


int main()
{
    WSAData wsaData;
    SOCKET server;
    sockaddr_storage adresa={};
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;
    if(UnesiAdresu(&adresa)==false)
    {
        WSACleanup();
        return 0;
    }
    cout << "Unesena je adresa ";
    IspisiAdresu(adresa, true);
    cout << endl;
    server = PosaljiZahtijevZaVezu(adresa);
    if(server == INVALID_SOCKET)
    {
        cout << "Nije uspjela uspostava veze. Mozda nije pokrenut server." << endl;
        WSACleanup();
        system("PAUSE");
        return 0;
    }
    cout << "Uspostavljena je veza." << endl;
    cout << "Utipkani sadrzaj bit ce poslan serveru." << endl << "Od servera primljeni sadrzaj biti ce ispisan." << endl;
    cout << "--------------------------" << endl;
    thread t1(Slanje,server);
    thread t2(Primanje,server);

    t1.join();
    t2.join();

    WSACleanup();
    system("PAUSE");
    return 0;
}

bool UnesiAdresu(sockaddr_storage *adresa)
{
    int iRes, numerickiUnos;
    char charUnos[64];
    cout << "Unesite adresu servera (IPv4 ili IPv6): ";
    cin >> charUnos;
    do
    {
        cout << "Unesite port servera: ";
        cin >> numerickiUnos;
        if(numerickiUnos<0||numerickiUnos>65535)
            cout << "Neispravan unos porta." << endl;
    }while(numerickiUnos<0||numerickiUnos>65535);

    iRes=inet_pton(AF_INET,charUnos,&((sockaddr_in *)adresa)->sin_addr);
    adresa->ss_family=AF_INET;
    ((sockaddr_in *)adresa)->sin_port=htons(numerickiUnos);
    if(iRes<=0)
    {
        iRes=inet_pton(AF_INET6,charUnos,&((sockaddr_in6 *)adresa)->sin6_addr);
        adresa->ss_family=AF_INET6;
        ((sockaddr_in6 *)adresa)->sin6_port=htons(numerickiUnos);
    }
    if(iRes<=0)
    {
        cout << "Neispravno unesena adresa." << endl;
        return false;
    }
    return true;
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

SOCKET PosaljiZahtijevZaVezu(sockaddr_storage adresa)
{
    SOCKET uticnica;
        if((uticnica=socket(adresa.ss_family, SOCK_STREAM, IPPROTO_TCP))== INVALID_SOCKET)
            return INVALID_SOCKET;
        if (connect(uticnica, (sockaddr *)&adresa, sizeof(adresa)) == SOCKET_ERROR)
        {
            cout << "Nije uspjelo povezivanje sa serverom." << endl;
            closesocket(uticnica);
            return SOCKET_ERROR;
        }
        else
        {
            cout << "Otvorena je uticnica na adresi ";
            IspisiAdresu(adresa, true);
            return uticnica;
        }
}
