#include "server.h"
//obavezno u projektu ukljuciti -std=c++11

using namespace std;


int main()
{
    SOCKET slusanje;
    SOCKET klijent;
    bool slanje_aktivno;
    unordered_map <SOCKET,sockaddr_storage> tablicaKlijenata;
    PokreniWSock(1,1);
    slusanje = SlusanjeZahtijevaZaVezu();
    slanje_aktivno=false;
    do
    {
        sockaddr_storage adresa = CekajVezu(slusanje, klijent);
        if(klijent==INVALID_SOCKET)
            cerr << "Nije uspojelo povezivanje s klijentom." << endl;
        else
        {
            cout << "Povezan je klijent na ";
            IspisiAdresu(adresa,true);
            cout << endl;
            tablicaKlijenata[klijent]=adresa;
            thread t1(Primanje,klijent, &tablicaKlijenata);
            t1.detach();
            if(slanje_aktivno==false)
            {
                thread t2(Slanje,klijent,&tablicaKlijenata,&slanje_aktivno);
                t2.detach();
            }
        }
    }while(1); //privremeno

    closesocket(slusanje);
    OslobodiResurse();
    return 0;
}

bool PokreniWSock(int glavnaVerzija, int minornaVerzija)
{
    int iRes;
    WSAData wsaData;
    iRes=WSAStartup(MAKEWORD(glavnaVerzija, minornaVerzija), &wsaData);
    if(iRes!=0)
    {
        switch(iRes)
        {
            case WSASYSNOTREADY:
                cerr << "Mrezni sustav nije spreman za komunikaciju." << endl;
                break;
            case WSAVERNOTSUPPORTED:
                cerr << "Verzija WinSock nije podrzana." << endl;
                break;
            case WSAEINPROGRESS:
                cerr << "Pokrenuta je blokirajuca WWinsock operacija." << endl;
                break;
            case WSAEPROCLIM:
                cerr << "Dosegnut je maksimum zadataka za Winsock." << endl;
                break;
            case WSAEFAULT:
                cerr << "Pridruzeni pokazivac nije ispravnog tipa." << endl;
                break;
        }
        return false;
    }
    cout << "Pokrenut je WSock " << int(LOBYTE(wsaData.wVersion)) << "." << int(HIBYTE(wsaData.wVersion)) << endl;
    cout << "Opis sustava: " << wsaData.szDescription << ", stanje sustava: " << wsaData.szSystemStatus << endl;
    cout << "Najvisa raspoloziva verzija: " << int(LOBYTE(wsaData.wHighVersion)) << "." << int(HIBYTE(wsaData.wHighVersion)) << endl;
    if(glavnaVerzija<2)
        cout << "Max. broj uticnica: " << wsaData.iMaxSockets << ", max. velicina datagrama: " << wsaData.iMaxUdpDg << endl;
    return true;
}
void OslobodiResurse()
{
    char c[3];
    WSACleanup();
    cout << "Svi resursi su oslobodjeni. Za izlaz pritisnite ENTER." << endl;
    cin.getline(c,3);
}
void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort, bool ispisiTip)
{
    unsigned short port;
    char ispisAdrese[INET6_ADDRSTRLEN];
    if(adresa.ss_family==AF_INET)
    {
        inet_ntop(AF_INET,&(((sockaddr_in *)&adresa)->sin_addr), ispisAdrese, INET_ADDRSTRLEN);
        port=ntohs(((sockaddr_in *)&adresa)->sin_port);
        cout << ispisAdrese;
        if(ispisiPort==true)
            cout << ":" << port;
        if(ispisiTip==true)
            cout << " (IPv4)";
    }
    else if(adresa.ss_family==AF_INET6)
    {
        inet_ntop(AF_INET6,&(((sockaddr_in6 *)&adresa)->sin6_addr), ispisAdrese, INET6_ADDRSTRLEN);
        port=ntohs(((sockaddr_in6 *)&adresa)->sin6_port);
        if(ispisiPort==false)
            cout << ispisAdrese;
        else
            cout << "[" << ispisAdrese << "]:" << port;
        if(ispisiTip==true)
            cout << " (IPv6)";
    }
}

