#include <iostream>
#include <vector>

using namespace std;

int main()
{
    vector<int> vektor;
    vector<int>::iterator it;

    it=vektor.begin();
    vektor.insert(it,5);
    vektor.insert(vektor.begin(),10);
    vektor.insert(vektor.end(),101);

    for(it=vektor.begin(); it<vektor.end(); it++)
        cout << *it << " ";
    cout << '\n';

    return 0;
}
