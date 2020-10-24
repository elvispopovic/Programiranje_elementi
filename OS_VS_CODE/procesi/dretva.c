#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

int id_seg;
int ZajedVar;

void *Pisanje(void *x)
{
    ZajedVar = *((int *)x);
    return NULL;
}

void *Citanje(void *x)
{
    int i;
    do
    {
        i = ZajedVar;
        printf("Zajednicka varijabla: %d\n", i);
        sleep(1);
    } while (i == 0);
    printf("Procitano je %d.\n", i);
    return NULL;
}

int main(int argc, char **argv)
{
    int i;
    pthread_t dr_id[2];

    ZajedVar = 0;
    i= 123;

    if(pthread_create(&dr_id[0], NULL, Citanje, NULL) != 0)
    {
        printf("Greska prilikom kreiranja dretve.\n");
        exit(1);
    }
    sleep(5);
    if(pthread_create(&dr_id[1], NULL, Pisanje, &i) != 0)
    {
        printf("Greska prilikom kreiranja dretve.\n");
        exit(1);
    }
    pthread_join(dr_id[0], NULL);
    pthread_join(dr_id[1], NULL);

    return 0;
}