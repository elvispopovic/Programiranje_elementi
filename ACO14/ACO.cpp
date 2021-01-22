#include "ACO.h"

using namespace std;


pass* passengers = nullptr;
node* nodes = nullptr;
car* cars = nullptr;
ant* ants = nullptr;


bool Solution(uint iter)
{
    nodeTraversal(&nodes[0]);
    return true;
}

void nodeTraversal(node *node)
{
    cout << "Current node: " << node->name << " (" << node->index << ")" << endl;
}


