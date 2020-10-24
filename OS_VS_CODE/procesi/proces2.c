#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char **argv)
{
    pid_t id;
    int i;

    printf("Pokrenuto 2\n");
    for(i=0; i<3; i++)
    {
        id = fork();
        switch(id)
        {
            case 0: printf("Pokrenut proces djeteta %d.\n", getpid()); exit(0); break;
            case -1: printf("Ne mogu pokrenuti proces.\n"); break;
            default: printf("Roditelj sa pid %d ceka.\n", getpid()); break;

        }
    }
    sleep(1);
    while(i--)
    {
        id = wait(NULL);
        printf("Docekan zavrsetak djeteta pid %d.\n", id);
    }
    printf("Zavrsava proces roditelja pid %d.\n", getpid());

    return 0;
}