#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char **argv)
{
    pid_t id;
    int i;

    printf("Pokrenuto\n");
    id = fork();
    if(id == 0)
    {
        printf("Pokrenut je proces - dijeteta, id %d\n", getpid());
        for(i=1; i<=5; i++)
        {
            printf("Brojac djeteta: %d\n",i);
            sleep(1);
        }
        printf("Brojac djeteta: %d\n",i);
        printf("Zatvara se proces djeteta.\n");
        exit(0);
    }
    printf("Proces - roditelj (pid=%d) je dobio od djeteta id %d.\n", getpid(), id);
    printf("Ceka se zavrsavanje procesa djeteta.\n");
    id = wait(NULL);
    printf("Roditelj je docekao zavrsetak procesa djeteta %d.\n",id);
    printf("Zavrsava proces roditelja.\n");

    return 0;
}