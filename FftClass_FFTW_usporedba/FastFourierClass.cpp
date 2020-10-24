#include <iostream>
#include <cmath>
#include <cstring>
#include "fastFourier.h"


using namespace std;

Fft :: Fft(unsigned char bitova, char smjer)
{
    /* za manje od 2 podatka (1 bita koji ih broji)  neke transformacije nisu definirane */
    if(bitova<1)
        bitova=1;
    broj_uzoraka=(1<<bitova);
    broj_bitova = bitova;
    /* ulazni i izlazni podaci (realni i imaginarni) */
    inBufferRe = new double[broj_uzoraka<<1];
    inBufferIm = new double[broj_uzoraka<<1];
    outBufferRe = new double[broj_uzoraka<<1];
    outBufferIm = new double[broj_uzoraka<<1];


    PromijeniSmjer(smjer);
    /* razni prethodno izracunati sinusi i kosinusi (bazne funkcije) */
    fftKosinusi = new double[broj_uzoraka];
    fftSinusi   = new double[broj_uzoraka];
    dctKosinusi = new double[broj_uzoraka<<1];
    dctSinusi= new double[broj_uzoraka<<1];
    /* fazna rampa za dct pripremu podataka poslije dft i prije idft */
    rampKosinusi= new double[broj_uzoraka];
    rampSinusi  = new double[broj_uzoraka];
    cos_min_sin = new double[broj_uzoraka];
    cos_plu_sin = new double[broj_uzoraka];

    /* redoslijed ulaznih podataka u dft kako ne bi bilo potrebe za rekurzijama */
    fftRedoslijed = new unsigned int[broj_uzoraka];
    dctRedoslijed = new unsigned int[broj_uzoraka<<1];
    predracun();
}

Fft :: ~Fft()
{
    delete[] inBufferRe;
    delete[] inBufferIm;
    delete[] outBufferRe;
    delete[] outBufferIm;
    delete[] fftKosinusi;
    delete[] fftSinusi;
    delete[] dctKosinusi;
    delete[] dctSinusi;
    delete[] rampKosinusi;
    delete[] rampSinusi;
    delete[] cos_min_sin;
    delete[] cos_plu_sin;
    delete[] fftRedoslijed;
    delete[] dctRedoslijed;
}
/* Metoda koja unaprijed izracunava potrebne podatke kako se ne bi racunali u toku tranformacija */
void Fft :: predracun()
{
    unsigned int i;
    double di;
    pi=4.0*atan(1.0); //moze i M_PI iz cmath
    norma=1.0/(double)broj_uzoraka;

    /* izracunavaju se razne skale baznih funkcija za dft i dct */
    for(i=0,di=0.0; i<broj_uzoraka; i++,di+=1.0)
    {
        fftRedoslijed[i] =zrcaljenjeBitova(i, broj_bitova); //okretanje bitova
        fftKosinusi[i]   =cos(2.0*pi*di/(1.0*broj_uzoraka));
        fftSinusi  [i]   =sin(2.0*pi*di/(1.0*broj_uzoraka));
        rampKosinusi[i]  =cos(pi*di/(2.0*broj_uzoraka));
        rampSinusi[i]    =sin(pi*di/(2.0*broj_uzoraka));
        cos_min_sin[i]   =cos(pi/(2.0*(broj_uzoraka))*di)-sin(pi/(2.0*(broj_uzoraka))*di);
        cos_plu_sin[i]   =cos(pi/(2.0*(broj_uzoraka))*di)+sin(pi/(2.0*(broj_uzoraka))*di);
    }
    /* redoslijedi se rješavaju okretanjem bitova LSb <-> MSb */
    for(i=0,di=0.0; i<broj_uzoraka<<1; i++,di+=1.0)
    {
        dctRedoslijed[i]=zrcaljenjeBitova(i, broj_bitova+1);
        dctKosinusi[i]=cos(pi/(1.0*broj_uzoraka)*di);
        dctSinusi[i]=sin(pi/(1.0*broj_uzoraka)*di);
    }
}
/* Metoda koja okrece potreban broj bitova u npr 11001 postaje 10011
   Ovo je tzv. FFT scramble i time se izbjegava potreba za rekurzijama
*/
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
/* Smjer transformacije odredjen je varijablom smjer koja se ovdje moze postaviti */
void Fft :: PromijeniSmjer(char smjer)
{
    this->smjer=smjer>=0?1.0:-1.0;
}
/* Javna metoda koja poziva obavlja diskretne furierove transformacije
   Oslanja se na privatnu metodu fft koja se koristi i za dft i za dct
*/
void Fft :: TransDFT(double rex[], double imx[], double ref[], double imf[])
{
    fft(rex,imx,ref,imf,fftSinusi,fftKosinusi,fftRedoslijed, broj_uzoraka);
}
/* Javna metoda koja obavlja diskretne sinusne transformacije
   Oslanja se na privatnu metodu fft koja se koristi i za dft i za dst
   Potrebni su samo realni parametri (signal) i rezultat je realni spektar i obrnuto
*/
void Fft :: TransDST(double x[], double f[])
{
   int i, j;
    if(smjer==1)
        for(i=0, j=broj_uzoraka-1; i<broj_uzoraka; i++,j--)
        {   //pripremamo ulazne podatke za dst
            inBufferRe[i]=inBufferIm[i]=x[i];
            inBufferRe[i+broj_uzoraka]=inBufferIm[i+broj_uzoraka]=-x[j];
        }
    else
    {
        //pripremamo ulazne podatke za idst
        inBufferRe[0]=inBufferIm[0]=0.0;
        inBufferRe[broj_uzoraka]=inBufferIm[broj_uzoraka]=x[broj_uzoraka-1];
        for(i=1, j=(broj_uzoraka<<1)-1; i<broj_uzoraka; i++, j--)
        {
            //mnozenje sa inverznom faznom rampom da se dobiju podaci za idct
            inBufferRe[i]=inBufferIm[j]=x[i-1]*cos_plu_sin[i];
            inBufferIm[i]=inBufferRe[j]=-x[i-1]*cos_min_sin[i];
        }
    }
    fft(inBufferRe,inBufferIm,outBufferRe,outBufferIm,dctSinusi,dctKosinusi,dctRedoslijed,broj_uzoraka<<1);
    if(smjer==1)
    {
        f[broj_uzoraka-1]=outBufferRe[broj_uzoraka]*rampKosinusi[0]+outBufferIm[broj_uzoraka]*rampSinusi[0];
        for(i=0; i<broj_uzoraka-1; i++) //mnozenje sa faznom rampom da se dobije izlaz
            f[i]=outBufferRe[i+1]*rampKosinusi[(i+1)]+outBufferIm[i+1]*rampSinusi[(i+1)];
    }
    else
        for(i=0; i<broj_uzoraka; i++) //uzimamo samo prve potrebne elemente
            f[i]=outBufferRe[i];
}

