#include <iostream>
#include <cmath>
#include <random>
#define unsign int uint;
#define BROJ_CVOROVA 7
#define BROJ_PUTNIKA 12
#define PUTNIKA_U_CVORU 5

struct cvor;

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


float prijelazi[BROJ_CVOROVA]
{
    10.0, 13.0, 9.0, 11.0, 8.0, 12.0, 7.0
};

float budgeti[BROJ_PUTNIKA]
{
    6.0, 5.0, 3.0, 4.0, 8.0, 4.0,
    6.0, 2.0, 4.0, 5.0, 3.0, 4.0
};

uint odredista[BROJ_PUTNIKA]
{
    3,4,6,4,
    5,4,
    0,4,
    1,
    2,6,
    3
};

short putniciUCvorovima[BROJ_CVOROVA][PUTNIKA_U_CVORU]
{
    { 0, 1, 2, 3,-1},
    { 4, 5,-1,-1,-1},
    { 6, 7,-1,-1,-1},
    {-1,-1,-1,-1,-1},
    { 8,-1,-1,-1,-1},
    { 9,10,-1,-1,-1},
    {11,-1,-1,-1,-1}
};

vjerojatnosti vj;


void ispisPodataka(cvor *staza);
int IzaberiPutnike(cvor *trenutniCvor, uint mjesta, bool *pokupljeni);
uint odaberiIzFrekventogPolja(float sum, uint n, float *vjerojatnosti);

using namespace std;

mt19937* mersenneGenerator;

int main()
{
    uint i, j;
    int izbor, mjesta;
    cvor *staza, *ptCvor;
    bool *pokupljeni, *ptBool;
    putnik *putnici, *ptPutnik, **pptPutnik;

    mt19937::result_type seed = time(0);
    mersenneGenerator = new mt19937(seed);

    staza = new cvor[BROJ_CVOROVA];
    putnici = new putnik[BROJ_PUTNIKA];
    pokupljeni = new bool[PUTNIKA_U_CVORU];
    for(i=0, ptPutnik=putnici; i<BROJ_PUTNIKA; i++, ptPutnik++)
    {
        ptPutnik->indeks = i;
        ptPutnik->budget = budgeti[i];
        ptPutnik->odrediste = staza+odredista[i];
    }
    for(j=0, ptCvor=staza; j<BROJ_CVOROVA; j++, ptCvor++)
    {
        ptCvor->indeks = j;
        ptCvor->putnici = new putnik*[PUTNIKA_U_CVORU];
        for(i=0, pptPutnik=ptCvor->putnici; i<PUTNIKA_U_CVORU; i++, pptPutnik++)
            if(putniciUCvorovima[j][i]==-1)
                *pptPutnik = nullptr;
            else
                *pptPutnik = putnici+putniciUCvorovima[j][i];        
    }


    vj.indeksi = new uint[PUTNIKA_U_CVORU];
    vj.kumulativ = new float[PUTNIKA_U_CVORU];
    vj.p = new float[PUTNIKA_U_CVORU];
    cout << "Sva polja alocirana." << endl;
    ispisPodataka(staza);

    for(j=0, ptCvor=staza, mjesta=5; j<BROJ_CVOROVA; j++, ptCvor++)
    {
        for(i=0, ptBool=pokupljeni; i<PUTNIKA_U_CVORU; i++, ptBool++)
            *ptBool = false;
        izbor = IzaberiPutnike(ptCvor, mjesta, pokupljeni);
        for(i=0; i<vj.n; i++)
            cout << "indeks: " << ptCvor->putnici[vj.indeksi[i]]->indeks << ", p: " << vj.p[i] << ", kum: " << vj.kumulativ[i] << endl;

        if(izbor == -1)
            cout << "Nije nitko izabran." << endl;
        else
            cout << "Izabran je putnik " << ptCvor->putnici[izbor]->indeks << endl;
    }

    delete[] vj.p;
    delete[] vj.kumulativ;
    delete[] vj.indeksi;
    delete[] pokupljeni;
    for(i=0, ptCvor=staza; i<BROJ_CVOROVA; i++, ptCvor++)
        delete[] ptCvor->putnici;
    delete[] putnici;
    delete[] staza;
    delete mersenneGenerator;
    cout << "Sva polja dealocirana." << endl;
    return 0;
}

void ispisPodataka(cvor *staza)
{
    uint i, j;
    cvor *ptCvor;
    putnik **pptPutnik;
    for(j=0, ptCvor=staza; j<BROJ_CVOROVA; j++, ptCvor++)
    {
        cout << "Cvor " << j << ", cijena prijelaza: " << prijelazi[j] << ", putnici:" << endl;
        for(i=0, pptPutnik=ptCvor->putnici; i<PUTNIKA_U_CVORU; i++,pptPutnik++)
            if(putniciUCvorovima[j][i] != -1)
                cout << " -putnik " << (*pptPutnik)->indeks << ", budget: " << (*pptPutnik)->budget << ", odrediste: " << (*pptPutnik)->odrediste->indeks << endl; 
            else if(i==0)
                cout << "Nema putnika." << endl;
    }
}

int IzaberiPutnike(cvor *trenutniCvor, uint mjesta, bool *pokupljeni)
{
    uint i, nPutnika;
    uint *ptUint;
    bool *ptBool;
    putnik **pptPutnik;
    float p, sum;
    float *ptFloat1, *ptFloat2;

    p=1.0;
    for(i=0, nPutnika=0, pptPutnik=trenutniCvor->putnici, ptBool=pokupljeni,
        sum = 0, ptUint = vj.indeksi, ptFloat1 = vj.kumulativ, ptFloat2 = vj.p; 
        i<PUTNIKA_U_CVORU; 
        i++, pptPutnik++, ptBool++)
        {
            if(*pptPutnik != nullptr && *ptBool == false)
            {
                p *= 0.9; //privremeno
                *(ptUint++) = i;
                *(ptFloat1++) = sum;
                *(ptFloat2++) = p;
                sum += p;
                nPutnika++;
            }
        }
    vj.n = nPutnika;
    vj.sum = sum;
    if(nPutnika==0 || mjesta==0) // none to select - no selection
    {
        vj.odabrano = -1;
        return -1;
    }
    else if(nPutnika == 1) // one to select - no selection
    {
        vj.odabrano = 0;
        return vj.indeksi[0];
    }
    i = odaberiIzFrekventogPolja(sum, nPutnika, vj.kumulativ);
    vj.odabrano = i;
    return vj.indeksi[i];
}

uint odaberiIzFrekventogPolja(float sum, uint n, float *vjerojatnosti)
{
    float l, r;
    uint left=0;
    uint right = n; //probabilities from 0 to n-1, right limit on n
    uint center =(left+right)>>1;
    l = nextafter(sum, numeric_limits<float>::max()); //upper closed interval limit
    uniform_real_distribution<float> distribution((float)0.0,l);
    r = distribution(*mersenneGenerator);
    while(left<center)
    {
        if(r >= *(vjerojatnosti+center))
            left = center;
        else
            right = center;
        center = (left+right)>>1;
    }
    return center;
}