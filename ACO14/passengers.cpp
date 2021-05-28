#include "ACO.h"

using namespace std;

void calculatePassengers(antNode *nodes, uint nodeCounter, bool *passPicked)
{
    int i, j;
    antNode *ptAntNode;
    bool *ptBool;
    for(j=0, ptAntNode=nodes; j<nodeCounter; j++, ptAntNode++)
    {
        cout << "Node: " << ptAntNode->curNode->name << ", nPass: " << ptAntNode->curNode->nPassengers << endl;

        for(i=0, ptBool = passPicked; i<ptAntNode->curNode->nPassengers; i++,ptBool++)
            *ptBool = false;

        //privremeno za testiranje
        
        for(i=0, ptBool = passPicked; i<ptAntNode->curNode->nPassengers/2; i++, ptBool++)
            *ptBool = true;
            

        PickPassengers(ptAntNode->curNode, ptAntNode->curNode->nPassengers, passPicked);
    }
}
