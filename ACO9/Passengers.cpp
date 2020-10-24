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
        cout << "Pass " << passProbArray[i].pa->name << ", p = " << passProbArray[i].p << ", ";
        while(j<maxCarPassLimit-1 && carPassArray[j] != nullptr) //jump over non empty elements
            j++;
        if(carPassArray[j]==nullptr)
        {
            carPassArray[j]=ptPassProb->pa;
            nPickedPassengers++;
            cout << ", passenger " << ptPassProb->pa->name << " enters car.";
        }
    }
    cout << endl;
}

void leavePassengers(node* currentNode, uchar& nPickedPassengers)
{
    uchar i;
    pass** pptPass;
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if((*pptPass)!=nullptr && (*pptPass)->destinationNode->n == currentNode->n)
        {
            cout << "Passenger " << (*pptPass)->name << " has left car. Current budget: " << (*pptPass)->currentBudget << endl;
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
    cout << "Shortage: " << shortage << endl;
    return shortage;
}

bool solvePassengers(ant *currentAnt, float& cost)
{
    tourArc *ptTourArc;
    pass *ptPass, **pptPass;
    node *ptNode, *currentNode, *destNode;
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

        cout << "Node: " << (nodes[ptTourArc->a->row].name) << ", cost: " << arcCost << ", passengers: " << (nodes[ptTourArc->a->row].nPass) << ":" << endl; 
        cout << "Current car: " << currentCar->name << ", capacity: " << (int)(currentCar->carPassLimit) << endl;
        for(i=0, ptPass = currentNode->passengers; i<nodes[ptTourArc->a->row].nPass; i++, ptPass++)
        {
            destNode = ptPass->destinationNode;
            cout << "Pass " << ptPass->name << ", budget: " << ptPass->budget << ", current budget: " << ptPass->currentBudget << 
            ", dest node: " << destNode->name << ", radius: " << findRadius(ptTourArc, destNode) << endl;
        }
        n = CalculatePassengerProbabilities(ptTourArc);
        /* leave passengers */
        leavePassengers(currentNode, nPickedPassengers);
        cout << "Picked passengers (after leaving): " << (int)nPickedPassengers << "/" << (int)(currentCar->carPassLimit) << endl;
        if(nPickedPassengers==0)
        {
            cout << ">>> Zero passenger point at " << nodes[ptTourArc->a->row].name << ", shortage: " << shortage << endl;
        }

        if(nPickedPassengers>currentCar->carPassLimit) //replaced with too small car
        {
            cout << "!!! New car is too small for picked passengers." << endl;
            return false;
        }

        /* pick passengers */
        pickPassengers(currentCar, currentNode, n, nPickedPassengers);
        costShare = arcCost/(nPickedPassengers+1); //passenger and driver shares cost
        cout << "Picked passengers (after entering): " << (int)nPickedPassengers << "/" << (int)(currentCar->carPassLimit) <<
        ", shared cost: " << costShare << endl;
        shortage=chargePassengers(costShare); //shortage added to driver's cost 
        cost+=(costShare+shortage);
        cout << "Cost after passenger charging and shortage calculating: " << (costShare+shortage) << ", total cost: " << cost << endl;
        
        
        ptTourArc++;
        cout << endl;
    }
    cout << endl; 
    return true;
}