#include "uticnice.h"

using namespace std;

int main()
{
    const internet_protokoli intProtokol = internet_protokoli::TCP;
    char hostname[256];
    char ch = 0;
    int i, n;
    int winsock;
    int uticnica;
    thread* dretvaTipkovnice;

    sockaddr_storage* adreseAdaptera = nullptr;
    if((winsock = pokreniWinSock()) == -1)
        return 1;
    gethostname(hostname,256);
    n = dohvatiAdaptere(hostname, &adreseAdaptera);
    for(i=0; i<n; i++)
    {
        cout << "Adapter " << i << " (" << (adreseAdaptera[i].ss_family == PF_INET?"IPv4":"IPv4" ) << ") " <<
        prikaziAdresu(adreseAdaptera[i], false) << endl;
    }

    //uticnica = veziUticnicu(hostname,internet_protokoli::UDP,IP_protokoli::IPv6,10000);
    //uticnica = veziUticnicu("fe80::98d5:9938:b30b:2694", internet_protokoli::UDP, 10000);
    uticnica = veziUticnicu(intProtokol, adreseAdaptera, 1, 10000);

    dretvaTipkovnice = new thread(&slusaj, ref(uticnica), intProtokol);

    while(ch != 27)
        ch = _getch();
    closesocket(uticnica);
    uticnica = -1;

    dretvaTipkovnice->join();
    delete dretvaTipkovnice;

    if(adreseAdaptera != nullptr)
        delete[] adreseAdaptera;
    WSACleanup();
    cout << "Winsock je oslobodjen." << endl;
    return 0;
}

/** \brief Za Windows OS, pokreæe se WinSock API (WSA)
 * \return 0 za uspjesno pokretanje, inace ovisi o pogresci
 */

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

/** \brief pretvara prezentaciju adrese (tekstualni opis i brojcani port)
 *      u zapis u sockaddr_storage strukturi
 *  Vrijedi i za IPv4 i za IPv6 adrese.
 * \param adresaUnos tekstualni unos adrese (IPv4 ili IPv6)
 * \param numericki unos porta od 0 do 65535
 * \return sockaddr_storage univerzalna struktura za zapis IPv4 i IPv6 adresa
 *
 */

sockaddr_storage upisiAdresu(const char* adresaUnos, short port)
{
    int iRes;
    sockaddr_storage adresa;
    memset(&adresa, 0, sizeof(adresa)); /**< obavezno postaviti sve u strukturi na 0 inace IPv6 adrese nece biti ispravne */
    iRes=inet_pton(PF_INET,adresaUnos,&((sockaddr_in *)&adresa)->sin_addr);
    if(iRes != 0)
    {
        adresa.ss_family=PF_INET;
        ((sockaddr_in*)&adresa)->sin_port=htons(port);
    }
    else
    {
        iRes=inet_pton(PF_INET6,adresaUnos,&((sockaddr_in6 *)&adresa)->sin6_addr);
        if(iRes != 0)
        {
            adresa.ss_family=PF_INET6;
            ((sockaddr_in6*)&adresa)->sin6_port=htons(port);
        }
    }
    return adresa;
}

/** \brief Prikazuje prezentaciju adrese zapisane u sockaddr_storage strukturu
 *  Vrijedi i za IPv4 i za IPv6 adrese. Na osnovu zapisa u prezentaciji odreðuje vrstu adrese.
 * \param adresa zapis adrese u sockaddr_storage strukturi
 * \return vraca tekstualni prikaz adrese (prezentaciju)
 */

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

/** \brief Kreira i veze uticnicu za odredjeni internet protokol (TCP ili UDP) i odredjenu IP adresu (IPv4 ili IPv6)
 *      dobivenu iz imena hosta
 * Proces ide principom pokusaj pogreska. Prva uticnica koja zadovoljava uvjete i uspije se vezati, je izabrana.
 * \param host naziv hosta na kojem se trazi prikladna adresa adaptera
 * \param internet protokol (TCP ili UDP) oblika internet_protokol::UDP ili internet_protokol::TCP
 * \param IP protokol (IPv4 ili IPv6 adresa) oblika IP_protokoli::IPv4 ili IP_protokoli::IPv6
 * \param port vrijednost porta u prezentacijskom obliku (od 0 do 65535)
 * \return vraca opisnik uticnice kao int vrijednost
 *
 */

