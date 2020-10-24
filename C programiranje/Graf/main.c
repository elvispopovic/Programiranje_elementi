#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* struktura koja se koristi za definiranje usmjerenog grafa */
typedef struct Vrh
{
    char oznaka[20];
    unsigned int outStupanj;
    unsigned int inStupanj;
    struct Vrh **susjedi;
    float *daljine;

    //za dijksktru
    struct Vrh *prethodni;
    float daljina;
    char trajna;

    //za topolosko sortiranje
    unsigned int inS; //privremeni in stupanj
    char prebaceno;
} vrh;

/* ova se struktura koristi samo za format ulaznih podataka */
typedef struct Luk
{
    unsigned int pocetni, zavrsni;
    float daljina;
} luk;


typedef struct Stog
{
    vrh **elementi, **pptPokazivac, **dno;
} stog;

unsigned int brojVrhova(const luk* lukovi, unsigned int nLukova)
{
    unsigned int i, max;
    if (nLukova<1)
        return 0;
    for(i=0, max=0; i<nLukova; i++)
    {
        if(lukovi[i].pocetni > max)
            max = lukovi[i].pocetni;
        if(lukovi[i].zavrsni > max)
            max = lukovi[i].zavrsni;
    }

    return max;
}

vrh** pripremiGraf(const luk* lukovi, unsigned int nLukova, unsigned int nVrhova)
{
    unsigned int i, stupanj;
    unsigned int *trenutniSusjedi, *ptTrenutniSusjedi;
    vrh **graf, **pptGraf;
    if (nVrhova<1 || nLukova<1)
        return NULL;
    graf = malloc(nVrhova*sizeof(vrh*));
    trenutniSusjedi = malloc(nVrhova*sizeof(unsigned int));
    /* inicijalizacija */
    for(i=0, pptGraf=graf, ptTrenutniSusjedi = trenutniSusjedi; i<nVrhova; i++, pptGraf++, ptTrenutniSusjedi++)
    {
        *ptTrenutniSusjedi = 0;
        *pptGraf = malloc(sizeof(vrh));
        (*pptGraf)->inStupanj = 0;
        (*pptGraf)->outStupanj = 0;
        (*pptGraf)->susjedi = NULL;
        (*pptGraf)->daljine = NULL;
        if(i<9)
            sprintf((*pptGraf)->oznaka, "V_0%u",i+1);
        else
            sprintf((*pptGraf)->oznaka, "V_%u",i+1);
    }
    /* prebrojavanje susjeda */
    for(i=0; i<nLukova; i++)
    {
        (graf[lukovi[i].pocetni-1]->outStupanj)++;
        (graf[lukovi[i].zavrsni-1]->inStupanj)++;
    }
    /* inicijalizacija polja susjeda */
    for(i=0, pptGraf=graf; i<nVrhova; i++, pptGraf++)
    {
        stupanj = (*pptGraf)->outStupanj;
        if(stupanj>0)
        {
            (*pptGraf)->susjedi = malloc(stupanj*sizeof(vrh*));
            (*pptGraf)->daljine = malloc(stupanj*sizeof(float));
        }
    }
    for(i=0; i<nLukova; i++)
    {
        graf[lukovi[i].pocetni-1]->susjedi[trenutniSusjedi[lukovi[i].pocetni-1]] = graf[lukovi[i].zavrsni-1];
        graf[lukovi[i].pocetni-1]->daljine[trenutniSusjedi[lukovi[i].pocetni-1]++] = lukovi[i].daljina;
    }
    free(trenutniSusjedi);
    return graf;
};

void ispisiGraf(vrh** graf, unsigned int nVrhova)
{
    unsigned int i, j;
    vrh** pptGraf, **pptSusjed;
    float* ptDaljina;
    if(nVrhova<1)
        return;
    for(i=0, pptGraf=graf; i<nVrhova; i++, pptGraf++)
    {
        printf("Vrh %s, out stupanj: %u, in stupanj: %u\n", (*pptGraf)->oznaka, (*pptGraf)->outStupanj, (*pptGraf)->inStupanj);
        if((*pptGraf)->outStupanj > 0)
            printf("\tSusjedi:\n");
        for(j=0, pptSusjed = (*pptGraf)->susjedi, ptDaljina = (*pptGraf)->daljine; j<(*pptGraf)->outStupanj; j++, pptSusjed++, ptDaljina++)
            printf("\t%s, d=%f\n",(*pptSusjed)->oznaka, *ptDaljina);
    }
}

void ispisiPut(vrh** put)
{
    vrh** pptPut;
    if(put == NULL)
    {
        printf("Nije pronadjen put izmedju ta dva vrha.\n");
        return;
    }
    pptPut = put;
    while((*pptPut)!=0)
        printf("%s, ",(*pptPut++)->oznaka);
    printf(" duljina: %f.\n",(*(pptPut-1))->daljina);
}

