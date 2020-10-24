#ifndef UTICNICE_H_INCLUDED
#define UTICNICE_H_INCLUDED

#define _WIN32_WINNT 0x0600

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <mutex>
#include <conio.h>
#include <queue>

#define MAX_CLIENTS 1000
#define MAX_PORUKA_NA_CEKANJU 20
#define DULJINA_IMENA_POSILJATELJA 24
#define MAX_DULJINA_PODATAKA 4096
#define VELICINA_SLANJA 256
#define VELICINA_PRIMANJA 256


std::mutex mtxKonzola;
std::mutex mtxKontekst;

struct poruka
{
    char ime_posiljatelja[DULJINA_IMENA_POSILJATELJA];
    int velicina;
    unsigned int kontrolna_suma;
    unsigned char podaci[MAX_DULJINA_PODATAKA];
};

struct veza_kontekst
{
    int uticnica;
    sockaddr_storage adresa;

    std::queue<poruka> redPoruka;
    poruka trenutnoSlanje; //koristimo istu strukturu poruke i za zapis buffera - ista velicina
    int indeksSlanja;
    poruka trenutnoPrimanje;
    int indeksPrimanja;
};


enum internet_protokoli:int {TCP, UDP};
enum IP_protokoli:int{IPv4, IPv6};

timeval *cekanje;

int pokreniWinSock();

/* server */
char* prikaziAdresu(sockaddr_storage adresa, bool ispisiPort = true);
sockaddr_storage odaberiAdapter(sockaddr_storage* adreseAdaptera, int nAdaptera);
int veziUticnicu(internet_protokoli iproto, sockaddr_storage* adresa, short port);

int dohvatiAdaptere(const char* hostname, sockaddr_storage** saAdrese);
void slusaj(veza_kontekst* listaVeza, int& uticnica);

int KreirajLoopback(sockaddr_storage adresa);

void PetljaPoruka(veza_kontekst* listaVeza);
void PetljaPoruka(veza_kontekst* listaVeza, int loopback);
bool uspostavaKonekcije(veza_kontekst* listaVeza, int uticnica);
bool primanjeOdKonekcije(veza_kontekst* veza);
bool slanjePremaKonekciji(veza_kontekst* veza);
void zatvoriKonekciju(veza_kontekst* konekcija);

void zatvoriUticnice(veza_kontekst* listaVeza, int& uticnica);

/* klijent */
sockaddr_storage odaberiAdapter(sockaddr_storage* adreseAdaptera, int nAdaptera, unsigned short port);
bool poveziSe(sockaddr_storage adresaServera, veza_kontekst* veza);
void komuniciraj(veza_kontekst* veza);
sockaddr_storage upisiAdresu(const char* adresaUnos, short port);


void Ispisi(const char *format, ...)
{
    /* sam se otkljuca kad se izadje izvan dosega (scope) */
    std::lock_guard<std::mutex> brava(mtxKonzola);
    va_list argumenti;
    va_start(argumenti, format);
    vprintf(format, argumenti);
    va_end(argumenti);
}
#endif // UTICNICE_H_INCLUDED
