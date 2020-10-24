#include "ACO.h"

using namespace std;

/* find passenger destination node distance */
uint findRadius(tourArc* tourNodes, node *destNode)
{
    tourArc *ptTourArc;
    uint radius;
    for(ptTourArc = tourNodes, radius=0; ptTourArc->a != nullptr; ptTourArc++, radius++)
    {
        if(destNode->n == nodes[ptTourArc->a->row].n)
            return radius;
    }
    return numeric_limits<uint>::max();
}

bool sortCompareFunction(passProb ptProb1, passProb ptProb2)
{
    return (ptProb1.p > ptProb2.p);
}

uint CalculatePassengerProbabilities(tourArc* currentArc)
{
    
    register uint i;
    uint n;
    pass* ptPass;
    passProb *ptProb;
    uint radius;
    uint counter;
    float eta;
    ptProb = passProbArray;

    ptPass = nodes[currentArc->a->row].passengers;
    n = nodes[currentArc->a->row].nPass;
    for(i=0, counter=0; i<n; i++, ptPass++)
    {
        radius = findRadius(currentArc, ptPass->destinationNode);
        if(radius == numeric_limits<uint>::max())
            continue;
        /* Heuristic part
        * 
        */
       
        eta = 1.0/radius + ptPass->budget / maxBudget;
        ptProb->p = pow(ptPass->tau, ALPHA)+
                pow(eta, BETA);
                
        ptProb->pa = ptPass;
        ptProb++;
        
        counter++;
    }
    /* probability array must be sorted */
    if(counter > 1)
        sort(passProbArray, passProbArray+counter, sortCompareFunction);
    return counter;
}

passProb* PassengerSelect(uint n)
{
    register uint i;
    uint select;
    float sum, *ptFloatSel;
    register passProb* ptPassProb;
    if(n<1 || n>prData.nPass)
        return nullptr;
    for(i=0, sum=0.0, ptFloatSel = selProbArray, ptPassProb=passProbArray; i<n; i++, ptPassProb++, ptFloatSel++)
    {
        *ptFloatSel = sum;
        sum+=ptPassProb->p;
    } 
    select = selectFromFreqArray(sum,n);
    return &passProbArray[select];
}

/* pick possible passengers */
void pickPassengers(car *currentCar, node *currentNode, uchar nPassengersToPick, uchar& nPickedPassengers)
{
    uchar i, j;
    passProb *ptPassProb;
    for(i=0, j=0, ptPassProb=passProbArray; i<nPassengersToPick && nPickedPassengers<currentCar->carPassLimit; i++, ptPassProb++)
    {
        while(j<maxCarPassLimit-1 && carPassArray[j] != nullptr) //jump over non empty elements
            j++;
        if(carPassArray[j]==nullptr)
        {
            carPassArray[j]=ptPassProb->pa;
            nPickedPassengers++;
        }
    }
}

void leavePassengers(node* currentNode, uchar& nPickedPassengers)
{
    uchar i;
    pass** pptPass;
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if((*pptPass)!=nullptr && (*pptPass)->destinationNode->n == currentNode->n)
        {
            *pptPass = nullptr;
            if(nPickedPassengers>0)
                nPickedPassengers--;
            
        }
}

float chargePassengers(float costShare)
{
    uchar i;
    pass** pptPass;
    float shortage = 0.0;
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if(*pptPass != nullptr)
        {
            (*pptPass)->currentBudget-=costShare; //charge passenger
            if((*pptPass)->currentBudget < 0.0) //if passenger doesn't have enough money
            {
                shortage -= (*pptPass)->currentBudget; //add shortage as absolute value
                (*pptPass)->currentBudget = 0.0; //passenger budget can't be negative
            }
        }
    return shortage;
}

bool solvePassengers(ant *currentAnt, float& cost)
{
    tourArc *ptTourArc;
    pass *ptPass, **pptPass;
    node *ptNode, *currentNode;
    car *currentCar;
    uint i, j, n;
    float arcCost, costShare, shortage;
    uchar nPickedPassengers = 0;
    ptTourArc = currentAnt->t.arcs;
    /* reset passengers' current budget */
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
        for(i=0, ptPass=ptNode->passengers; i<ptNode->nPass; i++, ptPass++)
            ptPass->currentBudget = ptPass->budget;

    /* reset picked passengers array */
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        (*pptPass)=nullptr;

    /* node traversal */
    cost = shortage = 0.0;
    while(ptTourArc->a != nullptr)
    {
        currentNode = &nodes[ptTourArc->a->row];
        currentCar = ptTourArc->c;
        /* cost to next node */
        arcCost = prData.edgeWeightMatrices[currentCar->n][ptTourArc->a->row][ptTourArc->a->column];

        n = CalculatePassengerProbabilities(ptTourArc);
        /* leave passengers */
        leavePassengers(currentNode, nPickedPassengers);

        if(nPickedPassengers>currentCar->carPassLimit) //replaced with too small car
        {
            return false;
        }

        /* pick passengers */
        pickPassengers(currentCar, currentNode, n, nPickedPassengers);
        costShare = arcCost/(nPickedPassengers+1); //passenger and driver shares cost
        shortage=chargePassengers(costShare); //shortage added to driver's cost 
        cost+=(costShare+shortage);
       
        
        ptTourArc++;
    }
    return true;
}