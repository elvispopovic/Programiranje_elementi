#include "ACO.h"

using namespace std;

rollback rollBack;

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
        ptProb->p = pow(ptPass->tau, ALPHA)*
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

/* <<<<<<<<<<<<< dodati putnike i u cvorove mrava ???*/
/* pick possible passengers */
void pickPassengers(ant *currentAnt, car *currentCar, tourArc* antTourArc, uchar nPassengersToPick, uchar& nPickedPassengers)
{
    uchar i, j;
    passProb *ptPassProb;
    pass **pptPass1, **pptPass2;
    pptPass1 = currentAnt->t.passList;
    while(*pptPass1!=nullptr)
        pptPass1++;
    for(i=0, j=0, ptPassProb=passProbArray; 
        i<nPassengersToPick && nPickedPassengers<currentCar->carPassLimit; i++, ptPassProb++)
    {
        cout << "Pass " << passProbArray[i].pa->name << ", p = " << passProbArray[i].p << ", ";
        while(j<maxCarPassLimit-1 && carPassArray[j] != nullptr) //jump over non empty elements
            j++;
        if(carPassArray[j]==nullptr)
        {
            carPassArray[j]=ptPassProb->pa;
            nPickedPassengers++;
            *(pptPass1++) = ptPassProb->pa;
            *pptPass1 = nullptr;
            cout << "passenger " << ptPassProb->pa->name << " enters car." << endl;
        }
    }
    for(i=0, pptPass1 = carPassArray, pptPass2 = antTourArc->p; i<maxCarPassLimit; i++, pptPass1++)
        if((*pptPass1)!=nullptr)
            (*pptPass2++)=(*pptPass1);
    (*pptPass2)=nullptr;
    cout << endl;
}

/* passengers leave car */
void leavePassengers(tourArc* antTourArc, uchar& nPickedPassengers)
{
    uchar i;
    pass** pptPass;
    node* currentNode = &nodes[antTourArc->a->row];
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if((*pptPass)!=nullptr && (*pptPass)->destinationNode->n == currentNode->n)
        {
            cout << "Passenger " << (*pptPass)->name << " has left car. Current budget: " << (*pptPass)->currentBudget << endl;
            *pptPass = nullptr;
            if(nPickedPassengers>0)
                nPickedPassengers--;
            
        }
}

/* press charge of shared cost to passengers */
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

/*  */
void setBasePoint(tourArc* ta, pass** passList, float currentCost, float& shortage)
{
    ushort j;
    pass **pptPass1, **pptPass2;
    rollBack.lastTourArc = ta;

    /* ant passenger list to rollback passenger list */
    for(j=0,pptPass1=passList, pptPass2=rollBack.lastPassList; j<prData.nPass+1; pptPass1++, pptPass2++, j++)
        *pptPass2 = *pptPass1;

    cout << "Base point passengers: ";
    for(j=0, pptPass1 = rollBack.lastPassList; j<prData.nPass && *pptPass1!=nullptr; j++, pptPass1++)
        cout << (*pptPass1)->name << ", ";
    cout << endl;

    rollBack.lastCost = currentCost;
    rollBack.counter = 0;
    shortage = 0;
    cout << ">>> Zero passenger point at " << nodes[ta->a->row].name << ", shortage: " << shortage 
                << ", cost: " << currentCost << endl;
}

tourArc* rollbackToBasePoint(pass** antPassList, uchar &nPickedPassengers, float& cost, float& shortage)
{
    uint i;
    tourArc* ptTourArc;
    pass **pptPass, **pptPass1, **pptPass2;
    ptTourArc = rollBack.lastTourArc;
    cost = rollBack.lastCost;
    nPickedPassengers = 0;
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if(*pptPass != nullptr)
        {
            (*pptPass)->tau += (*pptPass)->budget/shortage;
            (*pptPass)=nullptr;
        }
    for(i=0,pptPass1=antPassList, pptPass2=rollBack.lastPassList; i<=prData.nPass; pptPass1++, pptPass2++, i++)
        *pptPass1 = *pptPass2;

    rollBack.counter++;
    shortage = 0;
    rollBack.rollbacked = true;
    cout << "Rollback to " << nodes[rollBack.lastTourArc->a->row].name << ", and car " << rollBack.lastTourArc->c->name << 
    ", counter: " << rollBack.counter << ", cost: " << cost << endl;
    if(rollBack.counter == PASS_ITER)
    {
        rollBack.dontPick = true;
        for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
            (*pptPass)=nullptr;
    }
    return ptTourArc;
}

