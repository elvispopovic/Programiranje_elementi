#include <iostream>
#include "fastFourier.h"

using namespace std;

void ispisi(double[], double[]);

int main()
{
    int i;
    double *reU = new double[256];
    double *imU = new double[256];
    double *reI = new double[256];
    double *imI = new double[256];

    for(i=0; i<256; i++)
    {
        reU[i]=1.0/256.0*i;
        imU[i]=1.0/256.0*i;
    }

    Fft *fft = new Fft();
    fft->Transformiraj(reU,imU,reI,imI);
    fft->Normiraj(reI,imI);
    delete fft;
    ispisi(reI, imI);
    delete[] reU;
    delete[] imU;
    delete[] reI;
    delete[] imI;
    return 0;
}

void ispisi(double reI[], double imI[])
{
    int i;
    cout << "Fourierov transformat signala (fft)" << endl << endl;
    cout << "Realni:" << endl;
    for(i=0; i<256; i++)
        printf("%.5lf ",reI[i]);
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<256; i++)
        printf("%.5lf ",imI[i]);
    cout << endl;
}
