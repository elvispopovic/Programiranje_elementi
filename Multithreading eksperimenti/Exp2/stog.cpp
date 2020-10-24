#include <iostream>
#include <stack>

using namespace std;

int main()
{
    int i;
    stack<int> stog;
    for(int i=0; i<=100; i++)
        stog.push(i);

    cout << "Size: " << endl;
    while(!stog.empty())
    {
        cout << stog.top() << " ";
        stog.pop();
    }
    return 0;
}
