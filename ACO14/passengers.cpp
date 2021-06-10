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
void calculatePassengers(antNode *aNodes, uint nodeCounter, passContext *passengerContext)
{
    uint i, j;
    pass **pptPass1, **pptPass2;
    uchar   nPass, // number of passengers in node
            nSpace, // space in current car
            nUnload; // number of passengers at destination (leaving)
    int iPicked; // result of statistic pick
    antNode *ptAntNode;
    bool *ptBool;
    
    /* reset all passengers in array */
    for(i=0, pptPass1=passengerContext->passOnBoard, pptPass2=passengerContext->lastPassOnBoard; 
        i<prData.maxCarPassengers; i++, pptPass1++, pptPass2++)
    {
        (*pptPass1) = nullptr;
        (*pptPass1) = nullptr;
    }
    /* init context */
    passengerContext->updateFromContext = false;
    passengerContext->iterCounter = 0;
    passengerContext->lastAntNodeIndex = 0;
    passengerContext->lastANode = aNodes;
    passengerContext->lastANode = 0;
    passengerContext->lastNPicked = 0;



    for(j=0, passengerContext->nPicked = 0, ptAntNode=aNodes; j<nodeCounter; j++, ptAntNode++)
    {
        nPass = ptAntNode->curNode->nPassengers; // node passenger number
        nUnload = UnloadPassengers(passengerContext->passOnBoard, ptAntNode->curNode);
        /* update leaving passengers */
        passengerContext->nPicked -= nUnload;        

        /* using temporary array in context */
        for(i=0, ptBool = passengerContext->passPicked; i<ptAntNode->curNode->nPassengers; i++,ptBool++)
                *ptBool = false;

        /* remain space in car */
        if(ptAntNode->carOut->carPassLimit >= passengerContext->nPicked)
            nSpace = ptAntNode->carOut->carPassLimit - passengerContext->nPicked;
        else
            nSpace = 0;


        cout << "Node: " << ptAntNode->curNode->name << ", unloaded: " << (int)nUnload
        << ", car: " << ptAntNode->carOut->name << ", capacity: " << (int)ptAntNode->carOut->carPassLimit
        << ", nPass: " << (int)nPass
        << ", nPicked: " << (int)passengerContext->nPicked << ", nSpace: " << (int)nSpace
        <<  endl;

        /* loop - pick multiple passengers */
        for(i=0; i<nPass && nSpace > 0; i++)
        {
                /* picking one passenger */
                iPicked = PickPassengers(ptAntNode->curNode, ptAntNode->curNode->nPassengers, passengerContext->passPicked);
                
                if( iPicked >= 0 &&
                    checkPassenger(aNodes, ptAntNode, ptAntNode->curNode->passengers[iPicked]) == true
                ) // update total pick
                {
                    passengerContext->passPicked[iPicked] = true;
                    AddPassenger(ptAntNode->curNode->passengers[iPicked], passengerContext->passOnBoard);
                    //ptAntNode->pickedPassengers <--- dodati putnika u polje pokupljenih putnika za cvor vidjeti koje polje opt ili obicno
                    (passengerContext->nPicked)++;

                    /* temporary */
                    cout << "Picked: " << iPicked << ", passenger: " << (iPicked>=0?ptAntNode->curNode->passengers[iPicked]->name:"None")
                    << (iPicked==ptAntNode->curNode->nPassengers-1?"*":"") << endl;
                }
                        /* remain space in car */
            if(ptAntNode->carOut->carPassLimit >= passengerContext->nPicked)
                nSpace = ptAntNode->carOut->carPassLimit - passengerContext->nPicked;
            else
            {
                nSpace = 0;
                break;
            }
        }
        ListPassengersOnBoard(passengerContext->passOnBoard);
        /* end of loop */
        if(ptAntNode->carOut->carPassLimit >= passengerContext->nPicked)
            nSpace = ptAntNode->carOut->carPassLimit - passengerContext->nPicked;
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
