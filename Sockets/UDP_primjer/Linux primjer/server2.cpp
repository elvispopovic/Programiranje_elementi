#include <stdlib.h> /* atoi(), exit()*/

#include <arpa/inet.h>

# include <ifaddrs.h>

#include <unistd.h>

#include <stdio.h> /* printf(); alternativno cout i iostream*/

#include <string.h> /* memset() */

 

#define MAX_MSG 10000

 

int main(int argc, char *argv[]) {

  int udpSock, rc, i, j, n;

  socklen_t cliLen;

  struct sockaddr_in cliAddr;

  char meduspremnik[MAX_MSG];

  ifaddrs *adrese = NULL, *pAdrese;
  void* pAdresa;

  char adresaPrikaz[INET6_ADDRSTRLEN];
  char adresaIspis[64];
  int portIspis;
  char ipAdresa[20];
  char hostname[256];
  short portNum;
  sockaddr_storage odabranaAdresa;
 

  if(argc!=2){

    printf("Krivi broj argumenata!\nUpotreba programa: %s brojPorta\n",argv[0]);

    exit(1);

    }

   portNum=atoi(argv[1]);

  /* host name */
  gethostname(hostname, sizeof(hostname));
  printf("Host: %s\n", hostname);

  getifaddrs(&adrese);
  for(i=1, pAdrese = adrese; pAdrese != NULL; pAdrese = pAdrese->ifa_next)
  {
    if(pAdrese->ifa_addr->sa_family == AF_INET)
    {
      pAdresa = &((sockaddr_in*)pAdrese->ifa_addr)->sin_addr;
      inet_ntop(AF_INET, pAdresa, adresaPrikaz, INET_ADDRSTRLEN);
      printf("%d: %s (IPv4)\n",i,adresaPrikaz);
      i++;
    }
    else if(pAdrese->ifa_addr->sa_family == AF_INET6)
    {
      pAdresa = &((sockaddr_in6*)pAdrese->ifa_addr)->sin6_addr;
      inet_ntop(AF_INET6, pAdresa, adresaPrikaz, INET6_ADDRSTRLEN);
      printf("%d: %s (IPv6)\n", i, adresaPrikaz);
      i++;
    }	
  }

  do
  {
    printf("Unesi broj (od 1 do %d)!\n", i-1);
    scanf("%d",&j);
    
  }while(j<1 || j>=i);

  for(i=1, pAdrese = adrese; pAdrese != NULL; pAdrese = pAdrese->ifa_next)
  {
    if(i == j)
    {
      memset(&odabranaAdresa, 0, sizeof(odabranaAdresa));
      memcpy(&odabranaAdresa, (sockaddr_storage*)pAdrese->ifa_addr, sizeof(sockaddr_storage));
      if(odabranaAdresa.ss_family == AF_INET)
        ((sockaddr_in*)&odabranaAdresa)->sin_port = htons(portNum);
      else if(odabranaAdresa.ss_family == AF_INET6)
        ((sockaddr_in6*)&odabranaAdresa)->sin6_port = htons(portNum);
      break;      
    }
    if(pAdrese->ifa_addr->sa_family == AF_INET || pAdrese->ifa_addr->sa_family == AF_INET6)
      i++;
  }

  if(adrese != NULL)
     freeifaddrs(adrese);

  if(odabranaAdresa.ss_family == AF_INET)
  {
    inet_ntop(odabranaAdresa.ss_family, &(((sockaddr_in*)&odabranaAdresa)->sin_addr),adresaIspis, sizeof(adresaIspis)); 
    portIspis = ntohs(((sockaddr_in*)&odabranaAdresa)->sin_port);
    printf("Odabrana je IPv4 adresa %s:%d\n",adresaIspis, portIspis);
  }  
  else
  {
    inet_ntop(odabranaAdresa.ss_family, &(((sockaddr_in6*)&odabranaAdresa)->sin6_addr), adresaIspis, sizeof(adresaIspis));
    portIspis = ntohs(((sockaddr_in6*)&odabranaAdresa)->sin6_port);
    printf("Odabrana je IPv6 adresa [%s]:%d\n", adresaIspis, portIspis);    
  }

  /* stvaranje socketa */

  udpSock=socket(odabranaAdresa.ss_family, SOCK_DGRAM, 0);

  if(udpSock<0) {

    printf("%s: ne mogu otvoriti soket \n",argv[0]);


    exit(1);

  }

 

  /* povezivanje posluzitelja s portom */


  rc = bind (udpSock, (sockaddr *) &odabranaAdresa,sizeof(odabranaAdresa));

  if(rc<0) {

    printf("%s: ne mogu se povezati s portom broj %d \n", argv[0], portNum);

    exit(1);

  }
 

  printf("%s: cekam podatke\n", hostname);

 

  while(1) {

    /* popuni meduspremnik nulama */

    memset(meduspremnik,0,MAX_MSG);

 

    /* primi poruku */

    cliLen = sizeof(cliAddr);

    n = recvfrom(udpSock, meduspremnik, MAX_MSG, 0/*flags=0*/,(struct sockaddr *) &cliAddr, &cliLen);

 

    if(n<0) {

      printf("%s: ne mogu primiti podatke \n",argv[0]);

      continue;

    }

    /* ispis primljene poruke */

    printf("%s:od %s:UDP%u : %s \n", argv[0],inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port),meduspremnik);

       /*posalji poruku*/

    sendto(udpSock,meduspremnik,n,0,(struct sockaddr *)&cliAddr,cliLen);

  }

return 0;

}
