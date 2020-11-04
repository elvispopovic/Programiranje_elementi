#include "ACO.h"

using namespace std;

rollback rollBack;

/* find passenger destination node distance */
uint findRadius(tourArc* startTourNode, node *destTourNode)
{
    tourArc *ptTourArc;
    uint radius;
    for(ptTourArc = startTourNode, radius=0; ptTourArc->a != nullptr; ptTourArc++, radius++)
    {
        if(destTourNode->n == nodes[ptTourArc->a->row].n)
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
    /* raise pheromone */
    passProbArray[select].pa->tau *= (1.0+RHO);
    return &passProbArray[select];
}

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
        while(j<maxCarPassLimit-1 && carPassArray[j] != nullptr) //jump over non empty elements
            j++;
        if(carPassArray[j]==nullptr)
        {
            carPassArray[j]=ptPassProb->pa;
            nPickedPassengers++;
            *(pptPass1++) = ptPassProb->pa;
            *pptPass1 = nullptr;
        }
    }
    /* fill ant's tour arc's passenger array - no holes */
    for(i=0, pptPass1 = carPassArray, pptPass2 = antTourArc->p; i<maxCarPassLimit; i++, pptPass1++)
        if((*pptPass1)!=nullptr)
            (*pptPass2++)=(*pptPass1);
    (*pptPass2)=nullptr;
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

    rollBack.lastCost = currentCost;
    rollBack.counter = 0; //reset rollback counter
    shortage = 0;
    #ifdef ACO_VERBOSE
    cout << ">>> Zero passenger point at " << nodes[ta->a->row].name << ", shortage: " << shortage 
                << ", cost: " << currentCost << endl;
    #endif
}

tourArc* rollbackToBasePoint(ant *currentAnt, uchar &nPickedPassengers, float& cost, float& shortage)
{
    uint i;
    tourArc *ptTourArc1, *ptTourArc2;
    pass **pptPass, **pptPass1, **pptPass2;
    ptTourArc1 = rollBack.lastTourArc;
    cost = rollBack.lastCost;
    nPickedPassengers = 0;
    for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
        if(*pptPass != nullptr)
            (*pptPass)=nullptr;

    for(i=0,pptPass1=currentAnt->t.passList, pptPass2=rollBack.lastPassList; i<=prData.nPass; pptPass1++, pptPass2++, i++)
        *pptPass1 = *pptPass2;

    /* restore passenger's budgets in all nodes after rollbacked node */
    for(ptTourArc2=ptTourArc1; ptTourArc2->a != nullptr; ptTourArc2++)
        for(pptPass=ptTourArc2->p; (*pptPass) != nullptr; pptPass++)
            (*pptPass)->currentBudget = (*pptPass)->budget;
    /* reset passenger array in ant path rollback node */
    ptTourArc1->p[0]=nullptr;

    /* increase rollback counter */
    rollBack.counter++;
    (currentAnt->t.rollbacks)++;
    shortage = 0;
    rollBack.rollbacked = true;
    if(rollBack.counter == PASS_ITER)
    {
        rollBack.dontPick = true;
        for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
            (*pptPass)=nullptr;
    }
    return ptTourArc1;
}

/*
* Solve passenger pickups on known tour
*/
bool solvePassengers(ant *currentAnt, float& cost)
{
    tourArc *ptTourArc;
    car *ptCurrentCar;
    pass *ptPass, **pptPass;
    node *ptNode;
    uint i, j, radius;
    ushort k;
    bool result;

    /* there is no passengers at all - make bypass with basic calculations */
    if(prData.nPass == 0)
    {
        for(ptTourArc = currentAnt->t.arcs, cost =0.0; ptTourArc->a!=nullptr; ptTourArc++)
        {
            ptCurrentCar = ptTourArc->c;
            ptTourArc->cost = prData.edgeWeightMatrices[ptCurrentCar->n][ptTourArc->a->row][ptTourArc->a->column];
            cost += ptTourArc->cost;
        }
        return true;
    }
    
    /* reset passengers' current budget and pheromone */
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
        for(i=0, ptPass=ptNode->passengers; i<ptNode->nPass; i++, ptPass++)
        {
            ptPass->currentBudget = ptPass->budget;
            radius = findRadius(currentAnt->t.arcs, ptPass->destinationNode);
            if(radius == numeric_limits<uint>::max())
                ptPass->tau = TAU_0;
            else
                ptPass->tau = ptPass->currentBudget/radius;
        }

    /* reset node passengers in all arcs for that tour */
    for(i=0, ptTourArc=currentAnt->t.arcs; i<prData.dim+1; i++, ptTourArc++)
    {
        ptTourArc->cost = 0.0; //arc cost to zero
        /* do not reset ptTourArc->carReturnCost 
        *  for it is set in path finder
        */
        if(i==prData.dim)
            ptTourArc->p = nullptr; //limiter arc - null to pass array
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
    return result;
}

/*
* Tour traversal
*/

bool nodeTraversal2(ant* currentAnt, float& cost)
{
    uint i, n, radius;
    tourArc *ptTourArc;
    arc* ptCurrentArc;
    pass **pptPass;
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
        /* leave passengers */
        leavePassengers(ptTourArc, nPickedPassengers);
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
            else //no shortage, so all passengers in the car raise pheromone
            {
                for(i=0, pptPass=carPassArray; i<maxCarPassLimit; i++, pptPass++)
                    if(*pptPass != nullptr)
                    {
                        radius = findRadius(ptTourArc, (*pptPass)->destinationNode);
                        if(radius == numeric_limits<uint>::max())
                            (*pptPass)->tau = TAU_0;
                        else
                            (*pptPass)->tau += (*pptPass)->budget/radius;
                    }
            }
                       
        }
        else //have shortage
        {
            ptTourArc = rollbackToBasePoint(currentAnt, nPickedPassengers, cost, shortage);
            ptCurrentArc = ptTourArc->a; //restore arc
            ptCurrentCar = ptTourArc->c; //restore car
            continue;
        }
        n = CalculatePassengerProbabilities(ptTourArc);
        
        if(nPickedPassengers>ptCurrentCar->carPassLimit) //replaced with too small car
        {
            #ifdef ACO_VERBOSE
            cout << "!!! New car is too small for picked passengers." << endl;
            #endif
            return false;
        }

        /* pick passengers, gradually expell one by one trying to improve solution */
        if(rollBack.dontPick == false)
        {
            for(i=1; i<4; i++)
                if(rollBack.counter > i*PASS_ITER/4 && n>1)
                    n--;
            pickPassengers(currentAnt, ptCurrentCar, ptTourArc, n, nPickedPassengers);
        }
        else
        {
            ptTourArc->p[0]=nullptr;
            rollBack.dontPick = false;
        }

        costShare = arcCost/(nPickedPassengers+1); //passenger and driver shares cost
        shortage=chargePassengers(costShare); //shortage added to driver's cost 
        cost+=(costShare+shortage);
        ptTourArc->cost = (costShare+shortage);

        ptTourArc++;
        ptCurrentArc = ptTourArc->a; //restore arc
        ptCurrentCar = ptTourArc->c; //restore car
    }
    return true;
}