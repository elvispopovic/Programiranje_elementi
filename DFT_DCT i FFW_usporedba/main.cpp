#include <iostream>
#include <cmath>
#include <windows.h>
#include <complex.h>
#include "fastFourier.h"

//fastest fourier trans in the wes usporedba
#include "fftw3.h"

using namespace std;

typedef fftw_plan (*planF)(int, fftw_complex*, fftw_complex*, int, unsigned);
typedef fftw_plan (*planDCT) (int, double*, double*, fftw_r2r_kind, unsigned);
typedef void (*execPlan)(const fftw_plan);
typedef void (*execPlan1)(fftw_plan);
typedef void (*freeData) (void*);
typedef void* (*ffmalloc) (size_t);

void ispisi(double[], double[], int, bool ispisIm=true);

int main()
{
    int i;
    int broj_bitova;
    int broj_uzoraka;
    char unos[80];

    fftw_plan p;

    double *reU;
    double *imU;
    double *reI;
    double *imI;

    double *in, *out;

    HMODULE hFFTW = LoadLibrary("libfftw3-3.dll");
    if(!hFFTW)
    {
        cout << "Ne mogu ucitati biblioteku" << endl;
        cin.getline(unos,80);
        return 0;
    }

    planF fftw_plan_dft_1d = (planF)GetProcAddress(hFFTW,"fftw_plan_dft_1d");
    planDCT fftw_plan_r2r_1d =(planDCT)GetProcAddress(hFFTW, "fftw_plan_r2r_1d");
    execPlan fftw_execute  = (execPlan)GetProcAddress(hFFTW,"fftw_execute");
    execPlan1 fftw_destroy_plan = (execPlan1)GetProcAddress(hFFTW,"fftw_destroy_plan");
    ffmalloc fftw_malloc   = (ffmalloc)GetProcAddress(hFFTW,"fftw_malloc");
    freeData fftw_free = (freeData)GetProcAddress(hFFTW,"fftw_free");

    broj_bitova=4;
    broj_uzoraka=(1<<broj_bitova);

    reU = new double[broj_uzoraka];
    imU = new double[broj_uzoraka];
    reI = new double[broj_uzoraka];
    imI = new double[broj_uzoraka];

    Fft *fft = new Fft(broj_bitova);

    in = (double*) fftw_malloc(sizeof(double) * broj_uzoraka);
    out = (double*) fftw_malloc(sizeof(double) * broj_uzoraka);

    //plan mora biti prije podataka ako se koristi FFTW_MEASURE, jer brise ulazne podatke
    p = fftw_plan_r2r_1d(broj_uzoraka, in, out, FFTW_RODFT10, FFTW_MEASURE);

    for(i=0; i<broj_uzoraka; i++)
    {
        reU[i]=cos(2.0*M_PI/broj_uzoraka*i);
        //reU[i]=0.5-1.0/broj_uzoraka*i;
        //reU[i]=sin(2.0*M_PI/broj_uzoraka*i);
        in[i]=reU[i];
    }

    fft->TransDST(reU,reI);
    //fft->Normiraj(reI,imI);
    ispisi(reI, reI,broj_bitova, false);

    fftw_execute(p);
    fftw_destroy_plan(p);

    ispisi(out,out, broj_bitova,false);


    //koristimo FFTW_ESTIMATE jer ne zelimo brisati ulazne podatke. Izvodi se suboptimalno
    p = fftw_plan_r2r_1d(broj_uzoraka, out, in, FFTW_RODFT01, FFTW_ESTIMATE);
    fft->PromijeniSmjer(-1);
    fft->TransDST(reI,reU);
    ispisi(reU, reU,broj_bitova, false);
    fftw_execute(p);
    ispisi(in,in, broj_bitova,false);
    fftw_destroy_plan(p);
    delete fft;



    fftw_free(in);
    fftw_free(out);
    FreeLibrary(hFFTW);
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
