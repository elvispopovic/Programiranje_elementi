#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


typedef struct Cvor
{
    char oznaka[20];
    unsigned int brojDjece;
    struct Cvor *djeca;
} cvor;


cvor* kreirajStablo(const unsigned int* veze, unsigned int n)
{
    cvor *cvorovi;
    unsigned int *trenutnaDjeca;
    unsigned int i;
    /* odredjuje se broj cvorova */
    if(n<1)
        return NULL;

    /* inicijalizacija polja cvorova */
    cvorovi = malloc(n*sizeof(cvor));
    /* kreiraju se cvorovi i postavljaju elementi struktura na pocetne vrijednosti */
    for(i=0; i<n; i++)
    {
        if(i<9)
            sprintf(cvorovi[i].oznaka, "N_0%d",i+1);
        else
            sprintf(cvorovi[i].oznaka, "N_%d",i+1);
        cvorovi[i].brojDjece=0;
        cvorovi[i].djeca=NULL;
    }
    /* postavlja se broj djece za cvorove koji ih imaju */
    for(i=0; i<n-1; i++)
        cvorovi[veze[i]].brojDjece++;

    /* pomocno polje koje cuva polozaje u poljima djece svih cvorova */
    trenutnaDjeca = malloc(n*sizeof(unsigned int));
    /* postavljaju se svi clanovi pomocnog polja na nulu i alociraju se polja djece u cvorovima */
    for(i=0; i<n; i++)
    {
        trenutnaDjeca[i] = 0;
        if(cvorovi[i].brojDjece >0)
            cvorovi[i].djeca = malloc((cvorovi[i].brojDjece)*sizeof(cvor));
    }
    /* pune se polja djece cvorova */
    for(i=0; i<n-1; i++)
        if(cvorovi[veze[i]].djeca != NULL)
            memcpy(&(cvorovi[veze[i]].djeca)[trenutnaDjeca[veze[i]]++],&cvorovi[i+1], sizeof(cvor));

    /* dealocira se pomocno polje */
    free(trenutnaDjeca);
    return cvorovi;
};

void obrisiStablo(cvor* stablo, unsigned int n)
{
    int i;
    if(stablo==NULL)
        return;
    /* dealociraju se djeca u cvorovima, cvorovi i na kraju polje cvorova */
    for(i=0; i<n; i++)
    {
        if((stablo[i].djeca) != NULL)
            free(stablo[i].djeca);
    }
    free(stablo);
}

void ispisiCvorove(cvor* stablo, unsigned int n, bool ispisiDjecu)
{
    cvor *ptDjeca;
    int i, j, bd;
    if(stablo==NULL)
        return;
    for(i=0; i<n; i++)
    {
        printf("Cvor %s",stablo[i].oznaka);
        if(ispisiDjecu)
        {
            bd = stablo[i].brojDjece;
            printf(", broj djece: %d\n", bd );
            ptDjeca = stablo[i].djeca;
            for(j=0; j<bd; j++)
                printf("\tDijete %d: %s \n", j+1, ptDjeca[j].oznaka );
        }
        else
            printf("\n");
    }
    for(i=0; i<24; i++)
        printf("-");
    printf("\n");
}

cvor* obilazenjePoSirini(cvor* stablo, unsigned int n)
{
    unsigned int i, j, brojDjece;
    cvor *red, *ptRed1, *ptRed2, *ptDjeca;
    red = malloc(n*sizeof(cvor));
    ptRed1=red;
    ptRed2=red+1;
    memcpy(ptRed1, stablo, sizeof(cvor));
    for(i=0; i<n-1; i++, ptRed1++)
    {
        brojDjece = ptRed1->brojDjece;
        ptDjeca = ptRed1->djeca;
        for(j=0; j<brojDjece; j++, ptRed2++, ptDjeca++)
            memcpy(ptRed2, ptDjeca, sizeof(cvor));
    }
    return red;
}

