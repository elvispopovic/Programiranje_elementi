#include <iostream>
#include <chrono>
#include <thread>
#include <future>

using namespace std;

mutex print_lock;

void f1(int *polje)
{
    int i;
    for(i=0; i<100; i++)
    {
        polje[i]=i;
        this_thread::sleep_for(chrono::milliseconds(10));
        cout << "Upisujem u polje " << i << endl;
    }
}

void f2(int *polje)
{
    int i;
    for(i=0; i<100; i++)
    {
        polje[i]=2*i;
        this_thread::sleep_for(chrono::milliseconds(10));
        cout << "Povecavam vrijednosti polja na indexu " << i <<": " << polje[i] << endl;
    }
}

class DretveniObjekt
{
public:
    int *polje;
    DretveniObjekt()
    {
        polje = new int[100];
    }
    ~DretveniObjekt()
    {
        delete[] polje;
    }
    //operator funkcije koja æe se proslijediti thread objektu
    void operator() () const
    {
        f1(polje);
        f2(polje);
    }
};



int main()
{
    DretveniObjekt drObj;
    thread t((drObj));
    t.join();
    cout << "Ispisujem sadržaj polja u objektu:\n" << endl;
    for(int i=0; i<99; i++)
        cout << drObj.polje[i] << ", ";
    cout << drObj.polje[99] << "\n";
    return 0;
}
