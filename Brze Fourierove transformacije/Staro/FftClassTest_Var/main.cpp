#include <iostream>
#include "fastFourier.h"

using namespace std;

void ispisi(double[], double[], int);

int main()
{
    int i;
    int broj_bitova;
    int broj_uzoraka;
    double *reU = new double[1024];
    double *imU = new double[1024];
    double *reI = new double[1024];
    double *imI = new double[1024];

    broj_bitova=4;
    broj_uzoraka=(1<<broj_bitova);

    for(i=0; i<broj_uzoraka; i++)
    {
        reU[i]=1.0/broj_uzoraka*i;
        imU[i]=1.0/broj_uzoraka*i;
    }

    Fft *fft = new Fft(broj_bitova);
    fft->Transformiraj(reU,imU,reI,imI);
    fft->Normiraj(reI,imI);
    delete fft;
    ispisi(reI, imI,broj_bitova);

    delete[] reU;
    delete[] imU;
    delete[] reI;
    delete[] imI;
    return 0;
}

void ispisi(double reI[], double imI[], int broj_bitova)
{
    int i;
    int broj_uzoraka = (1<<broj_bitova);
    cout << "Fourierov transformat signala (fft)" << endl << endl;
    cout << "Realni:" << endl;
    for(i=0; i<broj_uzoraka; i++)
        printf("%.5lf ",reI[i]);
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<broj_uzoraka; i++)
        printf("%.5lf ",imI[i]);
    cout << endl;
}
