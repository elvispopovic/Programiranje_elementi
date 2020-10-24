#include "ACO.h"

using namespace std;

carProb* CarSelect(uint n)
{
    register uint i;
    uint select;
    float sum, *ptFloatSel;
    register carProb* ptCarProb;
    if(n<1 || n>prData.nCars)
        return nullptr;
    for(i=0, sum=0.0, ptFloatSel = selProbArray, ptCarProb=carProbArray; i<n; i++, ptCarProb++, ptFloatSel++)
    {
        *ptFloatSel = sum;
        sum+=ptCarProb->p;
    } 
    select = selectFromFreqArray(sum,n);
    return &carProbArray[select];
}


float changeCar(car*& currentCar, car**& antCars, car* newCar, uint currentNodeIndex)
{
    float** matrix;
    float cost = 0.0;

    if(newCar != currentCar)
    {
        if(currentCar != nullptr && currentNodeIndex != currentCar->rentedNode)
        {
            matrix = prData.returnRateMatrices[currentCar->n];
            cost+=matrix[currentNodeIndex][currentCar->rentedNode];
        }
        currentCar = newCar; 
        currentCar->rentedNode = currentNodeIndex;
        currentCar->rented = true;
        *(antCars++) = newCar;
        *antCars = nullptr; //current ant cars limit (last is null)
        cost += prData.rentFee;
    } 
    return cost;
}


void CalculateCarCostSum(car* c)
{
    register uint i;
    uint j;
    float **row;
    register float *col;
    float cost;
    float maxValue;
    /* sum of maximal row weights */
    for(j=0, cost=0.0, row=prData.edgeWeightMatrices[c->n]; j<prData.dim; j++, row++)
    {
        for(i=0, maxValue=0.0, col=*row; i<prData.dim; i++, col++)
            if(*col > maxValue)
                maxValue = *col;
        cost += maxValue;
    }
    /* maximal return value */
    for(j=0, maxValue=0.0, row=prData.returnRateMatrices[c->n]; j<prData.dim; j++, row++)
        for(i=0, col=*row; i<prData.dim; i++, col++)
            if(*col > maxValue)
                maxValue = *col;
    cost += maxValue;
    c->costSum = cost;
    c->invCostSum = 1.0/cost;
}
