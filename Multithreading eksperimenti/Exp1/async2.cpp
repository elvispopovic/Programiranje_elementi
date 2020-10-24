#include <iostream>
#include <chrono>
#include <thread>
#include <future>

using namespace std;

mutex print_lock;
int f1(int n)
{
    int i, z;
    for(i=0,z=0; i<n; i++)
    {
        z+=i;
        this_thread::sleep_for(chrono::milliseconds(99));
        print_lock.lock();
        printf("f1: dodajem %d\n",i);
        print_lock.unlock();
    }
    print_lock.lock();
    printf("f1 gotova.\n");
    print_lock.unlock();
    return z;
}

int f2(int n)
{
    int i,z;
    for(i=0,z=1; i<n;i++)
    {
        z=z+2;
        this_thread::sleep_for(chrono::milliseconds(101));
        print_lock.lock();
        printf("f2: dodajem novi z = %d.\n",z);
        print_lock.unlock();
    }
    print_lock.lock();
    printf("f2 je gotova.\n");
    print_lock.unlock();
    return z;
}

int main()
{
    int i, rez;
    cout << "Test pokrenut." << endl;
    /*ako je deffered, izvodi se u istoj dretvi serijski
      a ako je async, onda se izvodi paralelno*/
    future<int> fut_rez1 = async(launch::deferred,f1,51);
    future<int> fut_rez2 = async(launch::deferred,f2,49);
    for(i=0; i<100; i++)
    {
        print_lock.lock();
        printf("Brojim u glavnoj dretvi %d\n",i);
        print_lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    rez = fut_rez1.get();
    print_lock.lock();
    printf("f1 je završila sa rezultatom %d.\n",rez);
    print_lock.unlock();
    rez = fut_rez2.get();
    print_lock.lock();
    printf("f2 je završila sa rezultatom %d.\n",rez);
    print_lock.unlock();

    return 0;
}
