#ifndef __SERVER_H__
#define __SERVER_H__



#include <iostream>
#include <iomanip>

#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <Ws2tcpip.h>

#include <thread>
#include <unordered_map>
#include <vector>

bool PokreniWSock(int, int);
void OslobodiResurse();
void IspisiAdresu(sockaddr_storage adresa, bool ispisiPort, bool ispisiTip=false);
SOCKET SlusanjeZahtijevaZaVezu();
sockaddr_storage CekajVezu(SOCKET slusanje, SOCKET &klijent);

void Primanje(SOCKET, std::unordered_map <SOCKET,sockaddr_storage>*);
void Slanje(SOCKET, std::unordered_map <SOCKET,sockaddr_storage>*, bool*);
#endif // __SERVER_H__