/*
* Solve passenger pickups on known tour
*/
bool solvePassengers(ant *currentAnt, float& cost)
{
    tourArc *ptTourArc;
    pass *ptPass, **pptPass;
    node *ptNode;
    uint i, j;
    ushort k;
    bool result;
    
    /* reset passengers' current budget and pheromone */
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
        for(i=0, ptPass=ptNode->passengers; i<ptNode->nPass; i++, ptPass++)
        {
            ptPass->currentBudget = ptPass->budget;
            ptPass->tau = TAU_0;
        }

    /* reset node passengers in all arcs for that tour */
    for(i=0, ptTourArc=currentAnt->t.arcs; i<prData.dim+1; i++, ptTourArc++)
    {
        ptTourArc->cost = 0.0;
        /* do not reset ptTourArc->carReturnCost 
        *  for it is set in path finder
        */
        if(i==prData.dim)
            ptTourArc->p = nullptr;
        else
            for(k=0, pptPass=ptTourArc->p; k<maxCarPassLimit+1; k++, pptPass++)
                (*pptPass) = nullptr;
    }

    /* reset picked passengers array */
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        (*pptPass)=nullptr;

    /* reset ant pass array */
    for(k=0, pptPass=currentAnt->t.passList; k<prData.nPass+1; k++, pptPass++)
        (*pptPass)=nullptr;

    /* node traversal */
    ptTourArc = currentAnt->t.arcs; //arcs for that tour
    rollBack.lastTourArc = ptTourArc;
    rollBack.lastCost = 0.0;
    rollBack.dontPick = false;
    rollBack.counter = 0;
    rollBack.rollbacked = false;
    result = nodeTraversal2(currentAnt, cost);
    cout << "Ant tour pass status:" << endl;
    cout << "Node: " << nodes[ptTourArc->a->row].name << endl;
    for(pptPass = ptTourArc->p; (*pptPass)!=nullptr; pptPass++)
        cout << (*pptPass)->name << ", ";
    cout << endl;
    return result;
}

/*
* Tour traversal
*/

bool nodeTraversal2(ant* currentAnt, float& cost)
{
    uint i, n;
    ushort k;
    tourArc *ptTourArc;
    arc* ptCurrentArc;
    pass *ptPass, **pptPass;
    node *destNode;
    car *ptCurrentCar;
    float arcCost, costShare, shortage;
    uchar nPickedPassengers = 0;

    ptTourArc = currentAnt->t.arcs; //arcs for that tour
    ptCurrentArc = ptTourArc->a;
    ptCurrentCar = ptTourArc->c;
    shortage = 0.0;
    cost = 0.0;
    while(ptCurrentArc != nullptr)
    {
        //currentNode = &nodes[ptTourArc->a->row];
        
        /* cost to next node */
        arcCost = prData.edgeWeightMatrices[ptCurrentCar->n][ptTourArc->a->row][ptTourArc->a->column];

        cout << "Node: " << (nodes[ptTourArc->a->row].name) << ", cost to " << (nodes[ptTourArc->a->column].name) << ": " << arcCost << ", passengers: " 
        << (nodes[ptTourArc->a->row].nPass) << ":" << endl; 
        cout << "Current car: " << ptCurrentCar->name << ", capacity: " << (int)(ptCurrentCar->carPassLimit) << endl;
        /* leave passengers */
        leavePassengers(ptTourArc, nPickedPassengers);
        cout << "Picked passengers (after leaving): " << (int)nPickedPassengers << "/" << (int)(ptCurrentCar->carPassLimit) << endl;
        for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        {
            if(*pptPass!=nullptr)
                cout << (*pptPass)->name << ", cur budget: " << (*pptPass)->currentBudget << endl;
        }
        cout << "Current ant passengers:" << endl;
        for(k=0,pptPass=currentAnt->t.passList; k<prData.nPass+1; pptPass++, k++)
            if(*pptPass != nullptr)
                cout << (*pptPass)->name << ", ";
        cout << endl;

        /* base point - no shortage and no passengers */
        if(shortage == 0.0)
        {
            if(nPickedPassengers==0)
            {
                if(rollBack.rollbacked == false) //avoid double setting
                {
                    setBasePoint(ptTourArc, currentAnt->t.passList, cost, shortage);
                }
                else
                    rollBack.rollbacked = false; 
            }            
        }
        else 
        {
            ptTourArc = rollbackToBasePoint(currentAnt->t.passList, nPickedPassengers, cost, shortage);
            continue;
        }
        for(i=0, ptPass = (&nodes[ptCurrentArc->row])->passengers; i<nodes[ptCurrentArc->row].nPass; i++, ptPass++)
        {
            destNode = ptPass->destinationNode;
            cout << "Pass " << ptPass->name << ", budget: " << ptPass->budget << ", current budget: " << 
            ptPass->currentBudget << ", tau: " << ptPass->tau <<
            ", dest node: " << destNode->name << ", radius: " << findRadius(ptTourArc, destNode) << endl;
        }
        n = CalculatePassengerProbabilities(ptTourArc);
        

        if(nPickedPassengers>ptCurrentCar->carPassLimit) //replaced with too small car
        {
            cout << "!!! New car is too small for picked passengers." << endl;
            return false;
        }

        /* pick passengers */
        if(rollBack.dontPick == false)
            pickPassengers(currentAnt, ptCurrentCar, ptTourArc, n, nPickedPassengers);
        else
        {
            for(pptPass=ptTourArc->p; (*pptPass)!=nullptr; pptPass++)
                (*pptPass)=nullptr;
            cout << "Don't pick active." <<endl;
            rollBack.dontPick = false;
        }

        costShare = arcCost/(nPickedPassengers+1); //passenger and driver shares cost
        cout << "Picked passengers (after entering): " << (int)nPickedPassengers << "/" << (int)(ptCurrentCar->carPassLimit) <<
        ", shared cost: " << costShare << endl;
        shortage=chargePassengers(costShare); //shortage added to driver's cost 
        cost+=(costShare+shortage);
        ptTourArc->cost = (costShare+shortage);
        cout << "Cost after passenger charging and shortage calculating: " << (costShare+shortage) << ", total cost: " << cost << endl;

        ptTourArc++;
        ptCurrentArc = ptTourArc->a;
        ptCurrentCar = ptTourArc->c;
        cout << endl;
    }
    cout << endl; 
    return true;
}