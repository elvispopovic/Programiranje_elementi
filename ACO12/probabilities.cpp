#include "ACO.h"

using namespace std;

/* creates probability distribution for arcs in node and store it in arcProbArray (global array) */
uint CalculateArcProbabilities(arc* arcs, car* currentCar)
{
    register arc* ptArc;
    node* ptNode;
    arcProb* ptArcProb;
    register uint i;
    uint counter;
    if(arcs == nullptr)
        return 0;
    counter = 0;
    float eta;
    if(currentCar == nullptr)
        return 0;
    ptArcProb = arcProbArray;

    for(i=0, ptArc=arcs; i<prData.dim; i++,ptArc++)
    {
        ptNode = &nodes[ptArc->column];
        if(ptArc->row != ptArc->column && ptNode->v == false)
        {
            // eta = 1/d_i,j
            eta = 1.0/prData.edgeWeightMatrices[currentCar->n][ptArc->row][ptArc->column];
            ptArcProb->p = pow(ptArc->tau, ALPHA) * pow(eta, BETA);
            ptArcProb->a = ptArc;
            ptArcProb++;
            counter++;
        }
    }
    return counter;
}

/* creates probability distribution for cars in node and store it in carProbArray (global array) */
uint CalculateCarProbabilities(car* currentCar, node* currentNode, uint remainNodes, uchar carPersist)
{
    register uint i;
    uint counter;
    float eta, tau, remainToDim = (float)remainNodes*invDim;
    register car* ptCar;
    carProb* ptProb = carProbArray;
    counter = 0;
    for(i=0, ptCar=cars; i<prData.nCars; i++, ptCar++)
    {
        if(ptCar != currentCar && ptCar->rented == true)
            continue;
        /* Heuristic part
        * eta = (1/sum(max_row_cost+max_return_fee) * remain_nodes/number_of_nodes) * (car_passenger_limit/max_passenger_limit)
        */
        eta = ptCar->invCostSum * remainToDim * (float)(prData.carPassLimit[ptCar->n])* invMaxCarPassLimit;
        tau = /*ptCar->tau * */ currentNode->tau_cars[ptCar->n];
        ptProb->p = pow(tau, ALPHA)* //pheromone part
                    pow(eta , BETA ); //heuristic part     
        if(ptCar == currentCar)
            ptProb->p = ptProb->p * carPersist; // force current car
        ptProb->c = ptCar;
        ptProb++;
        counter++;
    }
    return counter;
}



/* select element from probability frequency array using divide and conquer */
uint selectFromFreqArray(float sum, uint n)
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
        if(r >= selProbArray[center])
            left = center;
        else
            right = center;
        center = (left+right)>>1;
    }
    return center;
}