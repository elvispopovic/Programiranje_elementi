#include "ACO.h"

using namespace std;

void calculatePassengers(antNode *nodes, uint nodeCounter)
{
    int i;
    antNode *ptAntNode;
    for(i=1, ptAntNode=nodes; i<nodeCounter; i++, ptAntNode++)
    {
        cout << "Node: " << ptAntNode->curNode->name << ", nPass: " << ptAntNode->curNode->nPassengers << endl;
        PickPassengers(ptAntNode->curNode, ptAntNode->curNode->nPassengers);
    }
}