int veziUticnicu(const char* host, internet_protokoli iproto, IP_protokoli ipp, short port)
{
    int n, uticnica;
    sockaddr_storage* adresa = nullptr;
    addrinfo upute, *info, *ptInfo;
    memset(&upute, 0, sizeof(upute));
    upute.ai_flags    = AI_PASSIVE;
    if(ipp == IP_protokoli::IPv6)
        upute.ai_family   = PF_INET6;
    else
        upute.ai_family   = PF_INET;
    if(iproto == internet_protokoli::UDP)
        upute.ai_socktype = SOCK_DGRAM;
    else
        upute.ai_socktype = SOCK_STREAM;
    n = getaddrinfo(host, "", &upute, &info);
    if(n < 0)
    {
        cout << "Greska pri dobavljanju adresa hosta: " << WSAGetLastError() << endl;
        return -1;
    }
    ptInfo = info;
    uticnica = -1;
    while(ptInfo != nullptr)
    {
        /**< Trazimo adapter ciju uticnicu mozemo koristiti */
        uticnica = socket(ptInfo->ai_family, ptInfo->ai_socktype, 0);
        if (uticnica >= 0) //ako je uspjelo kreirati uticnicu
        {
            /**< Upisujemo zeljeni port */// upisujemo zeljeni port
            if(ipp == IP_protokoli::IPv6)
                ((sockaddr_in6 *)ptInfo->ai_addr)->sin6_port = htons(port);
            else
                ((sockaddr_in *)ptInfo->ai_addr)->sin_port = htons(port);
            //pokusavamo vezati uticnicu
            if (bind(uticnica, ptInfo->ai_addr, ptInfo->ai_addrlen) == 0)
            {
                adresa = (sockaddr_storage *)ptInfo->ai_addr;
                cout << "Povezana je " << (iproto == internet_protokoli::UDP?"UDP":"TCP") <<
                " uticnica na adresi " << prikaziAdresu(*adresa) << endl;
                break;
            }
            else
            {
                closesocket(uticnica);
                uticnica = -1;
            }

        }
        ptInfo = ptInfo->ai_next; /**< info je vezana lista, idemo na slijedeceg */
    }
    freeaddrinfo(info); //brisemo vezanu listu
    return uticnica;
}

/** \brief Kreira i veze uticnicu na osnovu tekstualne (prezentacijske) adrese i porta
 * Vrijedi i za IPv4 i za IPv6 adrese, ali upisana adresa mora biti ispravna
 * \param strAdresa adresa zapisana u prezentacijskom obliku
 * \param internet protokol (TCP ili IP)
 * \param port brojcana vrijednost porta od 0 do 65535
 * \return vraca opisnik uticnice kao int vrijednost
 *
 */

int veziUticnicu(const char* strAdresa, internet_protokoli iproto, short port)
{
    int uticnica;
    sockaddr_storage adresa;
    adresa = upisiAdresu(strAdresa, port);
    if(iproto == internet_protokoli::UDP)
        uticnica = socket(adresa.ss_family, SOCK_DGRAM, 0);
    else
        uticnica = socket(adresa.ss_family, SOCK_STREAM, 0);
    if (uticnica < 0)
    {
        cout << "Nije uspjelo kreiranje uticnice." << endl;
        return -1;
    }
    if (bind(uticnica, (sockaddr*)&adresa, sizeof(adresa)) == 0)
    {
        cout << "Povezana je " << (iproto == internet_protokoli::UDP?"UDP":"TCP") << " uticnica na adresi " << prikaziAdresu(adresa) << endl;
    }
    else
    {
        cout << "Nije uspjelo vezanje uticnice na adresi " << prikaziAdresu(adresa) << endl;
        cout << "Greska: " << WSAGetLastError() << endl;
        closesocket(uticnica);
        return -1;
    }
    return uticnica;
}

/** \brief Kreira i veze uticnicu iz polja sockaddr_storage struktura izborom rednog broja
 *      pocevsi od nule
 *
 * \param iproto internet protokol (TCP ili UDP)
 * \param adreseAdaptera polje sockaddr_storage struktura adresa
 * \param brojAdaptera redni broj elementa polja adaptera
 * \param port brojcana vrijednost porta od 0 do 65535
 * \return vraca opisnik uticnice kao int vrijednost
 *
 */

int veziUticnicu(internet_protokoli iproto, sockaddr_storage* adreseAdaptera, int brojAdaptera, short port)
{
    int uticnica;
    sockaddr_storage *ptAdresa;
    if(adreseAdaptera == nullptr)
        return -1;
    ptAdresa = adreseAdaptera+brojAdaptera;
    if(iproto == internet_protokoli::UDP)
        uticnica = socket(ptAdresa->ss_family, SOCK_DGRAM, 0);
    else
        uticnica = socket(ptAdresa->ss_family, SOCK_STREAM, 0);
    if (uticnica < 0)
    {
        cout << "Nije uspjelo kreiranje uticnice." << endl;
        return -1;
    }

    if(ptAdresa->ss_family == PF_INET6)
        ((sockaddr_in6 *)ptAdresa)->sin6_port = htons(port);
    else
        ((sockaddr_in *)ptAdresa)->sin_port = htons(port);

    if (bind(uticnica, (sockaddr*)ptAdresa, sizeof(sockaddr_storage)) == 0)
    {
        cout << "Povezana je " << (iproto == internet_protokoli::UDP?"UDP":"TCP") << " uticnica na adresi " << prikaziAdresu(*ptAdresa) << endl;
    }
    else
    {
        cout << "Nije uspjelo vezanje uticnice na adresi " << prikaziAdresu(*ptAdresa) << endl;
        cout << "Greska: " << WSAGetLastError() << endl;
        closesocket(uticnica);
        return -1;
    }
    return uticnica;
}

