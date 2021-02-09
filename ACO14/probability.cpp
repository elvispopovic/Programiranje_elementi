#include "ACO.h"

using namespace std;

mt19937* mersenneGenerator = nullptr;



/* pick from cumulative probability aaray */
uint selectFromFreqArray(float sum, uint n, float *probabilities)
{
    float l, r;
    uint left=0;
    uint right = n; //probabilities from 0 to n-1, right limit on n
    uint center =(left+right)>>1;
    l = nextafter(sum, numeric_limits<float>::max()); //upper closed interval limit
    uniform_real_distribution<float> distribution((float)0.0,l);
    r = distribution(*mersenneGenerator);
    while(left<center)
    {
        if(r >= *(probabilities+center))
            left = center;
        else
            right = center;
        center = (left+right)>>1;
    }
    return center;
}

int PickNode(ant *currentAnt, node *currentNode, car *currentCar)
{
    uint i, nNeighbours;
    uint *ptUint;
    float sum, p=0.0;
    float eta, tau;
    float *ptFloat, *ptFloat2, *ptFloat3;
    bool *ptBool;
    /* edgeWeightMatrices[car][node][neighbour] */
    for(i=0, nNeighbours=0, ptBool = currentAnt->nodesVisited, 
        ptFloat3 = currentNode->pheroNeighbours, 
        ptFloat2=prData.edgeWeightMatrices[currentCar->index][currentNode->index],
        ptUint = probArrays.indices, ptFloat = probArrays.probabilities, sum=0.0;
        i<prData.dim; 
        i++, ptFloat2++, ptFloat3++, ptBool++)
    {
        if(*ptFloat2 != 0.0 && *ptFloat2 < 9999 && *ptBool == false) //can be diagonal but can be no link also
        {
            *(ptUint++) = i;        //freq array index
            eta = 1.0/(*ptFloat2);
            tau = *ptFloat3;
            p = pow(eta, parData.alpha) * pow(tau, parData.beta);
            *(ptFloat++) = sum;     //freq array probability (cumulative)
            sum += p;               //left shift, p0 to 0, sum after
            nNeighbours++;
        }
    }
    if(nNeighbours == 0)
        return -1;
    else if(nNeighbours == 1)
        return probArrays.indices[0];
    i = selectFromFreqArray(sum, nNeighbours, probArrays.probabilities);
    return probArrays.indices[i];
}

int PickCar(ant *currentAnt, node *currentNode, car *currentCar)
{
    uint i, j, counter, nCars;
    float avgReturn, avgNeighbour, eta, tau, p, sum;
    uint *ptUint;
    float *ptFloat, *ptFloat2, *ptFloat3;
    car *ptCar;
    int picked = 0;
    if(prData.nCars <= 1) //at least one car exists, car index 0
        return picked;
    for(j=0, nCars=0, ptCar=cars, ptFloat3=currentNode->pheroCars,
         ptUint = probArrays.indices, ptFloat = probArrays.probabilities, sum=0.0; 
        j<prData.nCars; 
        j++,ptCar++, ptFloat3++)
    {
        if(currentAnt->carsRented[ptCar->index]==false)
        {
            /* average return */
            if(ptCar->index == currentCar->index) //no return cost for current car
                avgReturn = 0.0;
            else
            {
                for(i=0, avgReturn=0.0, counter=0; i<prData.dim; i++)
                    if(currentNode->index != i)
                    {
                        avgReturn+=prData.returnRateMatrices[ptCar->index][i][currentNode->index];
                        counter++;
                    }
                if(counter>0)
                    avgReturn/=counter;
            }
            /* average forward */
            for(i=0, avgNeighbour=0.0, counter=0, 
                ptFloat2=prData.edgeWeightMatrices[ptCar->index][currentNode->index]; 
                i<prData.dim; i++, ptFloat2++)
                if(currentNode->index != i && *ptFloat2 != 0.0 && *ptFloat2 < 9999)
                {
                     avgNeighbour+=(*ptFloat2);
                     counter++;
                }
            if(counter>0)
                avgNeighbour/=counter;
            *(ptUint++)=j;
            eta = 1.0/(avgNeighbour+avgReturn);
            tau = *ptFloat3;
            p = pow(eta, parData.alpha) * pow(tau, parData.beta);
            *(ptFloat++) = sum;
            sum+=p;
            nCars++;
        }
    }
    if(nCars==0)
        return -1;
    else if(nCars==1)
        return probArrays.indices[0];
    i = selectFromFreqArray(sum, nCars, probArrays.probabilities);
    return probArrays.indices[i];
}
