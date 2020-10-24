#include <iostream>
#include <winsock2.h>
#include <windows.h>

#define _WIN32_WINNT _WIN32_WINNT_WIN8
#include <ws2tcpip.h>

// #pragma comment( lib, "Ws2_32.lib" )

using namespace std;

ADDRINFO** DobaviAdresu()
{
    ADDRINFO hints;
    ADDRINFO *info = new ADDRINFO();
    ADDRINFO *ptr;
    sockaddr *sa;
    char ime[80];
    char *adresa = new char[64];
    DWORD adrlen = 24;
    gethostname(ime,sizeof(ime));
    cout << "Ime hosta: " << ime << endl;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //uzimamo samo IP6
    hints.ai_protocol = IPPROTO_TCP; //TCP
    hints.ai_socktype = SOCK_STREAM;
    char *port = new char[6];
    strcpy(port,"5000");
    if(getaddrinfo(ime,port,&hints, &info)==0)
    for(ptr=info; ptr!=nullptr; ptr=ptr->ai_next)
    {
        if(ptr->ai_family==AF_INET)
        {
            sockaddr_in *in = (sockaddr_in *)ptr->ai_addr;
            char ipAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&(in->sin_addr), adresa, adrlen);
            cout << "IPv4 adresa: " << adresa << endl;
        }
        else if(ptr->ai_family==AF_INET6)
        {
            sockaddr_in6 *in = (sockaddr_in6 *)ptr->ai_addr;
            char ipAddress[INET_ADDRSTRLEN];
            //inet_ntop(AF_INET6,&(in->sin_addr), adresa, adrlen);
            cout << "IPv6 adresa: " << adresa << endl;
        }
    }
    delete[] adresa;
    return &info;
}

int main()
{
    WSAData wsaData;
    //potrebno dodati libwsock32 u projekt
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return 0;
    cout << "Aktiviran je WSA." << endl;

    ADDRINFO** info = DobaviAdresu();



    if(info!=nullptr) //nullprt zahtijeva C++11
        delete *info;
    WSACleanup();
    system("PAUSE");
    return 0;
}

static char *
inet_ntop6(const u_char *src, char *dst, socklen_t size)
{
	/*
	 * Note that int32_t and int16_t need only be "at least" large enough
	 * to contain a value of the specified size.  On some systems, like
	 * Crays, there is no such thing as an integer variable with 16 bits.
	 * Keep this in mind if you think this function should have been coded
	 * to use pointer overlays.  All the world's not a VAX.
	 */
	char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
	struct { int base, len; } best, cur;
#define NS_IN6ADDRSZ	16
#define NS_INT16SZ	2
	u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;

	/*
	 * Preprocess:
	 *	Copy the input (bytewise) array into a wordwise array.
	 *	Find the longest run of 0x00's in src[] for :: shorthanding.
	 */
	memset(words, '\0', sizeof words);
	for (i = 0; i < NS_IN6ADDRSZ; i++)
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
	best.base = -1;
	best.len = 0;
	cur.base = -1;
	cur.len = 0;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		} else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	/*
	 * Format the result.
	 */
	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base &&
		    i < (best.base + best.len)) {
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		/* Are we following an initial run of 0x00s or any real hex? */
		if (i != 0)
			*tp++ = ':';
		/* Is this address an encapsulated IPv4? */
		if (i == 6 && best.base == 0 && (best.len == 6 ||
		    (best.len == 7 && words[7] != 0x0001) ||
		    (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
				return (NULL);
			tp += strlen(tp);
			break;
		}
		tp += std::sprintf(tp, "%x", words[i]); // ****
	}
	/* Was it a trailing run of 0x00's? */
	if (best.base != -1 && (best.base + best.len) ==
	    (NS_IN6ADDRSZ / NS_INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	/*
	 * Check for overflow, copy, and we're done.
	 */
	if ((socklen_t)(tp - tmp) > size) {
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}