void ispisiListu(vrh** lista)
{
    vrh** pptLista;
    pptLista = lista;
    if(lista == NULL)
    {
        printf("Graf nije aciklican. Ne moze se topoloski sortirati.\n");
        return;
    }
    else
    {
        printf("Topolosko sortiranje vrhova:\n");
        while(*pptLista != NULL)
            printf("%s ",(*pptLista++)->oznaka);
        printf("\n");
    }

}

void obrisiGraf(vrh** graf, unsigned int nVrhova)
{
    unsigned int i;
    vrh** pptGraf;
    if(nVrhova<1)
        return;
    for(i=0, pptGraf=graf; i<nVrhova; i++, pptGraf++)
    {
        if((*pptGraf)->susjedi != NULL)
            free((*pptGraf)->susjedi);
        if((*pptGraf)->daljine != NULL)
            free((*pptGraf)->daljine);
        free(*pptGraf);
    }
    free(graf);
}

stog* kreirajStog(unsigned int nVrhova)
{
    stog* s;
    if(nVrhova<=0)
        return NULL;
    s = malloc(sizeof(stog));
    s->elementi = malloc(nVrhova*sizeof(vrh*));
    s->dno = (s->elementi)+(nVrhova-1);
    s->pptPokazivac = s->dno;
    return s;
}

void obrisiStog(stog* stog)
{
    free(stog->elementi);
    free(stog);
}

vrh** dijkstra(vrh** graf, stog* s, unsigned int nVrhova, vrh* pocetni, vrh* zavrsni)
{
    unsigned int i;
    vrh **put, **pptPut, **pptVrh;
    vrh *zadnji;
    float d, min;
    char izlaz = 0;

    for(i=0, pptVrh=graf; i< nVrhova; i++, pptVrh++)
    {
        (*pptVrh)->prethodni = NULL;
        if(*pptVrh == pocetni)
        {
            (*pptVrh)->daljina = 0.0;
            (*pptVrh)->trajna = 1;
        }
        else
        {
            (*pptVrh)->daljina = 1000000.0;
            (*pptVrh)->trajna = 0;
        }
    }
    zadnji = pocetni;
    do
    {
        if(zadnji->susjedi == NULL)
            break;
        for(i=0, pptVrh = zadnji->susjedi; i<zadnji->outStupanj; i++, pptVrh++)
        {
            if((*pptVrh)->trajna == 0)
            {
                d = zadnji->daljina + (zadnji->daljine)[i];
                if(d < (*pptVrh)->daljina)
                {
                    (*pptVrh)->prethodni = zadnji;
                    (*pptVrh)->daljina = d;
                }
            }
        }
        for(i=0, izlaz = 1, min = 1000000.0, pptVrh = graf; i<nVrhova; i++, pptVrh++)
        {
            if((*pptVrh)->trajna==0 && (*pptVrh)->daljina < min)
            {
                min = (*pptVrh)->daljina;
                zadnji = *(pptVrh);
                izlaz = 0;
            }
        }
        if(izlaz == 0)
        {
            zadnji->trajna = 1;
        }
    }while (izlaz == 0 && zadnji != zavrsni);


    zadnji = zavrsni;
    s->pptPokazivac = s->dno;
    while(zadnji != NULL && zadnji != pocetni)
    {
        *(s->pptPokazivac)-- = zadnji;
        zadnji = zadnji->prethodni;
    }
    if(zadnji == pocetni)
        *(s->pptPokazivac)-- = zadnji;
    else
        return NULL;



    put = malloc((nVrhova+1)*sizeof(vrh*));
    pptPut = put;
    (s->pptPokazivac)++;
    while(s->pptPokazivac <= s->dno)
    {
        *pptPut = *(s->pptPokazivac)++;
        pptPut++;
    }
    *pptPut = NULL; //zadnji mora biti NULL da bude granica
    return put;
}

vrh** topoloskoSortiranje(vrh** graf, unsigned int nVrhova)
{
    vrh **pptGraf, **pptSusjedi, **lista, **pptLista;
    unsigned int i, j;
    bool prebaceno;

    lista = malloc((nVrhova+1)*sizeof(vrh*));
    for(i=0, pptLista = lista; i<(nVrhova+1); i++, pptLista++)
        *pptLista = NULL;
    for(i=0, pptGraf = graf; i<nVrhova; i++, pptGraf++)
    {
        (*pptGraf)->inS = (*pptGraf)->inStupanj;
        (*pptGraf)->prebaceno = 0;
    }
    pptLista = lista;

    do
    {
        prebaceno = false;
        for(i=0, pptGraf = graf; i<nVrhova; i++, pptGraf++)
        {
            if((*pptGraf)->prebaceno == 0 && (*pptGraf)->inS == 0)
            {
                for(j=0, pptSusjedi = (*pptGraf)->susjedi; j<(*pptGraf)->outStupanj; j++, pptSusjedi++)
                    if((*pptSusjedi)->prebaceno == 0 && (*pptSusjedi)->inS>0)
                        (*pptSusjedi)->inS --;
                (*pptGraf)->prebaceno = 1;
                *pptLista++ = (*pptGraf);
                prebaceno = true;
            }
        }
    }while(prebaceno == true);

    for(i=0, pptGraf = graf; i<nVrhova; i++, pptGraf++)
        if((*pptGraf)->inS > 0)
        {
            free(lista);
            lista = NULL;
            return lista;
        }
    return lista;
}

