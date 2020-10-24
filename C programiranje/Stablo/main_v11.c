#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Cvor
{
    char oznaka[20];
    unsigned int brojDjece;
    struct Cvor **djeca;
} cvor;

typedef struct Stog
{
    cvor **elementi, **pptPokazivac, **dno;
} stog;

cvor** kreirajStablo(const unsigned int* veze, unsigned int n)
{
    cvor **cvorovi, **pptCvor;
    unsigned int *trenutnaDjeca, *ptTrenutnaDjeca;
    unsigned int i;
    /* odredjuje se broj cvorova */
    if(n<1)
        return NULL;

    /* inicijalizacija polja cvorova */
    cvorovi = malloc(n*sizeof(cvor*));
    /* kreiraju se cvorovi i postavljaju elementi struktura na pocetne vrijednosti */
    for(i=0, pptCvor = cvorovi; i<n; i++, pptCvor++)
    {
        *pptCvor = malloc(sizeof(cvor));
        if(i<9)
            sprintf((*pptCvor)->oznaka, "N_0%d",i+1);
        else
            sprintf((*pptCvor)->oznaka, "N_%d",i+1);
        (*pptCvor)->brojDjece=0;
        (*pptCvor)->djeca=NULL;
    }
    /* postavlja se broj djece za cvorove koji ih imaju */
    for(i=0; i<n-1; i++)
        cvorovi[veze[i]]->brojDjece++;

    /* pomocno polje koje cuva polozaje u poljima djece svih cvorova */
    trenutnaDjeca = malloc(n*sizeof(unsigned int));
    /* postavljaju se svi clanovi pomocnog polja na nulu i alociraju se polja djece u cvorovima */
    for(i=0, pptCvor = cvorovi, ptTrenutnaDjeca = trenutnaDjeca; i<n; i++, pptCvor++, ptTrenutnaDjeca++)
    {
        (*ptTrenutnaDjeca) = 0;
        if((*pptCvor)->brojDjece >0)
            (*pptCvor)->djeca = malloc(((*pptCvor)->brojDjece)*sizeof(cvor*));
    }
    /* pune se polja djece cvorova */
    for(i=0, pptCvor = cvorovi+1; i<n-1; i++, pptCvor++)
        if(cvorovi[veze[i]]->djeca != NULL)
            (cvorovi[veze[i]]->djeca)[trenutnaDjeca[veze[i]]++]=*pptCvor;

    /* dealocira se pomocno polje */
    free(trenutnaDjeca);
    return cvorovi;
};

void obrisiStablo(cvor** stablo, unsigned int n)
{
    int i;
    cvor **pptStablo;
    if(stablo==NULL)
        return;
    /* dealociraju se djeca u cvorovima, cvorovi i na kraju polje cvorova */
    for(i=0, pptStablo = stablo; i<n; i++, pptStablo++)
    {
        if(((*pptStablo)->djeca)!=NULL)
            free((*pptStablo)->djeca);
        free((*pptStablo));
    }
    free(stablo);
}

stog* kreirajStog(unsigned int n)
{
    stog* stog;
    if(n<=0)
        return NULL;
    stog = malloc(sizeof(stog));
    stog->elementi = malloc(n*sizeof(cvor*));
    stog->dno = (stog->elementi)+n-1;
    stog->pptPokazivac = stog->dno;
    return stog;
}

void obrisiStog(stog* stog)
{
    free(stog->elementi);
    free(stog);
}

void ispisiCvorove(cvor** stablo, unsigned int n, bool ispisiDjecu)
{
    cvor **pptStablo, **pptDjeca;
    int i, j, bd;
    if(stablo==NULL)
        return;
    for(i=0, pptStablo = stablo; i<n; i++, pptStablo++)
    {
        printf("Cvor %s",(*pptStablo)->oznaka);
        if(ispisiDjecu)
        {
            bd = (*pptStablo)->brojDjece;
            printf(", broj djece: %d\n", bd );
            for(j=0, pptDjeca=(*pptStablo)->djeca; j<bd; j++, pptDjeca++)
                printf("\tDijete %d: %s \n", j+1, (*pptDjeca)->oznaka );
        }
        else
            printf("\n");
    }
    for(i=0; i<24; i++)
        printf("-");
    printf("\n");
}