cvor* obilazenjePoDubiniPreorder(cvor* stablo, unsigned int n, char smjer)
{
    int i;
    cvor **stog, **pptStog;
    cvor *ptDjeca;
    cvor *rezultat, *ptRezultat;
    cvor *trenutni;
    stog = malloc(n*sizeof(cvor*));
    rezultat = malloc(n*sizeof(cvor));
    pptStog = stog+n-1;
    ptRezultat = rezultat;
    *pptStog-- = &stablo[0];
    while(pptStog < (stog+n-1))
    {
        trenutni = *((pptStog++)+1);
        memcpy(ptRezultat++,trenutni,sizeof(cvor));
        if(smjer<0)
            for(i=0, ptDjeca=trenutni->djeca; i<trenutni->brojDjece; i++,ptDjeca++)
                *pptStog-- = ptDjeca;
        else
            for(i=0, ptDjeca=(trenutni->djeca)+trenutni->brojDjece-1; i<trenutni->brojDjece; i++,ptDjeca--)
                *pptStog-- = ptDjeca;
    }
    free(stog);
    return rezultat;
}


/* postorder je kao i preorder, samo se ide suprotnim smjerom, a rezultat se popunjava naopako */

cvor* obilazenjePoDubiniPostorder(cvor* stablo, unsigned int n, char smjer)
{
    int i;
    cvor **stog, **pptStog;
    cvor *ptDjeca;
    cvor *rezultat, *ptRezultat;
    cvor *trenutni;
    stog = malloc(n*sizeof(cvor*));
    rezultat = malloc(n*sizeof(cvor));
    pptStog = stog+n-1;
    ptRezultat = rezultat+n-1;
    *pptStog-- = &stablo[0];
    while(pptStog < (stog+n-1))
    {
        trenutni = *((pptStog++)+1);
        memcpy(ptRezultat--,trenutni,sizeof(cvor));
        if(smjer>=0)
            for(i=0, ptDjeca=trenutni->djeca; i<trenutni->brojDjece; i++,ptDjeca++)
                *pptStog-- = ptDjeca;
        else
            for(i=0, ptDjeca=(trenutni->djeca)+trenutni->brojDjece-1; i<trenutni->brojDjece; i++,ptDjeca--)
                *pptStog-- = ptDjeca;
    }
    free(stog);
    return rezultat;
}


int main()
{
    /* 23 cvora, polje pokazuje koji je cvor povezan sa daljnjim cvorovima, npr,
   na pocetku 0, 0, 0, je veza nultog cvora sa 1, 2 i 3. i dalje sve po redu
   polje zato povezuje zapisane svorove sa 1,2,3,... cvorom po redu
   ima 22 elementa, to su cvorovi 1,2,3,...22, a uz nulti to je ukupno 23 cvora*/
    const unsigned int veze[]={0,0,0,1,2,2,2,3,3,4,6,7,8,11,12,13,13,13,13,18,19,21,21,21};
    unsigned int n;
    cvor *stablo;
    cvor *rezultat;
    n = sizeof(veze)/sizeof(unsigned int)+1;
    stablo = kreirajStablo(veze, n);
    printf("Ispis cvorova:\n");
    ispisiCvorove(stablo, n, true);
    rezultat = obilazenjePoSirini(stablo,n);
    printf("Obilazenje po sirini:\n");
    ispisiCvorove(rezultat,n, false);

    /* odgovornost pozivatelja da dealocira polje pretrazenih cvorova */
    free(rezultat);
    printf("Obilazenje preorder:\n");
    rezultat = obilazenjePoDubiniPreorder(stablo, n, 1);
    ispisiCvorove(rezultat,n, false);
    free(rezultat);
    printf("Obilazenje postorder:\n");
    rezultat = obilazenjePoDubiniPostorder(stablo, n, 1);
    ispisiCvorove(rezultat,n, false);
    free(rezultat);
    obrisiStablo(stablo, n);


    return 0;
}
