#include <iostream>
#include <cmath>
#include <chrono>
#include <windows.h>
#include <complex.h>
#include "fastFourier.h"

using namespace std;


void ispisi(double[], double[], int, bool ispisIm=true);
/*  za racun vremena potrebno je koristiti
    c++11 tj. -std=c++11 ili vise */
unsigned long long DohvatiVrijeme();
void IspisiVrijeme(unsigned long long mikrosekunde);

int main()
{
    int i;
    unsigned long long vrijeme;
    int broj_bitova;
    int broj_uzoraka;
    char unos[80];

    double *reU;
    double *imU;
    double *reI;
    double *imI;

    broj_bitova=8;
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
        imU[i]=sin(6.28/broj_uzoraka*i);
    }
    vrijeme=DohvatiVrijeme();
    fft->TransDFT(reU,imU,reI,imI);
    vrijeme=DohvatiVrijeme()-vrijeme;

    cout << "Fourijerov transformat signala (dft)" << endl;
    IspisiVrijeme(vrijeme);
    cout << "===================================" << endl;
    ispisi(reI, imI,broj_bitova);

    fft->PromijeniSmjer(-1);
    vrijeme=DohvatiVrijeme();
    fft->TransDFT(reI,imI,reU,imU);
    vrijeme=DohvatiVrijeme()-vrijeme;

    cout << "Inverzni fourijerov transformat spektra (idft)" << endl;
    IspisiVrijeme(vrijeme);
    cout << "===================================" << endl;
    ispisi(reU, imU,broj_bitova);
//kraj DFT testa
*/

//DST test
    for(i=0; i<broj_uzoraka; i++)
    {
        //reU[i]=cos(2.0*M_PI/broj_uzoraka*i);
        reU[i]=1.0/broj_uzoraka*i;
        //imU[i]=sin(2.0*M_PI/broj_uzoraka*i);
    }
    vrijeme=DohvatiVrijeme();
    fft->TransDST(reU,reI);
    vrijeme=DohvatiVrijeme()-vrijeme;

    cout << "Sinusni transformat signala (dst)" << endl;
    IspisiVrijeme(vrijeme);
    cout << "===================================" << endl;
    ispisi(reI, reI,broj_bitova, false);

    fft->PromijeniSmjer(-1);

    vrijeme=DohvatiVrijeme();
    fft->TransDST(reI,reU);
    vrijeme=DohvatiVrijeme()-vrijeme;

    cout << endl;
    cout << "Inverzni sinusni transformat spektra (idst)" << endl;
    IspisiVrijeme(vrijeme);
    cout << "===================================" << endl;
    ispisi(reU, reU,broj_bitova, false);
//kraj DST testa


    delete fft;
    delete[] reU;
    delete[] imU;
    delete[] reI;
    delete[] imI;
    cout << "Za nastavak unesi bilo sto." << endl;
    cin.getline(unos,80);
    return 0;
}

void ispisi(double re[], double im[], int broj_bitova, bool ispisIm)
{
    int i;
    int broj_uzoraka = (1<<broj_bitova);
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

unsigned long long DohvatiVrijeme()
{
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}
void IspisiVrijeme(unsigned long long mikrosekundi)
{
    unsigned long long mikrosekunde;
    unsigned long long milisekunde;
    unsigned long long sekunde;
    sekunde = mikrosekundi/1000000ULL;
    milisekunde = (mikrosekundi/1000ULL)%1000ULL;
    mikrosekunde = mikrosekundi%1000ULL;
    cout << sekunde << " sekundi, " << milisekunde << " milisekundi, " << mikrosekunde << " mikrosekundi." << endl;
    cout << "(ukupno mikrosekundi: " << mikrosekundi << ")" << endl;
}