/* Javna metoda koja obavlja diskretne kosinusne transformacije
   Oslanja se na privatnu metodu fft koja se koristi i za dft i za dct
   Potrebni su samo realni parametri (signal) i rezultat je realni spektar i obrnuto
*/
void Fft :: TransDCT(double x[], double f[])
{
    int i, j;
    if(smjer==1)
        for(i=0, j=broj_uzoraka-1; i<broj_uzoraka; i++,j--)
        {   //pripremamo ulazne podatke za dct
            inBufferRe[i]=inBufferIm[i]=x[i];
            inBufferRe[i+broj_uzoraka]=inBufferIm[i+broj_uzoraka]=x[j];
        }
    else
    {
        //pripremamo ulazne podatke za idct
        inBufferRe[0]=inBufferIm[0]=x[0];
        inBufferRe[broj_uzoraka]=inBufferIm[broj_uzoraka]=0.0;
        for(i=1, j=(broj_uzoraka<<1)-1; i<broj_uzoraka; i++, j--)
        {
            //mnozenje sa inverznom faznom rampom da se dobiju podaci za idct
            inBufferRe[i]=inBufferIm[j]=x[i]*cos_min_sin[i];
            inBufferIm[i]=inBufferRe[j]=x[i]*cos_plu_sin[i];
        }
    }
    //izvodjenje diskretnih fourierovih transformacija (ovisno o smjeru dft ili idft
    fft(inBufferRe,inBufferIm,outBufferRe,outBufferIm,dctSinusi,dctKosinusi,dctRedoslijed,broj_uzoraka<<1);

    if(smjer==1)
        for(i=0; i<broj_uzoraka; i++) //mnozenje sa faznom rampom da se dobije izlaz
            f[i]=outBufferRe[i]*rampKosinusi[i]+outBufferIm[i]*rampSinusi[i];
    else
        for(i=0; i<broj_uzoraka; i++) //uzimamo samo prve potrebne elemente
            f[i]=outBufferRe[i];


}
/* Metoda za normiranje podataka za dft, ukoliko postoji za tim potreba */
void Fft :: Normiraj(double rex[], double imx[])
{
    unsigned int i;
    for(i=0; i<broj_uzoraka; i++)
    {
        rex[i]*=norma;
        imx[i]*=norma;
    }
}
/* Jezgra svega: ova metoda zapravo izvodi diskretne fourierove transformacije */
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
       ovo omogucava izlaz sa obrnutim redoslijedom kao u fftw biblioteci
       No prvi element ostaje jednak, a okrecu se samo ostali
       Ovdje smo okrenuli sve ali smo onda cirkularno pomakli polje za jedan element
    */
    /* Prva faza: najmanje grupe od po dva elementa */
    temp=uzoraka-1;
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
    /* glavna faza postupno sirenje grupa za duplo te njihovo smanjivanje dok jedna grupa ne obuhvati cijeli uzorak */
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

