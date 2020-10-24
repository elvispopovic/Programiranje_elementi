#include <iostream>
#include <random>
#include <chrono>
#include <functional>
//radi iskuljucivo kao 64 bitna aplikacija
//potreban je C++ 11 komajler

using namespace std;

inline void Zamjena(int& a, int& b);
inline bool Usporedba(int a, int b);
void PomiciGore(int *polje, int indeks);
void PomiciDolje(int *polje, int indeks, int granica);
void HeapSort(int *polje, int granica);

void NapuniPolje(int *polje, int granica);
void IspisiPolje(int *polje, int granica);
bool Test(int *polje, int granica);
unsigned long long DohvatiVrijeme();
void IspisiVrijeme(unsigned long long mikrosekunde);


int main()
{
    unsigned long long vrijeme;
    int n = 5000000;
    int *polje = new int[n+1];
    polje[0]=0;

    cout << "Punim polje sa " << n << " slucajnih brojeva." << endl;
    NapuniPolje(polje, n);

    cout << "Sortiram niz od " << n << " brojeva." << endl;
    vrijeme=DohvatiVrijeme();
    HeapSort(polje,n);
    vrijeme=DohvatiVrijeme()-vrijeme;

    if(n<=10000)
        IspisiPolje(polje,n);
    else
        cout << "Preveliko polje za ispisivanje u konzolu." << endl;
    cout << "------------------" << endl;
    cout << "1: " << polje[1] << endl << n << ": " << polje[n] << endl;
    cout << "Test: " << (Test(polje,n)?"Prosao":"Pao") << endl;

    cout << "Sortiranje obavljeno u: ";
    IspisiVrijeme(vrijeme);


    delete[] polje;
    return 0;
}

/* HEAP SORT FUNKCIJE */
inline void Zamjena(int& a, int& b)
{
    int t=a;
    a=b;
    b=t;
}

inline bool Usporedba(int a, int b)
{
    return a<=b?false:true;
}

void PomiciGore(int *polje, int indeks)
{
    int roditelj;
    if(indeks==1)
        return;
    roditelj = indeks >> 1;
    if(Usporedba(polje[indeks],polje[roditelj]))
    {
        Zamjena(polje[indeks],polje[roditelj]);
        PomiciGore(polje,roditelj);
    }
}

void PomiciDolje(int *polje, int indeks, int granica)
{
    int dijeteL=indeks<<1;
    int dijeteD=dijeteL+1;
    if(dijeteL>granica)
        return;
    if(dijeteD>granica)
    {
        if(Usporedba(polje[dijeteL],polje[indeks]))
        {
            Zamjena(polje[dijeteL],polje[indeks]);
            PomiciDolje(polje,dijeteL,granica);
        }
    }
    else
    {
        if(Usporedba(polje[dijeteL],polje[dijeteD]))
        {
            if(Usporedba(polje[dijeteL],polje[indeks]))
            {
                Zamjena(polje[dijeteL],polje[indeks]);
                PomiciDolje(polje,dijeteL,granica);
            }
        }
        else
        {
            if(Usporedba(polje[dijeteD],polje[indeks]))
            {
                Zamjena(polje[dijeteD],polje[indeks]);
                PomiciDolje(polje,dijeteD,granica);
            }

        }
    }
}

void HeapSort(int *polje, int granica)
{
    int i;
    for(i=2; i<=granica; i++)
        PomiciGore(polje,i);

    for(i=granica; i>1; i--)
    {
       Zamjena(polje[1],polje[i]);
       PomiciDolje(polje,1,i-1);
    }
}

/* UPRAVLJANJE POLJEM, MJERENJE I TESTIRANJE */
void NapuniPolje(int *polje, int granica)
{
    int i;
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    auto rand = bind(uniform_int_distribution<int>(0,granica*3),mt19937_64(seed));
    for(i=1; i<=granica; i++)
        polje[i]=rand();

}

void IspisiPolje(int *polje, int granica)
{
    int i;
    for(i=1; i<granica; i++)
        cout << polje[i] << ", ";
    cout << polje[i] << endl;
}

bool Test(int *polje, int granica)
{
    bool test = true;
    int i;
    for(i=1; i<granica; i++)
        if(Usporedba(polje[i],polje[i+1]))
        {
            test=false;
            break;
        }
    return test;
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