SOCKET SlusanjeZahtijevaZaVezu()
{
    int iRes, i, izbor;
    sockaddr_storage adresa;
    vector<sockaddr_storage> listaAdresa;
    ADDRINFO upute={};
    ADDRINFO *info, *ptr;
    SOCKET uticnica;
    char domacinIme[256];
    char ispisAdrese[INET6_ADDRSTRLEN];
    upute.ai_family = AF_UNSPEC;
    upute.ai_socktype = SOCK_STREAM;
    upute.ai_protocol = IPPROTO_TCP;
    gethostname(domacinIme,256);
    getaddrinfo(domacinIme,"5000",&upute,&info); //ovdje moze i web adresa servera ili ime drugog hosta
    if(iRes!=0)
    {
        cerr << "Nije uspjelo dobavljanje adresnih informacija." << endl;
        return INVALID_SOCKET;
    }
    cout << "Domacin: " << domacinIme << ", lista adresa adaptera: " << endl;
    for(ptr=info; ptr!=nullptr; ptr=ptr->ai_next)
        if(ptr->ai_family==AF_INET||ptr->ai_family==AF_INET6)
        {
            memcpy(&adresa,ptr->ai_addr,ptr->ai_addrlen);
            listaAdresa.push_back(adresa);
        }
    if(listaAdresa.size()==0)
    {
        cerr << "Nema dostupnih adaptera." << endl;
        FreeAddrInfo(info);
        return INVALID_SOCKET;
    }
    do
    {
        i=0;
        for(auto it=listaAdresa.begin(); it!=listaAdresa.end(); it++, i++)
        {
            cout << i+1 << ": ";
            IspisiAdresu(*it,false,true);
            cout << endl;
        }
        cout << "Izaberi adresu: (1-" << listaAdresa.size() << "):";
        cin >> izbor;
        if(izbor<1||izbor>listaAdresa.size())
            cout << "Neispravan unos." << endl;
    }while(izbor<1||izbor>listaAdresa.size());
    adresa = listaAdresa.at(izbor-1);
    uticnica=socket(adresa.ss_family, upute.ai_socktype, upute.ai_protocol);
    if(uticnica==INVALID_SOCKET)
    {
        cerr << "Nije uspjelo kreiranje uticnice za uspostavu TCP veze." << endl;
        FreeAddrInfo(info);
        return INVALID_SOCKET;
    }
    do
    {
        cout << "Unesi port (10000-60000): ";
        cin >> izbor;
        if(izbor<10000||izbor>60000)
           cout << "Neispravan unos." << endl;
        else
        {
            if(adresa.ss_family==AF_INET)
                ((sockaddr_in*)&adresa)->sin_port=htons(izbor);
            else
                ((sockaddr_in6*)&adresa)->sin6_port=htons(izbor);
            iRes=bind(uticnica, (sockaddr*)&adresa,sizeof(adresa));
            if(iRes==SOCKET_ERROR)
            {
                cout << "Nije uspjelo vezanje porta. Mozda je port zauzet." << endl;
                izbor=0;
            }
            else
            {
                cout << "Uticnica je povezana sa adresom ";
                IspisiAdresu(adresa,true);
                cout << endl;
            }
        }
    }while(izbor<10000||izbor>60000);
    FreeAddrInfo(info);
    return uticnica;
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

void Primanje(SOCKET klijent, unordered_map<SOCKET,sockaddr_storage> *tablica)
{
    char *recvbuffer;
    int i;
    sockaddr_storage adresaKlijenta;
    recvbuffer  = new char[1048576];
    int recvLen = 1048576;
    int primljeno;
    char adresaIspis[INET6_ADDRSTRLEN];
    adresaKlijenta=tablica->find(klijent)->second;
    inet_ntop(adresaKlijenta.ss_family,(sockaddr*)&adresaKlijenta,adresaIspis,INET6_ADDRSTRLEN);
    do
    {
        memset(recvbuffer,0,1048576);
        primljeno = recv(klijent, recvbuffer, recvLen, 0);
        IspisiAdresu(adresaKlijenta,true,false);
        cout << ": procitano je " << primljeno << " bajtova." << endl;
        if(primljeno<1000)
            cout << recvbuffer << endl;
    }while(primljeno>0);
    tablica->erase(klijent);
    cout << "Veza sa klijentom na ";
    IspisiAdresu(adresaKlijenta,true,true);
    cout << " je prekinuta." << endl;
    delete[] recvbuffer;
}

void Slanje(SOCKET klijent, std::unordered_map <SOCKET,sockaddr_storage> *tablica, bool *slanjeAktivno)
{
    char *sendbuffer;
    sendbuffer = new char[1024];
    int poslano;
    *slanjeAktivno=true;
    cout << "Konzola za upis omogucena." << endl;
    do
    {
        memset(sendbuffer,0,1024);
        cin >> sendbuffer;
        cin.clear();
        cin.ignore();
        for(auto it=tablica->begin(); it!=tablica->end(); it++)
        {
            poslano = send(it->first,sendbuffer,strlen(sendbuffer),0);
            if(poslano==-1)
                break;
        }
    }while(poslano!=-1&&klijent!=INVALID_SOCKET);
    delete[] sendbuffer;
    cout << "Konzola onemogucena." << endl;
    *slanjeAktivno=false;
    closesocket(klijent);
}

