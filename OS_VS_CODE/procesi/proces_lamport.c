#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct _zajednicka
{
    float fVarijabla;
    int iVarijabla;
    int parentId;
    int polje[1];
} zajednicka;


int id_seg, Nprocesa;
zajednicka *zajed;

void udji_u_kriticni(int id)
{
    int *broj, *trazim;
    int i, max;
    broj = &(zajed->polje[0]);
    trazim =&(zajed->polje[Nprocesa]);

    trazim[id]=1;
    max = -1;
    for(i=0; i<Nprocesa; i++)
        if(broj[i]>max)
            max=broj[i];
    broj[id]=max+1;
    trazim[id]=0;
    for(i=0; i<Nprocesa; i++)
    {
        while(trazim[i] != 0)
            usleep(20000);
        while(broj[i] != 0 && (broj[i] < broj[id] || (broj[i] == broj[id] && i < id) ))
            usleep(20000);
    }
}

void izadji_iz_kriticnog(int id)
{
    int* broj = &(zajed->polje[0]);
    broj[id]=0;
}



void Citanje(int id)
{
    int i, k, n=5;
    while(n>0)
    {
        sleep(2);
        // Kriticni odsjecak
        udji_u_kriticni(id);
        printf("Iteracija %d, id %d: citanje zajednicke varijable tri puta:\n", n, id);
        for(k=1; k<=3; k++)
        {
            i = zajed->iVarijabla;
            printf("Citanje %d: zajednicka varijabla: %d\n",k, i);
            usleep(30000);
        }
        izadji_iz_kriticnog(id);
        // kraj kriticnog odsjecka
        n--;
    };
}

void Pisanje(int id)
{
    int i, k, n=5;
    while(n>0)
    {
        sleep(2);
        // Kriticni odsjecak
        udji_u_kriticni(id);
;        printf("Iteracija %d, upisivanje zajednicke varijable 3 puta:\n",n);
        for(k=1; k<=3; k++)
        {
            printf("Upisivanje %d: zajednicka varijabla: %d\n",k,i);
            zajed->iVarijabla = i++;
            usleep(30000);
        }
        izadji_iz_kriticnog(id);
        // Kraj kriticnog odsjecka
        n--;
    }
}

void Otpustanje()
{
    int parentid = zajed->parentId;
    if(getpid() != parentid)
    {
        printf("pid %d: Otpustam zajednicki spremnik.\n", getpid());
        shmdt(&zajed);
        exit(0);
    }
}

void signal_most(int sig)
{
    printf("pid %d: stigao je signal prekida %d.\n", getpid(), sig);
    Otpustanje();
}

int main(int argc, char **argv)
{
    pid_t id;
    int i;
    if(argc < 2)
    {
        printf("Koristenje: %s n\n", argv[0]);
        return 0;
    }
    Nprocesa = atoi(argv[1]);
    if(Nprocesa < 2) Nprocesa=2;
    else if(Nprocesa > 20) Nprocesa=20;
    //printf("Izabrano je %d procesa.\n",n);

    id_seg = shmget(IPC_PRIVATE, sizeof(zajednicka)+2*Nprocesa*sizeof(int), 0600);
    if(id_seg == -1)
        exit(1);

    zajed = (zajednicka*) shmat(id_seg, NULL, 0);
    zajed->fVarijabla = 0.0;
    zajed->iVarijabla = 0;
    zajed->parentId = getpid();
    for(i=0; i<2*Nprocesa; i++)
        zajed->polje[i]=0;
    signal(SIGINT, signal_most); // umjesto sigset, da se može po voji prekinuti i sve obrisati.

    for(i=0; i<Nprocesa; i++)
    {
        id = fork();
        if(id == 0) //dijete
        {
            if(i==0) //za pisanje
            {
                printf("Pokrenuto je dijete za pisanje %d.\n", getpid());
                Pisanje(i);
                exit(0);
            }
            else //dijete ili vise djece za citanje
            {
                printf("Pokrenuto je dijete za citanje %d.\n", getpid());
                Citanje(i);
                exit(0);
            } 
        } 
    }
    for(i=0; i<Nprocesa; i++)
    {
        id = wait(NULL);
        printf("Docekano je dijete %d.\n", id);
    }
    printf("pid %d: svi procesi - djeca su završili, završava i glavni program.\n", getpid());
    printf("Otpustam zajednicki spremnik.\n");
    shmdt(&zajed);
    printf("Brisem zajednicki spremnik.\n");
    shmctl(id_seg, IPC_RMID, NULL);
    return 0;
}