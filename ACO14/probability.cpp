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

int CalculateNodeProbabilities(ant *currentAnt, node *currentNode, car *currentCar)
{
    uint i, nNeighbours;
    uint *ptUint;
    float sum, p=0.0;
    float *ptFloat, *ptFloat2;
    bool *ptBool;
    /* edgeWeightMatrices[car][node][neighbour] */
    for(i=0, nNeighbours=0, ptFloat2=prData.edgeWeightMatrices[currentCar->index][currentNode->index], 
        ptBool = currentAnt->nodeVisited, 
        ptUint = currentAnt->nodeCandidatesIndices, ptFloat = currentAnt->nodeCandidateProbs, sum=0.0;
        i<prData.dim; 
        i++, ptFloat2++, ptBool++)
    {
        if(*ptFloat2 != 0.0 && *ptFloat2 < 9999 && *ptBool == false)
        {
            *(ptUint++) = i;
            p = 1.0/(*ptFloat2);
            *(ptFloat++) = sum;
            sum += p;
            nNeighbours++;
        }
    }
    if(nNeighbours == 0)
        return -1;
    else if(nNeighbours == 1)
        return currentAnt->nodeCandidatesIndices[0];
    i = selectFromFreqArray(sum, nNeighbours, currentAnt->nodeCandidateProbs);
    return currentAnt->nodeCandidatesIndices[i];
}