int main()
{
    const luk lukovi1[] = {
        {1,2,1.0},{1,5,4.0},{2,3,1.0},{2,5,3.0},{3,5,1.0},{5,3,1.0},{3,4,5.0},{4,3,1.0},{5,4,1.0}
    };
    const luk lukovi2[] = {
        {1,2,1.0},{2,3,4.0},{3,4,2.0},{4,5,5.0},{1,5,3.0},
        {6,1,5.0},{8,2,3.0},{10,3,2.0},{4,12,2.0},{5,14,1.0},
        {7,6,2.0},{8,7,4.0},{9,8,1.0},{10,9,5.0},{11,10,2.0},
        {11,12,3.0},{12,13,4.0},{14,13,3.0},{14,15,4.0},{6,15,2.0},
        {7,17,1.0},{9,18,3.0},{19,11,4.0},{13,20,2.0},{15,16,5.0},
        {16,17,3.0},{17,18,5.0},{18,19,1.0},{20,19,5.0},{20,16,1.0},{21,22,0.5}
     };
     const luk lukovi3[] = {
         {8,5,1.0},{8,7,1.0},{8,9,1.0},{2,5,1.0},{2,7,1.0},{3,1,1.0},{3,2,1.0},{3,5,1.0},{4,3,1.0},
         {6,4,1.0},{6,5,1.0},{7,1,1.0},{7,4,1.0},{9,1,1.0},{9,4,1.9},{9,6,1.0}
     };
     const luk lukovi4[] = {
        {1,2,1.0},{1,3,1.0},{3,4,1.0},{2,3,1.0},{2,5,1.0},{5,4,1.0}
     };


    unsigned int nLukova, nVrhova;
    vrh** graf, **put, **lista;
    stog* s;
    int a, b;

    srand(time(0));

    nLukova = sizeof(lukovi1)/sizeof(luk);
    nVrhova = brojVrhova(lukovi1, nLukova);
    graf = pripremiGraf(lukovi1, nLukova, nVrhova);
    printf("Graf 1 je pripremljen.\n");
    ispisiGraf(graf, nVrhova);

    s = kreirajStog(nVrhova);
    for(int i=0; i<3; i++)
    {

        a = rand()%5;
        b = rand()%5;
        put = dijkstra(graf, s, nVrhova, graf[a],graf[b]);
        printf("Ispisujem najkraci put izmedju %d i %d.\n", a+1, b+1);
        ispisiPut(put);
        free(put);
    }
    obrisiStog(s);
    obrisiGraf(graf,nVrhova);


/*
    printf("==========================\n");
    nLukova = sizeof(lukovi2)/sizeof(luk);
    nVrhova = brojVrhova(lukovi2, nLukova);
    graf = pripremiGraf(lukovi2, nLukova, nVrhova);
    printf("Graf 2 je pripremljen.\n");
    ispisiGraf(graf, nVrhova);
    s = kreirajStog(nVrhova);

    for(int i=0; i<3; i++)
    {

        a = rand()%22;
        b = rand()%22;
        put = dijkstra(graf, s, nVrhova, graf[a],graf[b]);
        printf("Ispisujem najkraci put izmedju %d i %d.\n", a+1, b+1);
        ispisiPut(put);
        free(put);
    }

    obrisiStog(s);
    obrisiGraf(graf,nVrhova);
*/


    nLukova = sizeof(lukovi3)/sizeof(luk);
    nVrhova = brojVrhova(lukovi3, nLukova);
    graf = pripremiGraf(lukovi3, nLukova, nVrhova);
    printf("Graf 3 je pripremljen.\n");
    ispisiGraf(graf, nVrhova);
    lista = topoloskoSortiranje(graf, nVrhova);
    ispisiListu(lista);
    if(lista != NULL)
        free(lista);
    obrisiGraf(graf,nVrhova);

    nLukova = sizeof(lukovi4)/sizeof(luk);
    nVrhova = brojVrhova(lukovi4, nLukova);
    graf = pripremiGraf(lukovi4, nLukova, nVrhova);
    printf("Graf 4 je pripremljen.\n");
    ispisiGraf(graf, nVrhova);
    lista = topoloskoSortiranje(graf, nVrhova);
    ispisiListu(lista);
    if(lista != NULL)
        free(lista);
    obrisiGraf(graf,nVrhova);
    return 0;
}
