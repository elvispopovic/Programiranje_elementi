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




int main()
{
    int polje[100];
    int polje2[100]={0};

    thread t1([&polje](){f1(polje);});
    thread t2([&polje](){f2(polje);});
    thread t3([polje2]()mutable{f1(polje2);});
    t1.join();
    t2.join();
    t3.join();
    cout << "Ispisujem sadržaj polja u objektu:\n" << endl;
    for(int i=0; i<99; i++)
        cout << polje[i] << ", ";
    cout << polje[99] << "\n";

    cout << "Ispisujem sadržaj polja2 u objektu:\n" << endl;
    for(int i=0; i<99; i++)
        cout << polje2[i] << ", ";
    cout << polje2[99] << "\n";


    return 0;
}
