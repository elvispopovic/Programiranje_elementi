#ifndef _FASTFOURIER_H
#define _FASTFOURIER_H
#include "fft_dll.h"

class Fft : public InterfaceFFT
{
private:
        unsigned int broj_uzoraka, broj_bitova;
        double pi;
        unsigned int *fftRedoslijed, *dctRedoslijed;
        double *fftKosinusi, *fftSinusi, *dctKosinusi, *dctSinusi;
        double *rampKosinusi, *rampSinusi, *cos_min_sin, *cos_plu_sin;
        double *inBufferRe, *inBufferIm, *outBufferRe, *outBufferIm;

        double norma;
        double smjer;
        void predracun();
        unsigned int zrcaljenjeBitova(unsigned int, unsigned char bitova);
        void fft(double rex[], double imx[], double ref[], double imf[], double sinusi[], double kosinusi[], unsigned int redoslijed[], unsigned int uzoraka);
public:
        Fft(unsigned char bitova, char smjer=1);
        ~Fft();
        void PromijeniSmjer(char smjer);
        void TransDFT(double rex[], double imx[], double ref[], double imf[]);
        void TransDST(double x[], double f[]);
        void TransDCT(double x[], double f[]);
        void Normiraj(double rex[], double imx[]);
};
#endif // _FASTFOURIER_H
