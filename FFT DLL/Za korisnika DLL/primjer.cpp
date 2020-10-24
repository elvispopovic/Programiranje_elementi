#include <iostream>
#include <cmath>
#include "fft_dll.h"
#define bitova 4

using namespace std;

int main()
{
    int i, uzoraka;
    //pokazivac na objekt koji ce se dobaviti iz DLL-a
    InterfaceFFT *FFTObjekt;
    //pokazivac na funkciju koja ce dobaviti objekt
    pfInit InicijalizirajFFT;
    //polja podataka za rad sa fourierovim transformacijama
    double *ulazRe, *ulazIm, *izlazRe, *izlazIm;

    HINSTANCE hInst = LoadLibrary("FFT_DLL.dll");
    if(!hInst)
	{
		cout << "Ne mogu otvoriti dll." << endl;
		return 1;
	}
	else
        cout << "FFT_DLL biblioteka je uspjesno ucitana." << endl;
    InicijalizirajFFT = (pfInit)GetProcAddress(hInst, "InicijalizirajFFT");
    if(!InicijalizirajFFT)
    {
        cout << "Nije uspjelo dobavljanje funkcije za inicijalizaciju objekta." << endl;
        FreeLibrary(hInst);
        return 1;
    }
    cout << "Funkcija za inicijalizaciju objekta je uspjesno dobavljena." << endl;
    FFTObjekt = InicijalizirajFFT(bitova,1);
    if(!FFTObjekt)
    {
        cout << "Nije uspjelo inicijaliziranje FFT objekta." << endl;
        FreeLibrary(hInst);
        return 1;
    }
    cout << "Objekt je uspjesno inicijaliziran. FFT je spremna za rad." << endl;
    cout << "********************************************************" << endl;

    uzoraka=1<<bitova;
    ulazRe = new double[uzoraka];
    ulazIm = new double[uzoraka];
    izlazRe = new double[uzoraka];
    izlazIm = new double[uzoraka];
    for(i=0; i<uzoraka; i++)
    {
        ulazRe[i]=-0.3+1.0/uzoraka*i;
        ulazIm[i]=cos(M_PI/uzoraka*i);
    }
    FFTObjekt->Normiraj(ulazRe,ulazIm);
    cout << "Ulazni podaci:" << endl;
    cout << "Realni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << ulazRe[i] << " ";
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << ulazIm[i] << " ";
    cout << endl;
    FFTObjekt->TransDFT(ulazRe,ulazIm,izlazRe,izlazIm);
    FFTObjekt->Normiraj(izlazRe,izlazIm);
    cout << "Ulazni podaci su transformirani. Izlazni podaci su:" << endl;
    cout << "Realni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << izlazRe[i] << " ";
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << izlazIm[i] << " ";
    cout << endl;
    cout << "Provodi se inverzna fourierova transformacija:" << endl;
    FFTObjekt->PromijeniSmjer(-1);
    FFTObjekt->TransDFT(izlazRe,izlazIm,ulazRe,ulazIm);
    cout << "Ulazni podaci su transformirani. Izlazni podaci su:" << endl;
    cout << "Realni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << ulazRe[i] << " ";
    cout << endl << "Imaginarni:" << endl;
    for(i=0; i<uzoraka; i++)
        cout << ulazIm[i] << " ";
    cout << endl;

    delete[] ulazRe;
    delete[] ulazIm;
    delete[] izlazRe;
    delete[] izlazIm;
    cout << "********************************************************" << endl;
    delete FFTObjekt;
    cout << "FFT objekt je unisten." << endl;
    FreeLibrary(hInst);
    cout << "FFT DLL biblioteka je uklonjena iz memorije." << endl;
    return 0;
}
