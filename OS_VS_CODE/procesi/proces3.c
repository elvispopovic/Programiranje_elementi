#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int id_seg;
int *ZajedVar;

void Citanje()
{
    int i;
    do
    {
        i = *ZajedVar;
        printf("Zajednicka varijabla: %d\n", i);
        sleep(1);
    } while (i == 0);
    printf("Procitano je %d.\n", i);
}



void Brisanje(int sig)
{
    shmdt(ZajedVar);
    shmctl(id_seg, IPC_RMID, NULL);
    exit(0);
}

void signal_most(int sig)
{
    printf("Stigao je signal prekida %d.\n", sig);
    Brisanje(sig);
}

int main(int argc, char **argv)
{
    pid_t id;
    id_seg = shmget(IPC_PRIVATE, sizeof(int)*10, 0600);
    if(id_seg == -1)
        exit(1);

    ZajedVar = (int*) shmat(id_seg, NULL, 0);
    *ZajedVar = 0;
    signal(SIGINT, signal_most); // umjesto sigset da se može po voji prekinuti i sve obrisati.

    id = fork();
    if(id == 0) //dijete
    {
        printf("Pokrenuto je dijete za citanje %d.\n", getpid());
        Citanje();
        exit(0);
    }
    id = fork();
    if(id == 0) //drugo dijete
    {
        printf("Pokrenuto je dijete za pisanje %d.\n", getpid());
        sleep(5);
        *ZajedVar = 1;
        exit(0);
    }   
    id = wait(NULL);
    printf("Docekano je dijete %d.\n", id);
    id = wait(NULL);
    printf("Docekano je dijete %d.\n", id);
    Brisanje(0);

    return 0;
}