#include <iostream>
#include <chrono>
#include <thread>
#include <future>

using namespace std;

mutex print_lock;

void f1(int &x)
{
    int i;
    for(i=0; i<100; i++)
    {
        x+=i;
        this_thread::sleep_for(chrono::milliseconds(10));
        cout << "Povecavam x: " << x << endl;
    }
}

void f2(string const& polje)
{
    int i;
    for(i=0; i<100; i++)
        cout << "Ispisujem vrijednost iz stringa " <<  polje << ".\n";

}





int main()
{
    int x=0;
    char polje[100];
    sprintf(polje,"Bla bla");
    thread t1(f1, ref(x));
    thread t2(f2,string(polje));
    t1.join();
    t2.join();
    cout << "Ispisujem vrijednost x: "<< x << "\n" << endl;
    cout << "Ispisujem vrijednost polja:" << polje << endl;


    return 0;
}
