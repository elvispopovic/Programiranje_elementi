#ifndef _FASTFOURIER_H
#define _FASTFOURIER_H

class Fft
{
private:
        unsigned int broj_uzoraka;
        double pi;
        unsigned char *redoslijed;
        double *kosinusi, *sinusi;
        double norma;
        double smjer;
        void predracun();
public:
        Fft(char smjer=1);
        ~Fft();
        void PromijeniSmjer(char smjer);
        void Transformiraj(double rex[], double imx[], double ref[], double imf[]);
        void Normiraj(double rex[], double imx[]);
};
#endif // _FASTFOURIER_H
