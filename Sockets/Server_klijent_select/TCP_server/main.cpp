#include "uticnice.h"

using namespace std;

int main()
{
    const internet_protokoli intProtokol = internet_protokoli::TCP;
    static veza_kontekst listaVeza[MAX_CLIENTS];
    veza_kontekst *ptListaVeza;
    char hostname[256];

    int i, n;
    int winsock;
    int uticnica, loopback;
    unsigned short portUnos;
    thread* dretva;

    sockaddr_storage* adreseAdaptera = nullptr;
    sockaddr_storage odabranaAdresa;
    if((winsock = pokreniWinSock()) == -1)
        return 1;
    gethostname(hostname,256);
    cout << "Host: " << hostname << endl;
    n = dohvatiAdaptere(hostname, &adreseAdaptera);
    for(i=0; i<n; i++)
    {
        cout << "Adapter " << (i+1) << " (" << (adreseAdaptera[i].ss_family == PF_INET?"IPv4":"IPv4" ) << ") " <<
        prikaziAdresu(adreseAdaptera[i], false) << endl;
    }
    odabranaAdresa = odaberiAdapter(adreseAdaptera, n);
    
    /* unos porta */
	do
	{
		cout << "Unesi port!" << endl;
		cin >> portUnos;
		if(cin.fail())
        {
        	cin.clear();
        	cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
	}while(portUnos < 10000 || portUnos > 40000);

    for(i=0, ptListaVeza=listaVeza; i<MAX_CLIENTS; i++, ptListaVeza++)
    {
        ptListaVeza->uticnica = -1;
    }

    uticnica = veziUticnicu(intProtokol, &odabranaAdresa, portUnos);
    if(uticnica <0)
    {
        WSACleanup();
        return -1;
    }

    dretva = new thread(&slusaj, listaVeza, ref(uticnica));

    loopback = KreirajLoopback(odabranaAdresa);

    PetljaPoruka(listaVeza, loopback);
    zatvoriUticnice(listaVeza, uticnica);

    dretva->join();
    delete dretva;

    closesocket(loopback);

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

sockaddr_storage odaberiAdapter(sockaddr_storage* adreseAdaptera, int nAdaptera)
{
    sockaddr_storage odabrano;
    sockaddr_storage *ptAdrese;
    int i;
    do
    {
        Ispisi("Unesi redni broj adrese:\n");
        cin >> i;
        if(cin.fail() || i<1 || i>nAdaptera)
        {
            cin.clear();
            cin.ignore();
            Ispisi("Greska. Broj mora biti izmedju 1 i %d.\n", nAdaptera);
        }
    }while(i<1 || i>nAdaptera);
    odabrano = adreseAdaptera[i-1];
    return odabrano;
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

int veziUticnicu(internet_protokoli iproto, sockaddr_storage* adresa, short port)
{
    int uticnica;
    char ponovnoKoristenje;
    sockaddr_storage *ptAdresa;
    ptAdresa = adresa;
    if(iproto == internet_protokoli::UDP)
        uticnica = socket(ptAdresa->ss_family, SOCK_DGRAM, 0);
    else
        uticnica = socket(ptAdresa->ss_family, SOCK_STREAM, 0);
    if (uticnica < 0)
    {
        cout << "Nije uspjelo kreiranje uticnice." << endl;
        return -1;
    }
    ponovnoKoristenje = 1;
    if (setsockopt(uticnica, SOL_SOCKET, SO_REUSEADDR, &ponovnoKoristenje, sizeof(ponovnoKoristenje)) != 0)
    {
        cout << "Greska pri postavljanju uticnice na ponovno koristenje." << endl;
        closesocket(uticnica);
        return -1;
    }


    if(ptAdresa->ss_family == PF_INET6)
        ((sockaddr_in6 *)ptAdresa)->sin6_port = htons(port);
    else
        ((sockaddr_in *)ptAdresa)->sin_port = htons(port);

    if (bind(uticnica, (sockaddr*)ptAdresa, sizeof(sockaddr_storage)) == 0)
    {
        Ispisi("Povezana je %s uticnica na adresi %s\n", (iproto == internet_protokoli::UDP?"UDP":"TCP"), prikaziAdresu(*ptAdresa) );
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


/** \brief Dohvaca adrese svih dostupnih mreznih adaptera
 *
 * \param hostname ime domacina (hosta) ciji se adapteri ispituju
 * \param adrese pokazivac na polje sockaddr_storage struktura koje se popunjavaju
 * \return broj adaptera
 *
 */
int dohvatiAdaptere(const char* hostname, sockaddr_storage** adrese)
{
    sockaddr_storage *ptrAdrese;
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
    *adrese = new sockaddr_storage[n];
    for(ptrInfo = info, ptrAdrese=*adrese; ptrInfo != nullptr; ptrInfo = ptrInfo->ai_next, ptrAdrese++)
        memcpy(ptrAdrese, (sockaddr_storage*)ptrInfo->ai_addr, sizeof(sockaddr_storage));
    freeaddrinfo(info);
    return n;
}


/** \brief Glavna komunikacijska petlja koja pokrece slusanje i dvosmjernu komunikaciju sa spojenim klijentima
 *
 * \param listaVeza polje adresa povezanih klijenata
 * \param uticnica uticnica slusanja koja se prosljedjuje referencom
 * \return nema povratne vrijednosti
 *
 */
void slusaj(veza_kontekst* listaVeza, int& uticnica)
{
    int i, najvisaUticnica, dogadjaj;
    veza_kontekst *ptListaVeza;
    fd_set fds_citanje, fds_pisanje, fds_iznimka;

    if(listen(uticnica, 3)<0)
    {
        Ispisi("Greska prilikom postavljanja uticnice u stanje slusanja: %d\n", WSAGetLastError());
        return;
    }
    while(1)
    {
        FD_ZERO(&fds_citanje);
        FD_ZERO(&fds_pisanje);
        FD_ZERO(&fds_iznimka);
        FD_SET(uticnica, &fds_citanje);
        FD_SET(uticnica, &fds_iznimka);
        najvisaUticnica = uticnica;
        for (i = 0, ptListaVeza = listaVeza; i < MAX_CLIENTS; ++i, ptListaVeza++)
        {
            if (ptListaVeza->uticnica != -1)
            {
                FD_SET(ptListaVeza->uticnica, &fds_citanje);
                if(!ptListaVeza->redPoruka.empty())
                    FD_SET(ptListaVeza->uticnica, &fds_pisanje);
                FD_SET(ptListaVeza->uticnica, &fds_iznimka);
            }
            if(ptListaVeza->uticnica > najvisaUticnica)
                najvisaUticnica = ptListaVeza->uticnica;

        }
        /* dogadjaji ukljucuju primljene podatke na uticnicu (bilo koju), te nastavak nedovrsenog slanja sa bilo koje uticnice kao i razne iznimke */
        dogadjaj = select(najvisaUticnica + 1, &fds_citanje, &fds_pisanje, &fds_iznimka, NULL);
        switch(dogadjaj)
        {
            case -1:
                if(WSAGetLastError() != WSAENOTSOCK)
                {
                    cout << "Greska pri izboru dogadjaja: "<< dogadjaj << ", greska: " << WSAGetLastError() << endl;
                    zatvoriUticnice(listaVeza, uticnica);
                }
                return;
            case 0:
                for (i = 0, ptListaVeza = listaVeza; i < MAX_CLIENTS; ++i, ptListaVeza++)
                    if(ptListaVeza->uticnica >= 0)
                        if (slanjePremaKonekciji(ptListaVeza) == false)
                        {
                            zatvoriKonekciju(ptListaVeza);
                            continue;
                        }

            default:
                if (FD_ISSET(uticnica, &fds_citanje))
                {
                    uspostavaKonekcije(listaVeza, uticnica);
                }
                if (FD_ISSET(uticnica, &fds_iznimka))
                {
                    cout << "Iznimka na uticnici za uspostavu veze." << endl;
                    zatvoriUticnice(listaVeza, uticnica);
                    return;
                }
                for (i = 0, ptListaVeza = listaVeza; i < MAX_CLIENTS; ++i, ptListaVeza++)
                {
                    if (ptListaVeza->uticnica != -1)
                    {
                        if(FD_ISSET(ptListaVeza->uticnica, &fds_citanje))
                        {
                            if (primanjeOdKonekcije(ptListaVeza) == false)
                            {
                                zatvoriKonekciju(ptListaVeza);
                                continue;
                            }

                        }
                        if(FD_ISSET(ptListaVeza->uticnica, &fds_pisanje))
                        {
                            if (slanjePremaKonekciji(ptListaVeza) == false)
                            {
                                zatvoriKonekciju(ptListaVeza);
                                continue;
                            }

                        }
                        if(FD_ISSET(ptListaVeza->uticnica, &fds_iznimka))
                        {
                            cout << "Iznimka na uticnici " << ptListaVeza->uticnica << endl;
                            zatvoriKonekciju(ptListaVeza);
                            continue;
                        }
                    }
                }
        }//end of dogadjaj
    }//end of while
}

/** \brief Kreira uticnicu kojom se otkljucava blokirani select na primjer sa tipkovnice
 * \param adresa adresa servera, ovog istog s kojim se povezuje loopback uticnica
 * \return vraca rukovatelj loopback uticnice
 *
 */
int KreirajLoopback(sockaddr_storage adresa)
{
    int n, uticnica;
    uticnica = socket(adresa.ss_family, SOCK_STREAM, 0);
    if (uticnica < 0)
    {
        Ispisi("Nije uspjelo kreiranje loopback uticnice. Greska: %s\n", WSAGetLastError());
        return -1;
    }
    n = connect(uticnica, (sockaddr *)&adresa, sizeof(adresa));
    if(n<0)
    {
        Ispisi("Nije uspjelo povezivanje loppback uticnice na server. Greska: %s\n", WSAGetLastError());
        closesocket(uticnica);
        return -1;
    }
    return uticnica;
}

/** \brief sadrzi accept koji prihvaca vezu sa klijentom
 * \param listaVeza polje konteksta povezanih klijenata
 * \param uticnica rukovatelj uticnice za slusanje
 * \return boolean koji vraca uspjesnost
 *
 */
bool uspostavaKonekcije(veza_kontekst* listaVeza, int uticnica)
{
    int i, uticnica2;
    veza_kontekst *ptListaVeza;
    sockaddr_storage* ptAdresa;
    int duljina;

    unsigned long modUticnice = 1; //uticnice su neblokirajuce (vrijednost 1)
    uticnica2 = -1;
    for(i=0, ptListaVeza = listaVeza; i<MAX_CLIENTS; i++, ptListaVeza++)
    {
        if(ptListaVeza->uticnica == -1)
        {
            ptAdresa = &(ptListaVeza->adresa);
            duljina = sizeof(ptListaVeza->adresa);
            memset(ptAdresa, 0, duljina);
            uticnica2 = accept(uticnica, (sockaddr *)ptAdresa, &duljina);
            if(uticnica2 < 0)
            {
                if(WSAGetLastError() != WSAENOTSOCK)
                    cout << "Greska prilikom uspostave konekcije: " << WSAGetLastError() << endl;
                return false;
            }
            Ispisi("TCP veza sa klijentom na adresi %s je uspostavljena.\n",prikaziAdresu(*ptAdresa, false) );
            Ispisi("Spremno za slanje pritiskom na tipku S.\n");

            ioctlsocket(uticnica2, FIONBIO, &modUticnice); //postavlja se uticnica na neblokirajucu
            ptListaVeza->uticnica = uticnica2;
            ptListaVeza->indeksPrimanja = 0;
            ptListaVeza->indeksSlanja = -1;
            return true;
        }
    }
    cout << "Premasen je maksimalni broj uticnica." << endl;
    return false;
}

/** \brief omogucava slanje poruke pritiskom na tipku S, pri cemu salje poruku preko loopback uticnice
 *
 * \param listaVeza polje komunikacijskih konteksta. U ovom primjeru svima se salje ista poruka.
 * \param loopback rukovatelj loopback uticnice
 * \return nema povratne vrijednosti
 */
void PetljaPoruka(veza_kontekst* listaVeza, int loopback)
{
    char ch = 0;
    int i, j;
    while(ch != 27)
    {
        //ch = 'S';
        //Sleep(500);
        ch = _getch();
        if(toupper(ch) == 'S')
        {
            poruka p, lbp;
            strcpy(p.ime_posiljatelja,"SERVER");

            for(i=0; i<MAX_CLIENTS; i++)
            {
                if(listaVeza[i].uticnica >= 0)
                {
                    if(listaVeza[i].redPoruka.size()>MAX_PORUKA_NA_CEKANJU)
                    {
                        Ispisi("Red poruka za %d je prepunjen.\n", listaVeza[i].uticnica);
                        continue;
                    }
                    else
                    {
                        p.velicina = 2048+(rand()%2048);
                        p.kontrolna_suma = 0;
                        for(j=0; j<p.velicina; j++)
                        {
                            p.podaci[j]=(unsigned char)j;
                            p.kontrolna_suma+=p.podaci[j];
                        }
                        mtxKontekst.lock();
                        listaVeza[i].redPoruka.push(p);
                        Ispisi("Dodana je nova poruka u red poruka uticnice %d.\n", listaVeza[i].uticnica);
                        mtxKontekst.unlock();

                        lbp.velicina = 0;
                        lbp.kontrolna_suma = 0;
                        strcpy(lbp.ime_posiljatelja,"LOOPBACK");
                        send(loopback,(char *)&lbp,(DULJINA_IMENA_POSILJATELJA + 2*(int)sizeof(int)),0);
                    }
                }
            }
        }
    }
}

/** \brief zatvara konekciju za odredjeni kontekst
 *
 * \param konekcija pokazivac na kontekst veze sa klijentom
 * \return nema povratne vrijednosti
 */
void zatvoriKonekciju(veza_kontekst* konekcija)
{
    lock_guard<mutex> brava(mtxKontekst);
    cout << "Konekcija na " << konekcija->uticnica << " se zatvara." << endl;
    closesocket(konekcija->uticnica);
    konekcija->uticnica = -1;
    while(!(konekcija->redPoruka.empty()))
        konekcija->redPoruka.pop();
    konekcija->indeksPrimanja = 0;
    konekcija->indeksSlanja = -1;
}

/** \brief zatvara sve uticnice, odnosno veze
 *
 * \param listaVeza polje komunikacijskih konteksta.
 * \param uticnica uticnica za slusanje (nije u polju konteksta)
 * \return nema povratne vrijednosti
 */
void zatvoriUticnice(veza_kontekst* listaVeza, int& uticnica)
{
    lock_guard<mutex> brava(mtxKontekst);
    int i;
    veza_kontekst *ptListaVeza;
    if(uticnica != -1) //zatvara se uticnica slusanja
    {
        closesocket(uticnica);
        uticnica = -1;
    }
    /* zatvaraju se uticnice veza sa klijentima */
    for(i=0, ptListaVeza = listaVeza; i<MAX_CLIENTS; i++, ptListaVeza++)
    {
        if(ptListaVeza->uticnica != -1)
        {
            closesocket(ptListaVeza->uticnica);
            ptListaVeza->uticnica = -1;
        }
    }
    delete cekanje;
    cekanje = nullptr;
    cout << "Sve uticnice su zatvorene." << endl;
}

/** \brief petlja primanja poruke u malim komadima
 *
 * \param veza pokazivac na kontekst
 * \return boolean koji odredjuje ispravnost komunikacijskog kanala
 */
bool primanjeOdKonekcije(veza_kontekst* veza)
{
    int potrebnoPrimiti, primljeno;
    int i, velicina;
    unsigned int kontrolna_suma;
    do
    {
        /* prvo prima ime, velicinu i kontrolnu sumu */
        if(veza->indeksPrimanja < (DULJINA_IMENA_POSILJATELJA + 2*(int)sizeof(int)))
            velicina = (DULJINA_IMENA_POSILJATELJA + 2*(int)sizeof(int));
        /* kad je primio prethodno zna koliko još mora primiti jer je to zapisano u velicini poruke */
        else
            velicina = (DULJINA_IMENA_POSILJATELJA + 2*(int)sizeof(int)) + veza->trenutnoPrimanje.velicina;
        if (veza->indeksPrimanja >= velicina)
        {
            for(i=0, kontrolna_suma=0; i<veza->trenutnoPrimanje.velicina; i++)
                kontrolna_suma+=(unsigned char)(veza->trenutnoPrimanje.podaci[i]);
            Ispisi("Primljena je poruka sa %s velicine %d bajtova.\n", prikaziAdresu(veza->adresa, false), veza->indeksPrimanja);
            Ispisi("Upisana velicina: %d\n", veza->trenutnoPrimanje.velicina);
            Ispisi("Upisana kontrolna suma: %d, izracunata suma: %d, %s\n",
                   veza->trenutnoPrimanje.kontrolna_suma, kontrolna_suma, (veza->trenutnoPrimanje.kontrolna_suma==kontrolna_suma?" O.K.":" ERR"));
            veza->indeksPrimanja = 0;
        }
        /* racuna koliko je jos ostalo za primiti */
        potrebnoPrimiti = velicina - veza->indeksPrimanja;
        /* ako je to vece od limita, prima limit */
        if(potrebnoPrimiti > VELICINA_PRIMANJA)
            potrebnoPrimiti = VELICINA_PRIMANJA;
        primljeno = recv(veza->uticnica, (char *)&veza->trenutnoPrimanje + veza->indeksPrimanja, potrebnoPrimiti,0);
        if (primljeno < 0)
        {
            if (WSAGetLastError() == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK)
            {
                Ispisi("Klijent je prekinuo slanje.\n");
            }
            else
            {
                Ispisi("Greska u primanju: %d\n", WSAGetLastError());
                return false;
            }
        }
        else if (primljeno < 0 && (WSAGetLastError() == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK))
        {
            break;
        }
        else if (primljeno == 0)
        {
            Ispisi("Konekcija se zatvara jer je druga strana poslala praznu poruku.\n");
            return false;
        }
        else if (primljeno > 0)
        {
            veza->indeksPrimanja += primljeno;
            Ispisi("Primljeno je %d od potrebnih %d bajtova.\n", primljeno, velicina);
        }

    }while(primljeno>0);
    return true;
}

/** \brief petlja slanja poruke u malim komadima
 *
 * \param veza pokazivac na kontekst
 * \return boolean koji odredjuje ispravnost komunikacijskog kanala
 */
bool slanjePremaKonekciji(veza_kontekst* veza)
{
    int potrebnoPoslati, poslano, velicina;
    do
    {
        velicina = (DULJINA_IMENA_POSILJATELJA + 2*(int)sizeof(int)) + veza->trenutnoSlanje.velicina;
        if (veza->indeksSlanja < 0 || veza->indeksSlanja >= velicina)
        {
            if(!(veza->redPoruka.empty()))
            {
                lock_guard<mutex> brava(mtxKontekst);
                veza->trenutnoSlanje=veza->redPoruka.front();
                veza->redPoruka.pop();
                veza->indeksSlanja = 0;
                Ispisi("Slanje poruke velicine %d bajtova.\n", velicina);
            }
            else
            {
                if(veza->indeksSlanja != -1)
                {
                    Ispisi("Nema nista za slanje na uticnicu %d.\nUkupno poslano: %d bajtova.\n", veza->uticnica, veza->indeksSlanja);
                    Ispisi("Spremno za slanje (pritiskom na tipu S) na %s \n", prikaziAdresu(veza->adresa, false));
                    veza->indeksSlanja = -1;
                }
                break;
            }
        }
        potrebnoPoslati = velicina - veza->indeksSlanja;
        if (potrebnoPoslati > VELICINA_SLANJA)
            potrebnoPoslati = VELICINA_SLANJA;
        poslano = send(veza->uticnica, (char *)&veza->trenutnoSlanje + veza->indeksSlanja, potrebnoPoslati, 0);
        if (poslano < 0)
        {
            if (WSAGetLastError() == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK)
            {
                Ispisi("Klijent je zavrsio primanje.\n");
            }
            else
            {
                Ispisi("Greska u slanju: %d\n", WSAGetLastError());
                return false;
            }
        }
        else if (poslano < 0 && (WSAGetLastError() == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK))
        {
            break;
        }
        else if (poslano == 0)
        {
            Ispisi("Nije poslano nista (0 bajtova).\n");
            break;
        }
        else if (poslano > 0)
        {
            veza->indeksSlanja += poslano;
            Ispisi("Poslano je %d od potrebnih %d bajtova.\n", poslano, velicina);
        }
    }while(poslano>0);
    return true;
}



