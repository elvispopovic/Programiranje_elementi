#include <iostream>
#include <cmath>
#include <windows.h>
#include <complex.h>
#include "fastFourier.h"

using namespace std;

void ispisi(double[], double[], int, bool ispisIm=true);

int main()
{
    int i;
    int broj_bitova;
    int broj_uzoraka;
    char unos[80];

    double *reU;
    double *imU;
    double *reI;
    double *imI;

    broj_bitova=6;
    broj_uzoraka=(1<<broj_bitova);

    reU = new double[broj_uzoraka];
    imU = new double[broj_uzoraka];
    reI = new double[broj_uzoraka];
    imI = new double[broj_uzoraka];

    Fft *fft = new Fft(broj_bitova);
/*
//DFT test
   for(i=0; i<broj_uzoraka; i++)
    {
        //reU[i]=cos(2.0*M_PI/broj_uzoraka*i);
        reU[i]=1.0/broj_uzoraka*i;
        imU[i]=sin(2.0*M_PI/broj_uzoraka*i);
    }

    fft->TransDFT(reU,imU,reI,imI);
    ispisi(reI, imI,broj_bitova);

    fft->PromijeniSmjer(-1);
    fft->TransDFT(reI,imI,reU,imU);
    ispisi(reU, imU,broj_bitova);
*/

//DCT test
    for(i=0; i<broj_uzoraka; i++)
    {
        //reU[i]=cos(2.0*M_PI/broj_uzoraka*i);
        reU[i]=1.0/broj_uzoraka*i;
        //imU[i]=sin(2.0*M_PI/broj_uzoraka*i);
    }

    fft->TransDCT(reU,reI);
    ispisi(reI, reI,broj_bitova, false);

    fft->PromijeniSmjer(-1);
    fft->TransDCT(reI,reU);
    ispisi(reU, reU,broj_bitova, false);

    delete fft;

    delete[] reU;
    delete[] imU;
    delete[] reI;
    delete[] imI;
    cin.getline(unos,80);
    return 0;
}

void ispisi(double re[], double im[], int broj_bitova, bool ispisIm)
{
    int i;
    int broj_uzoraka = (1<<broj_bitova);
    cout << "Fourierov transformat signala (fft)" << endl;
    cout << "===================================" << endl;
    cout << "Realni:" << endl;
    for(i=0; i<broj_uzoraka; i++)
        printf("%.5lf ",re[i]);
    if(!ispisIm)
    {
        cout << endl;
        return;
    }
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<broj_uzoraka; i++)
        printf("%.5lf ",im[i]);
    cout << endl;
}
