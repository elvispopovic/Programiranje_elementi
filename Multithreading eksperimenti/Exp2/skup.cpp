#include <iostream>
#include <set>
#include <random>

using namespace std;

int main()
{
    int i;
    set<int> skup;
    set<int>::iterator it;
    srand(10);
    for(i=0; i<100; i++)
        skup.insert(rand()%1000);
    for (it=skup.begin(); it!=skup.end(); ++it)
    std::cout << *it << " ";
    std::cout << "\n\n";


    for(auto it2=skup.cbegin(); it2!=skup.cend(); ++it2)
        std::cout << *it2 << " ";
    std::cout << '\n';

    return 0;
}