cvor** obilazenjePoSirini(cvor** stablo, unsigned int n)
{
    unsigned int i, j, brojDjece;
    cvor **red, **pptRed1, **pptRed2, **pptDjeca;
    red = malloc(n*sizeof(cvor*));
    pptRed1=red;
    pptRed2=red+1;
    *pptRed1 = *stablo;
    for(i=0; i<n-1; i++, pptRed1++)
    {
        brojDjece = (*pptRed1)->brojDjece;
        for(j=0, pptDjeca=(*pptRed1)->djeca; j<brojDjece; j++, pptRed2++, pptDjeca++)
            *pptRed2 = *pptDjeca;
    }
    return red;
}

cvor** obilazenjePoDubiniPreorder(cvor** stablo, stog* stog, unsigned int n, char smjer)
{
    int i;
    cvor **pptDjeca;
    cvor **rezultat, **pptRezultat;
    cvor *trenutni;
    rezultat = malloc(n*sizeof(cvor*));
    pptRezultat = rezultat;
    stog->pptPokazivac = stog->dno;
    *(stog->pptPokazivac)-- = *stablo;
    while(stog->pptPokazivac < stog->dno)
    {
        *pptRezultat++ = trenutni = *(((stog->pptPokazivac)++)+1);
        if(smjer<0)
            for(i=0, pptDjeca=trenutni->djeca; i<trenutni->brojDjece; i++,pptDjeca++)
                *(stog->pptPokazivac)-- = *pptDjeca;
        else
            for(i=0, pptDjeca=(trenutni->djeca)+trenutni->brojDjece-1; i<trenutni->brojDjece; i++,pptDjeca--)
                *(stog->pptPokazivac)-- = *pptDjeca;
    }
    return rezultat;
}

/* postorder je kao i preorder, samo se ide suprotnim smjerom, a rezultat se popunjava naopako */
cvor** obilazenjePoDubiniPostorder(cvor** stablo, stog* stog, unsigned int n, char smjer)
{
    int i;
    cvor **pptDjeca;
    cvor **rezultat, **pptRezultat;
    cvor *trenutni;
    rezultat = malloc(n*sizeof(cvor*));
    stog->pptPokazivac = stog->dno;
    pptRezultat = rezultat+n-1;
    *(stog->pptPokazivac)-- = *stablo;
    while(stog->pptPokazivac < stog->dno)
    {
        *pptRezultat-- = trenutni = *(((stog->pptPokazivac)++)+1);
        if(smjer>=0)
            for(i=0, pptDjeca=trenutni->djeca; i<trenutni->brojDjece; i++,pptDjeca++)
                *(stog->pptPokazivac)-- = *pptDjeca;
        else
            for(i=0, pptDjeca=(trenutni->djeca)+trenutni->brojDjece-1; i<trenutni->brojDjece; i++,pptDjeca--)
                *(stog->pptPokazivac)-- = *pptDjeca;
    }
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
    cvor **stablo, **rezultat;
    stog* stog;
    n = sizeof(veze)/sizeof(unsigned int)+1;
    stablo = kreirajStablo(veze, n);
    printf("Ispis cvorova:\n");
    ispisiCvorove(stablo, n, true);
    rezultat = obilazenjePoSirini(stablo,n);
    printf("Obilazenje po sirini:\n");
    ispisiCvorove(rezultat,n, false);
    /* odgovornost pozivatelja da dealocira polje pretrazenih cvorova */
    free(rezultat);
    stog = kreirajStog(n);

    printf("Obilazenje preorder:\n");
    rezultat = obilazenjePoDubiniPreorder(stablo, stog, n, 1);
    ispisiCvorove(rezultat,n, false);
    free(rezultat);
    printf("Obilazenje postorder:\n");
    rezultat = obilazenjePoDubiniPostorder(stablo, stog, n, 1);
    ispisiCvorove(rezultat,n, false);
    free(rezultat);

    obrisiStog(stog);
    obrisiStablo(stablo, n);


    return 0;
}
