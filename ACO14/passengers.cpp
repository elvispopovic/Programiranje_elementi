#include "ACO.h"

bool checkPassenger(antNode *nodes, antNode *currentNode, pass *passenger);
void AddPassenger(pass *passenger, pass **passArray);
uint UnloadPassengers(pass **passArray, node *currentNode);
void ListPassengersOnBoard(pass **passArray);
using namespace std;

/*
nodes - ant nodes, non opt or opt
node counter - number of and nodes
passArray - array of passengers onboard
passPicked - temporary array of picked flags, no need to limit it by counter
*/
void calculatePassengers(ant *currentAnt)
{
    uint i, j;
    pass **pptPass1;
    uchar   nPass, // number of passengers in node
            nSpace, // space in current car
            nUnload, // number of passengers at destination (leaving)
            nPicked;
    int iPicked; // result of statistic pick
    antNode *ptAntNode;
    bool *ptBool;
    
    /* reset all passengers in array */
    for(i=0, pptPass1=currentAnt->passOnBoard; 
        i<prData.maxCarPassengers; i++, pptPass1++)
    {
        (*pptPass1) = nullptr;
    }

    for(j=0, nPicked = 0, ptAntNode=currentAnt->nodes; j<currentAnt->nodeCounter; j++, ptAntNode++)
    {
        nPass = ptAntNode->curNode->nPassengers; // node passenger number
        nUnload = UnloadPassengers(currentAnt->passOnBoard, ptAntNode->curNode);
        /* update leaving passengers */
        nPicked -= nUnload;        

        /* using temporary pickup array */
        for(i=0, ptBool = currentAnt->passPicked; i<ptAntNode->curNode->nPassengers; i++,ptBool++)
                *ptBool = false;

        /* remain space in car, temporary no negative values (too small car)*/
        if(ptAntNode->carOut->carPassLimit >= nPicked)
            nSpace = ptAntNode->carOut->carPassLimit - nPicked;
        else
            nSpace = 0;


        cout << "Node: " << ptAntNode->curNode->name << ", unloaded: " << (int)nUnload
        << ", car: " << ptAntNode->carOut->name << ", capacity: " << (int)ptAntNode->carOut->carPassLimit
        << ", nPass: " << (int)nPass
        << ", nPicked: " << (int)nPicked << ", nSpace: " << (int)nSpace
        <<  endl;

        /* loop - pick multiple passengers until there are passengers and until there are space into car */
        for(i=0; i<nPass && nSpace > 0; i++)
        {
                /* picking one passenger */
                iPicked = PickPassengers(ptAntNode->curNode, ptAntNode->curNode->nPassengers, currentAnt->passPicked);
                
                if( iPicked >= 0 &&
                    checkPassenger(currentAnt->nodes, ptAntNode, ptAntNode->curNode->passengers[iPicked]) == true
                ) // update total pick
                {
                    currentAnt->passPicked[iPicked] = true;
                    AddPassenger(ptAntNode->curNode->passengers[iPicked], currentAnt->passOnBoard);
                    //ptAntNode->pickedPassengers <--- dodati putnika u polje pokupljenih putnika za cvor vidjeti koje polje opt ili obicno
                    nPicked++;

                    /* temporary */
                    cout << "Picked: " << iPicked << ", passenger: " << (iPicked>=0?ptAntNode->curNode->passengers[iPicked]->name:"None")
                    << (iPicked==(int)(ptAntNode->curNode->nPassengers)-1?"*":"") << endl;
                }
                        /* remain space in car */
            if(ptAntNode->carOut->carPassLimit >= nPicked)
                nSpace = ptAntNode->carOut->carPassLimit - nPicked;
            else
            {
                nSpace = 0;
                break;
            }
        }
        ListPassengersOnBoard(currentAnt->passOnBoard);
        /* end of loop */
        if(ptAntNode->carOut->carPassLimit >= nPicked)
            nSpace = ptAntNode->carOut->carPassLimit - nPicked;
        else
            nSpace = 0;
    }
}


uint UnloadPassengers(pass **passArray, node *currentNode)
{
    uint i, counter;
    pass **pptPass;
    if(currentNode == nullptr)
        return 0;
    for(i=0, counter=0, pptPass=passArray; i<prData.maxCarPassengers; i++, pptPass++)
    {
        if((*pptPass)!=nullptr && (*pptPass)->endNode == currentNode->index)
        {
            counter++;
            (*pptPass) = nullptr;
        }
    }
    return counter;
}

/* TODO: make circular relaxation: first node is last, so no pass */
bool checkPassenger(antNode *aNodes, antNode *currentNode, pass *passenger)
{
    antNode *ptAntNode;
    for(ptAntNode = aNodes; ptAntNode->curNode->index != currentNode->curNode->index; ptAntNode++)
        if(ptAntNode->curNode->index == passenger->endNode)
        {
            cout << "Passenger " << passenger->name << " destination " << nodes[passenger->endNode].name << " passed." << endl;
            return false;
        }
    return true;
}

void AddPassenger(pass *passenger, pass **passArray)
{

    // TODO: ne dodavati putnike ciji su cvorovi prosli, jer nemaju sanse za stici na odrediste 
    uint i;
    pass **pptPass;
    for(i=0, pptPass=passArray; i<prData.maxCarPassengers; i++, pptPass++)
    {
        if(*pptPass == nullptr)
        {
            *pptPass = passenger;
            break;
        }
    }
}

/* for testing purpose */
void ListPassengersOnBoard(pass **passArray)
{
    uint i;
    pass **pptPass;
    cout << "Passengers onboard: ";
    for(i=0, pptPass = passArray; i<prData.maxCarPassengers; i++, pptPass++)
        if((*pptPass) != nullptr)
            cout << (*pptPass)->name << " ";
    cout << endl;
}
