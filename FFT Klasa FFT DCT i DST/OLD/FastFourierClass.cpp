#include <iostream>
#include <cmath>
#include <cstring>
#include "fastFourier.h"

/* Konstruktor - inicijalizacija i alokacija polja */
Fft :: Fft(unsigned char bitova, char smjer)
{
    if(bitova<2)
        bitova=2;
    broj_uzoraka=(1<<bitova);
    broj_bitova = bitova;
    //posebni 4-struki bufferi ulaznih i izlaznih podataka za DCT
    inBuffer = new double[broj_uzoraka<<2];
    outBufferRe = new double[broj_uzoraka<<2];
    outBufferIm = new double[broj_uzoraka<<2];

    PromijeniSmjer(smjer);
    fftKosinusi = new double[broj_uzoraka];
    fftSinusi   = new double[broj_uzoraka];
    //finiji kosinusi i sinusi za DCT kalkulacije
    dctKosinusi = new double[broj_uzoraka<<2];
    dctSinusi= new double[broj_uzoraka<<2];

    //redoslijedi (zrcalni bitosi) koji uklanjaju potrebu za rekurzijama
    fftRedoslijed = new unsigned int[broj_uzoraka];
    dctRedoslijed = new unsigned int[broj_uzoraka<<2];
    predracun();
}
/* Destruktor */
Fft :: ~Fft()
{
    delete[] inBuffer;
    delete[] outBufferRe;
    delete[] outBufferIm;
    delete[] fftKosinusi;
    delete[] fftSinusi;
    delete[] dctKosinusi;
    delete[] dctSinusi;
    delete[] fftRedoslijed;
    delete[] dctRedoslijed;
}

/* Prekalkulacija svih potrebnih elemenata za DCT i DFT */
void Fft :: predracun()
{
    unsigned int i;
    double di;
    pi=4.0*atan(1.0); //moze i M_PI iz cmath
    norma=1.0/(double)broj_uzoraka;

    for(i=0; i<broj_uzoraka<<2; i+=2)
        inBuffer[i]=0.0;

    for(i=0,di=0.0; i<broj_uzoraka; i++,di+=1.0)
    {
        fftRedoslijed[i] =zrcaljenjeBitova(i, broj_bitova); //okretanje bitova
        fftKosinusi[i]   =cos(2.0*pi*di/(1.0*broj_uzoraka));
        fftSinusi  [i]   =sin(2.0*pi*di/(1.0*broj_uzoraka));
    }
    for(i=0,di=0.0; i<broj_uzoraka<<2; i++,di+=1.0)
    {
        dctRedoslijed[i]=zrcaljenjeBitova(i, broj_bitova+2);
        dctKosinusi[i]=cos(2.0*pi/(4.0*broj_uzoraka)*i);
        dctSinusi[i]=sin(2.0*pi/(4.0*broj_uzoraka)*i);
    }
}
/* Zrcaljenje bitova - FFT scrambling */
unsigned int Fft :: zrcaljenjeBitova(unsigned int x, unsigned char bitova)
{
    unsigned int i;
    unsigned int maska,setter,rezultat;
    setter=1<<(bitova-1);
    for(i=0, rezultat=0, maska=1; i<bitova; i++, maska <<=1, setter>>=1)
        if(maska&x)
            rezultat|=setter;

    return rezultat;
}
/* Ova metoda namješta smjer fourierovih transformacija */
void Fft :: PromijeniSmjer(char smjer)
{
    this->smjer=smjer>=0?1.0:-1.0;
}

