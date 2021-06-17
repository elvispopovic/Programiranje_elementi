#ifndef PHEROTREE_H
#define PHEROTREE_H
#include <iostream>
#include <cmath>
#include <random>
#define uint unsigned int
#define uchar unsigned char

#define BROJ_CVOROVA 7
#define BROJ_AUTOMOBILA 3
#define BROJ_PUTNIKA 12
#define PUTNIKA_U_CVORU 5

 

struct cvor;

struct automobil
{
    uint indeks;
    uchar kapacitet;
};

struct putnik
{
    uint indeks;
    float phero;
    float budget;
    cvor *odrediste;
};

struct cvor
{
    uint indeks;
    automobil *automob;
    putnik **putnici;
};

struct vjerojatnosti
{
    uint n;
    int odabrano; 
    float sum;
    uint *indeksi; 
    float *p; 
    float *kumulativ; 
};

void ispisPodataka(cvor *staza);
int IzaberiPutnika(cvor *trenutniCvor, bool *pokupljeni);
uint odaberiIzFrekventogPolja(float sum, uint n, float *vjerojatnosti);

#endif