void slusaj(int& uticnica, internet_protokoli iproto)
{
    char bufferSnd[16384];
    char bufferRcv[1024];
    int uticnica2;
    sockaddr_storage adresaKlijenta;
    int duljinaKlijenta, n;
    duljinaKlijenta = sizeof(adresaKlijenta);

    if(iproto == internet_protokoli::UDP)
    {
        cout << "UDP uticnica ceka poruku klijenta." << endl;
        while(uticnica>=0)
        {
            n = recvfrom(uticnica, bufferRcv, 1024, 0, (sockaddr *)&adresaKlijenta, &duljinaKlijenta);
            if(n>=0)
            {
                cout << "Primljeno je " << n << " bajtova zahtjeva na uticnici " << uticnica << endl;
                n = sendto(uticnica, bufferSnd, 8192, 0, (sockaddr *)&adresaKlijenta, duljinaKlijenta);
                if(n>=0)
                    cout << "Poslano je kao odgovor " << n << " bajtova." << endl;
                else
                    cout << "Greska u slanju odgovora: " << WSAGetLastError() << endl;
            }
            else
                cout << "Greska u primanju zahtjeva: " << WSAGetLastError() << endl;
        };
    }
    else
    {
        listen(uticnica, 3);
        while(uticnica>=0)
        {
            cout << "TCP konekcijska uticnica ceka klijentov zahtjev za spajanje..." << endl;
            uticnica2 = accept(uticnica, (sockaddr *)&adresaKlijenta, &duljinaKlijenta);
            if(uticnica2 >= 0)
            {
                cout << "TCP veza sa klijentom na adresi " << prikaziAdresu(adresaKlijenta, false) << " je uspostavljena." << endl;
                n = recv(uticnica2, bufferRcv, 1024,0);
                if(n>=0)
                {
                    cout << "Primljeno je " << n << " bajtova zahtjeva." << endl;
                    n = send(uticnica2, bufferSnd, 8192, 0);
                    if(n>=0)
                        cout << "Poslano je kao odgovor " << n << " bajtova." << endl;
                    else
                        cout << "Greska u slanju odgovora: " << WSAGetLastError() << endl;
                }
                else
                    cout << "Greska u primanju zahtjeva: " << WSAGetLastError() << endl;

                closesocket(uticnica2);
            }
            else
                cout << "Greska pri uspostavi veze: " << WSAGetLastError() << endl;
        }
    }
    return;
}
/** \brief Dohvaca adrese svih dostupnih mreznih adaptera
 *
 * \param hostname ime domacina (hosta) ciji se adapteri ispituju
 * \param pokazivac na polje sockaddr_storage struktura koje se popunjavaju
 * \return broj adaptera
 *
 */

int dohvatiAdaptere(const char* hostname, sockaddr_storage** saAdrese)
{
    sockaddr_storage *ptrSaAdrese;
    int iRes, n;
    addrinfo upute;
    addrinfo *info, *ptrInfo;

    memset(&upute, 0, sizeof(upute));
    upute.ai_family = AF_UNSPEC;
    upute.ai_socktype = SOCK_STREAM;
    iRes = getaddrinfo(hostname, nullptr, &upute, &info);
    if(iRes!=0)
    {
        cout << "Nije uspjelo dobavljanje adresnih informacija (WSA: %d), " << WSAGetLastError() << endl;
        return 0;
    }
    for(ptrInfo = info, n = 0; ptrInfo != nullptr; ptrInfo=ptrInfo->ai_next, n++);
    if(n==0)
        return 0;
    *saAdrese = new sockaddr_storage[n];
    for(ptrInfo = info, ptrSaAdrese=*saAdrese; ptrInfo != nullptr; ptrInfo = ptrInfo->ai_next, ptrSaAdrese++)
        memcpy(ptrSaAdrese, (sockaddr_storage*)ptrInfo->ai_addr, sizeof(sockaddr_storage));
    freeaddrinfo(info);
    return n;
}