/* Diskretne fourierove transformacije (DFT) */
void Fft :: TransDFT(double rex[], double imx[], double ref[], double imf[])
{
    fft(rex,imx,ref,imf,fftSinusi,fftKosinusi,fftRedoslijed, broj_uzoraka);
}
/* Diskretne kosinusne transformacije tip II (DCT) i tip III (IDCT) */
void Fft :: TransDCT(double x[], double f[])
{
    unsigned int i;
    int j;
    //podesavanje ulaznih elemenata za tip II
    if(smjer==1.0)
        for(i=0,j=4*broj_uzoraka-1; i<broj_uzoraka; i++,j-=2)
        {
            inBuffer[(i<<1)+1]=x[i];
            inBuffer[j]=x[i];
        }
    else //podesavanje ulaznih elemenata za tip III (inverzno)
    {
        for(i=0; i<broj_uzoraka; i++)
            inBuffer[i]=x[i];
        inBuffer[i++]=0.0;
        for(j=broj_uzoraka-1; j>=0; i++, j--)
            inBuffer[i]=-x[j];
        for(j=1; j<(int)broj_uzoraka; j++,i++)
            inBuffer[i]=-x[j];
        inBuffer[i++]=0.0;
        for(j=broj_uzoraka-1; j>0; j--,i++)
            inBuffer[i]=x[j];
        inBuffer[i++]=0.0;
    }
    //poziva se glavna DFT metoda koja obavlja posao i za DFT i za DCT
    fft(inBuffer,inBuffer,outBufferRe,outBufferIm,dctSinusi,dctKosinusi,dctRedoslijed,broj_uzoraka<<2);
    //prilagodjavanje izlaznih rezultata
    if(smjer==1)
        for(i=0; i<broj_uzoraka; i++)
            f[i]=outBufferRe[i];
    else
        for(i=0; i<broj_uzoraka; i++)
            f[i]=0.5*outBufferRe[(i<<1)+1];
}
/* normiranje za FFT */
void Fft :: Normiraj(double rex[], double imx[])
{
    unsigned int i;
    for(i=0; i<broj_uzoraka; i++)
    {
        rex[i]*=norma;
        imx[i]*=norma;
    }
}
/* glavna metoda - diskretne fourierove transformacije obavljaju se ovdje */
void Fft :: fft(double rex[], double imx[], double ref[], double imf[], double sinusi[], double kosinusi[], unsigned int redoslijed[], unsigned int uzoraka)
{
    unsigned int broj_grupa;
    unsigned int sirina;
    unsigned int polusirina;
    unsigned int grupa, sirgrupa;
    unsigned int element, polusirelement;
    unsigned int trig_indeks;
    unsigned int i1, i2;
    double cosref, cosimf, sinref, sinimf, temp;
    double reElement, imElement, reKopija, imKopija;
    broj_grupa=uzoraka>>1;
    sirina=2;
    polusirina=1;

    /* fourierov transformat racunamo naopako, od zadnjeg prema prvom unutar svake grupe
       ovo omogucava izlaz sa obrnutim redoslijedom
       No prvi element ostaje jednak, a okrecu se samo ostali
       Ovdje smo okrenuli sve ali smo onda cirkularno pomakli polje za jedan element
       U inverznoj transformaciji na pocetku ce biti nula */
    temp=uzoraka-1;
    /*  prva faza - uzimanje parametara i izvodjenje transformacija za najmanje grupe od 2 elementa
        "najvise frekvencije" - sinusi i kosinusi imaju vrijednosti samo 1 i -1
    */
    for(grupa=0,sirgrupa=0, polusirelement=polusirina; grupa<broj_grupa;
        grupa++,sirgrupa+=sirina,polusirelement+=sirina)
    {
        i1=redoslijed[sirgrupa];
        i2=redoslijed[polusirelement];

        reElement=rex[i1]+rex[i2];
        imElement=imx[i1]+imx[i2];
        reKopija=rex[i1]-rex[i2];
        imKopija=imx[i1]-imx[i2];

        //okrecemo naopako
        i1=temp-redoslijed[i1];
        i2=temp-redoslijed[i2];
        ref[i1]=reElement;
        imf[i1]=imElement;
        ref[i2]=reKopija;
        imf[i2]=imKopija;
    }
    /* druga faza - postupno se produljuju grupe i smanjuje njihov broj
        sukladno tome, prilagodjeni su (skalirani) i sinusi i kosinusi tj. bazne funkcije
    */
    sirina=4;
    polusirina=2;
    while(sirina<=uzoraka)
    {
        broj_grupa>>=1;
        for(grupa=0, sirgrupa=0; grupa<broj_grupa; grupa++,sirgrupa+=sirina)
        {   //idemo u obrnutom smjeru unutar grupa
            for(element=sirgrupa+sirina-1, trig_indeks=0, polusirelement=sirgrupa+polusirina-1;
                element>=sirgrupa+polusirina; element--, trig_indeks+=broj_grupa, polusirelement--)
            {
                cosref = ref[polusirelement]*kosinusi[trig_indeks];
                sinref = smjer*ref[polusirelement]*  sinusi[trig_indeks];
                cosimf = imf[polusirelement]*kosinusi[trig_indeks];
                sinimf = smjer*imf[polusirelement]*  sinusi[trig_indeks];
                ref[polusirelement]  = ref[element]-cosref+sinimf;
                imf[polusirelement]  = imf[element]-cosimf-sinref;
                ref[element]+=cosref-sinimf;
                imf[element]+=cosimf+sinref;
            }
        }
        sirina <<=1;
        polusirina <<=1;
    }
    //pomicemo sve cirkularno udesno za jedan element
    temp=ref[uzoraka-1];
    for(element=uzoraka-1; element>0; element--)
        ref[element]=ref[element-1];
    ref[0]=temp;
    temp=imf[uzoraka-1];
    for(element=uzoraka-1; element>0; element--)
        imf[element]=imf[element-1];
    imf[0]=temp;

}

