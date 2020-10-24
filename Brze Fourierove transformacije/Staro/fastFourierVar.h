#ifndef _FASTFOURIER_H
#define _FASTFOURIER_H

class Fft
{
private:
        unsigned int broj_uzoraka, broj_bitova;
        double pi;
        unsigned short *redoslijed;
        double *kosinusi, *sinusi;
        double norma;
        double smjer;
        void predracun();
        unsigned short zrcaljenjeBitova(unsigned short);
public:
        Fft(unsigned char bitova, char smjer=1);
        ~Fft();
        void PromijeniSmjer(char smjer);
        void Transformiraj(double rex[], double imx[], double ref[], double imf[]);
        void Normiraj(double rex[], double imx[]);
};
#endif // _FASTFOURIER_H
