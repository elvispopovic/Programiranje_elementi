#include "uticnice.h"

using namespace std;

int main(int argc, char** argv)
{
    int n;
    char adresaUnos[INET6_ADDRSTRLEN];
    unsigned short port, portUnos;
    veza_kontekst vp;
    sockaddr_storage* adreseAdaptera = nullptr;
    sockaddr_storage adresa;
    int winsock;
    thread* dretva;

    if((winsock = pokreniWinSock()) == -1)
        return 1;

	/* unos adrese */
	cout << "Unesi IP adresu posluzitelja (IPv4 ili IPv6)!" << endl;
	cin >> adresaUnos;	
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
	adresa = upisiAdresu(adresaUnos, portUnos);
		
	/* prikaz adrese i porta za test */
	cout << "Unesena je adresa " <<  prikaziAdresu(adresa, true) << endl;
	

    if(poveziSe(adresa, &vp) == false)
        return 1;


    memcpy(&vp.adresa, &adresa, sizeof(adresa));

    dretva = new thread(&komuniciraj, &vp);


    PetljaPoruka(&vp);

    zatvoriKonekciju(&vp);
    dretva->join();
    delete dretva;

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

/** \brief prezentacija adrese unesena kao sockaddr_storage strukture uz posebni unos porta
 * \param adresa zapis adrese u sockaddr_storage strukturi
 * \param port numericki zapis porta koji ce se ugraditi u adresnu strukturu
 * \return prezentacija adrese u prikladnoj IPv4 ili IPv6 notaciji uz zapis porta
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


/** \brief Daje korisniku mogucnost odabira adrese sa popisa adresa mreznih adaptera
 *
 * \param adreseAdaptera lista adresa koje se prosljedjuju na biranje
 * \param nAdaptera broj adaptera u listi
 * \param port port koji ce se prikljuciti adresi
 * \return adresa u univerzalnoj sockaddr_storage strukturi (IPv4 ili IPv6)
 */

/** \brief povezuje se sa serverom i stvara komunikacijski kontekst
 *
 * \param adresaServera adresa servera na koju se klijent spaja. Sadrzi i port.
 * \param veza pokazivac na kontekst
 * \return boolean koji daje uspjesnost povezivanja
 */
bool poveziSe(sockaddr_storage adresaServera, veza_kontekst* veza)
{
    int n;
    unsigned long modUticnice = 1;

    veza->uticnica = socket(adresaServera.ss_family, SOCK_STREAM, 0);
    if (veza->uticnica < 0)
    {
        cout << "Nije uspjelo kreiranje uticnice." << endl;
        veza->uticnica = -1;
        return false;
    }
    else
        cout << "Kreirana je uticnica " << veza->uticnica << endl;

    veza->indeksSlanja = -1;
    veza->indeksPrimanja = 0;
    n = connect(veza->uticnica, (sockaddr *)&adresaServera, sizeof(adresaServera));
    if(n<0)
    {
        cout << "Nije uspjelo spajanje uticnice na server: " << WSAGetLastError() << endl;
        closesocket(veza->uticnica);
        veza->uticnica = -1;
        return false;
    }
    else
    {
        Ispisi("Povezano sa serverom na %s\n", prikaziAdresu(adresaServera) );
        Ispisi("Spremno za slanje pritiskom na tipku S.\n");
    }

    ioctlsocket(veza->uticnica, FIONBIO, &modUticnice);
    return true;
}

/** \brief komunikacijska petlja sa vremenski blokirajucim selectom
 *
 * \param veza pokazivac na kontekst
 * \return nema povratne vrijednosti
 */
void komuniciraj(veza_kontekst* veza)
{
    int najvisaUticnica, dogadjaj;
    fd_set fds_citanje, fds_pisanje, fds_iznimka;

    cekanje = new timeval;
    cekanje->tv_sec = 0;
    cekanje->tv_usec = 250000;
    while(1)
    {
        FD_ZERO(&fds_citanje);
        FD_ZERO(&fds_pisanje);
        FD_ZERO(&fds_iznimka);

        FD_SET(veza->uticnica, &fds_citanje);
        if(!veza->redPoruka.empty())
            FD_SET(veza->uticnica, &fds_pisanje);
        FD_SET(veza->uticnica, &fds_iznimka);
        najvisaUticnica = veza->uticnica;

        dogadjaj = select(najvisaUticnica + 1, &fds_citanje, &fds_pisanje, &fds_iznimka, cekanje);
        switch(dogadjaj)
        {
            case -1:
                if(WSAGetLastError() != WSAENOTSOCK)
                {
                    cout << "Greska pri izboru dogadjaja: "<< dogadjaj << ", greska: " << WSAGetLastError() << endl;
                    zatvoriKonekciju(veza);
                }
                return;
            case 0:
                if(veza->uticnica >= 0)
                    if (slanjePremaKonekciji(veza) == false)
                    {
                        zatvoriKonekciju(veza);
                        continue;
                    }
            default:
                if (veza->uticnica != -1)
                {
                    if (FD_ISSET(veza->uticnica, &fds_citanje))
                    {
                        if (primanjeOdKonekcije(veza) == false)
                        {
                            zatvoriKonekciju(veza);
                            return;
                        }

                    }
                    if (FD_ISSET(veza->uticnica, &fds_pisanje))
                    {
                        if (slanjePremaKonekciji(veza) == false)
                        {
                            zatvoriKonekciju(veza);
                            return;
                        }

                    }

                    if (FD_ISSET(veza->uticnica, &fds_iznimka))
                    {
                        Ispisi("Iznimka na uticnici %d.\n", veza->uticnica);
                        zatvoriKonekciju(veza);
                        return;
                    }
                }
        }
    };
}

/** \brief omogucava slanje poruke pritiskom na tipku S
 *
 * \param veza pokazivac na kontekst
 * \return nema povratne vrijednosti
 */
void PetljaPoruka(veza_kontekst* listaVeza)
{
    char ch = 0;
    int i;
    while(ch != 27)
    {
        ch = _getch();
        //ch = 'S';
        //Sleep(500);
        if(toupper(ch) == 'S')
        {
            poruka p;
            strcpy(p.ime_posiljatelja,"KLIJENT");
            if(listaVeza->uticnica >= 0)
            {
                if(listaVeza->redPoruka.size()>MAX_PORUKA_NA_CEKANJU)
                {
                    cout << "Red poruka je prepunjen." << endl;
                    continue;
                }
                else
                {
                    p.velicina = 2048+(rand()%1024);
                    p.kontrolna_suma = 0;
                    for(i=0; i<p.velicina; i++)
                    {
                        p.podaci[i]=(unsigned char)i;
                        p.kontrolna_suma+=p.podaci[i];
                    }
                    mtxKontekst.lock();
                    listaVeza->redPoruka.push(p);
                    cout << "Dodana je nova poruka velicine " << p.velicina << " u red poruka sa kontrolnom sumom " << p.kontrolna_suma << "." << endl;
                    mtxKontekst.unlock();
                }
            }
        }
    }
}
/** \brief pretvara prezentaciju adrese u obliku adresa-port u sockaddr_storage kontejner
 *
 * \param adresaUnos prezentacijski oblik adrese (IPv4 ili IPv6)
 * \param port numericki zapis porta
 * \return zapis adrese u sockaddr_storage strukturi
 */
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

/** \brief zatvara konekciju za odredjeni kontekst
 *
 * \param veza pokazivac na kontekst
 * \return nema povratne vrijednosti
 */
void zatvoriKonekciju(veza_kontekst* konekcija)
{
    lock_guard<mutex> brava(mtxKontekst);
    closesocket(konekcija->uticnica);
    konekcija->uticnica = -1;
    delete cekanje;
    cekanje = nullptr;
    while(!(konekcija->redPoruka.empty()))
        konekcija->redPoruka.pop();
    konekcija->indeksPrimanja = 0;
    konekcija->indeksSlanja = -1;
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
            Ispisi("Primljena je poruka od servera sa %s velicine %d bajtova.\n", prikaziAdresu(veza->adresa), veza->indeksPrimanja);
            Ispisi("Upisana velicina: %d\n", veza->trenutnoPrimanje.velicina);
            Ispisi("Upisana kontrolna suma: %d, izracunata suma: %d, %s\n",
                   veza->trenutnoPrimanje.kontrolna_suma, kontrolna_suma, (veza->trenutnoPrimanje.kontrolna_suma==kontrolna_suma?" O.K.":" ERR"));
            veza->indeksPrimanja = 0;
        }
        potrebnoPrimiti = velicina - veza->indeksPrimanja;
        if(potrebnoPrimiti > VELICINA_PRIMANJA)
            potrebnoPrimiti = VELICINA_PRIMANJA;
        primljeno = recv(veza->uticnica, (char *)&veza->trenutnoPrimanje + veza->indeksPrimanja, potrebnoPrimiti,0);
        if (primljeno < 0)
        {
            if (WSAGetLastError() == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK)
            {
                Ispisi("Server je prekinuo slanje.\n");
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
                Ispisi("Server je zavrsio primanje.\n");
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


