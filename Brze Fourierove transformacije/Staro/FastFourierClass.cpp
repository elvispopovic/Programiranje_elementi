#include <cmath>
#include "fastFourier.h"

Fft :: Fft(char smjer)
{
    broj_uzoraka=256;
    PromijeniSmjer(smjer);
    kosinusi = new double[broj_uzoraka];
    sinusi   = new double[broj_uzoraka];
    redoslijed = new unsigned char[broj_uzoraka];
    predracun();
}

Fft :: ~Fft()
{
    delete[] kosinusi;
    delete[] sinusi;
    delete[] redoslijed;
}

void Fft :: predracun()
{
    unsigned long i;
    double di;
    pi=4.0*atan(1.0);
    norma=1.0/(double)broj_uzoraka;
    for(i=0,di=0.0; i<256; i++,di+=1.0)
    {
        redoslijed[i]=(unsigned char)(((i * 0x0802LU & 0x22110LU)
                    |(i * 0x8020LU & 0x88440LU))
                    * 0x10101LU >> 16); //okretanje bitova
        kosinusi[i]=cos(2.0*pi*di/(1.0*broj_uzoraka));
        sinusi  [i]=sin(2.0*pi*di/(1.0*broj_uzoraka));
    }
}

void Fft :: PromijeniSmjer(char smjer)
{
    this->smjer=smjer>=0?1.0:-1.0;
}

void Fft :: Transformiraj(double rex[], double imx[], double ref[], double imf[])
{
    unsigned int broj_grupa;
    unsigned int sirina;
    unsigned int polusirina;
    unsigned int grupa, sirgrupa;
    unsigned int element, polusirelement;
    unsigned int trig_indeks;
    unsigned int i1, i2;
    double cosref, cosimf, sinref, sinimf;
    double reElement, imElement, reKopija, imKopija;
    broj_grupa=broj_uzoraka>>1;
    sirina=2;
    polusirina=1;
    for(grupa=0,sirgrupa=0, polusirelement=polusirina; grupa<broj_grupa;
        grupa++,sirgrupa+=sirina,polusirelement+=sirina)
    {
        i1=redoslijed[sirgrupa];
        i2=redoslijed[polusirelement];

        reElement=rex[i1]+rex[i2];
        imElement=imx[i1]+imx[i2];
        reKopija=rex[i1]-rex[i2];
        imKopija=imx[i1]-imx[i2];

        i1=redoslijed[i1];
        i2=redoslijed[i2];

        ref[i1]=reElement;
        imf[i1]=imElement;
        ref[i2]=reKopija;
        imf[i2]=imKopija;
    }
    sirina=4;
    polusirina=2;
    while(sirina<=broj_uzoraka)
    {
        broj_grupa>>=1;
        for(grupa=0, sirgrupa=0; grupa<broj_grupa; grupa++,sirgrupa+=sirina)
        {
            for(element=sirgrupa, trig_indeks=0, polusirelement=sirgrupa+polusirina;
                element<sirgrupa+polusirina; element++, trig_indeks+=broj_grupa, polusirelement++)
            {
                cosref = ref[polusirelement]*kosinusi[trig_indeks];
                sinref = smjer*ref[polusirelement]*  sinusi[trig_indeks];
                cosimf = imf[polusirelement]*kosinusi[trig_indeks];
                sinimf = smjer*imf[polusirelement]*  sinusi[trig_indeks];

                reElement = ref[element]+cosref-sinimf;
                imElement = imf[element]+cosimf+sinref;
                ref[polusirelement]  = ref[element]-cosref+sinimf;
                imf[polusirelement]  = imf[element]-cosimf-sinref;

                ref[element]=reElement;
                imf[element]=imElement;
            }
        }
        sirina <<=1;
        polusirina <<=1;
    }
}

void Fft :: Normiraj(double rex[], double imx[])
{
    unsigned int i;
    for(i=0; i<broj_uzoraka; i++)
    {
        rex[i]*=norma;
        imx[i]*=norma;
    }
}

