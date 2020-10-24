#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>

using namespace std;

mutex print_mutex;

int f1(int n,const char *ime, int seed)
{
    int i, r, rd;
    srand(seed);
    rand();
    rand();
    for(i=0,r=0; i<n; i++)
    {
        r++;
        rd=rand()%50;
        print_mutex.lock();
        cout << ime << ": brojim r= " << r << endl;
        print_mutex.unlock();
        this_thread::sleep_for(chrono::milliseconds(10+rd));
    }
    return r;
}


int main()
{
    thread t1(f1,100,"t1",25);
    thread t2(f1,100,"t2",26);

    t1.join();
    t2.join();
    return 0;
}
