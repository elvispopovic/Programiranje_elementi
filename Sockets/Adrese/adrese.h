#ifndef ADRESE_H_INCLUDED
#define ADRESE_H_INCLUDED

#define _WIN32_WINNT 0x0600

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

enum internet_protokoli:int {TCP, UDP};
enum IP_protokoli:int{IPv4, IPv6};

int pokreniWinSock();

/* server */
char* prikaziAdresu(sockaddr_storage adresa, bool ispisiPort = true);
int veziUticnicu(const char* host, internet_protokoli iproto, IP_protokoli ipp, short port);
int veziUticnicu(const char* strAdresa, internet_protokoli iproto, short port);
int veziUticnicu(internet_protokoli iproto, sockaddr_storage* adreseAdaptera, int brojAdaptera, short port);
int dohvatiAdaptere(const char* hostname, sockaddr_storage** saAdrese);


#endif // ADRESE_H_INCLUDED
