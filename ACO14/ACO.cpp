#include "ACO.h"

using namespace std;


pass* passengers = nullptr;
node* nodes = nullptr;
car* cars = nullptr;
ant* ants = nullptr;
probabilityArrays probArrays;
bestPath bPath;


void resetAnts();
bool nodeTraversal(node *startNode, ant *currentAnt);

bool Solution(uint iter, node *startNode)
{
    uint j;
    ant *ptAnt;

    /* add ant loop here */

    resetAnts();
    for(j = 0, ptAnt = ants; j < parData.nAnts; j++, ptAnt++)
    {
        nodeTraversal(startNode, ptAnt); 
    }   
    return true;
}

bool nodeTraversal(node *startNode, ant *currentAnt)
{
    int pickedNode, pickedCar;
    float value;
    node *currentNode;
    car *currentCar;

    /* start node is visited, and counted as passed */
    currentNode = startNode;
    currentCar = cars;

    currentAnt->nodes[currentAnt->nodeCounter].prevNode = nullptr;
    currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;   
    currentAnt->nodesVisited[currentNode->index] = true;

    
    currentAnt->nodes[currentAnt->nodeCounter].carIn = nullptr;
    pickedCar = PickCar(currentAnt, currentNode, currentCar);
    if(pickedCar >= 0)
        currentCar = cars+pickedCar;
    else return -1;
    currentAnt->nodes[currentAnt->nodeCounter].carOut = currentCar;
    currentAnt->carsRented[currentCar->index] = true;
    currentAnt->carPickedNode = currentNode;

    /* for min purpose */
    /* nodes */
    currentAnt->nodes[currentAnt->nodeCounter].choices = 1.0; //predefined, only one choice
    currentAnt->nodes[currentAnt->nodeCounter].prob = 1.0;  // predefined, so probability is 1
    /* cars */
    currentAnt->nodes[currentAnt->nodeCounter].choices *= 3.0*probArrays.n; //three cars
    currentAnt->nodes[currentAnt->nodeCounter].prob *= 1.0*probArrays.probs[probArrays.selected]/probArrays.sum; //prob for chosen car
    /* end of min purpose */

    currentAnt->price = 0.0;
    currentAnt->nodeCounter++;
    do /* node traversal loop starts here */
    {
        pickedNode = PickNode(currentAnt, currentNode, currentCar);
        if(pickedNode >= 0) //can pick at least last node
        {
            /* for nodes - picked node */
            currentAnt->nodes[currentAnt->nodeCounter].choices = probArrays.n;
            currentAnt->nodes[currentAnt->nodeCounter].prob = probArrays.probs[probArrays.selected]/probArrays.sum;


            currentAnt->price += (float)prData.edgeWeightMatrices[currentCar->index][currentNode->index][nodes[pickedNode].index];
            currentAnt->nodes[currentAnt->nodeCounter].prevNode = currentNode;
            currentNode = nodes+pickedNode;
            currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = currentNode;
            currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;
            currentAnt->nodesVisited[pickedNode] = true;      

            currentAnt->nodes[currentAnt->nodeCounter].carIn = currentCar;
            pickedCar = PickCar(currentAnt, currentNode, currentCar);
            
            if(pickedCar >= 0)
            {
                /* for cars - picked cars */
                currentAnt->nodes[currentAnt->nodeCounter].choices *= probArrays.n;
                currentAnt->nodes[currentAnt->nodeCounter].prob *= probArrays.probs[probArrays.selected]/probArrays.sum;
                currentAnt->price += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                currentCar = cars+pickedCar;
                currentAnt->carsRented[currentCar->index] = true;
                currentAnt->carPickedNode = currentNode;
            }
             
            currentAnt->nodes[currentAnt->nodeCounter].carOut = currentCar;
            currentAnt->nodeCounter++;         
        }
        else //no node can be picked so it may be start node (circular) 
        {
            /* check if we have path to last node in matrix */
            value = prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];
            if(value != 0.0 && value < 9999) //can we connect last node and start node?
            {
                currentAnt->price += value;
                currentNode = startNode;
                currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = currentNode;
                currentAnt->price += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                currentCar = nullptr;
                currentAnt->closedPath = true;
            } /* no, so there is not tour */
            else 
            {
                currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = nullptr;
                currentAnt->closedPath = false;
            }
        }
    } while (pickedNode >= 0);
    if(currentAnt->nodeCounter == prData.dim && currentAnt->closedPath == true)
        return true;
    return false;
}

void PheromoneEvaporation()
{
    uint i, j;
    node *ptNode;
    float *ptFloat;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
            if(i!=j)
                *ptFloat = (1.0-parData.rho) * (*ptFloat);
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = (1.0-parData.rho) * (*ptFloat);
    }
}

bool updatePheromones(ant *bestAnt)
{
    uint i;
    float pheroUpdate;
    antNode *ptAntNode;
    if(bestAnt->nodeCounter < prData.dim || bestAnt-> closedPath == false || bestAnt->price < 0.001)
        return false;
    if(bestAnt->price>0.001)
        pheroUpdate = 1.0/bestAnt->price;
    else
        return false;
    /* use non-opt or opt path */
    if(bestAnt->bestOptPrice >= bestAnt->price)
    {   
        pheroUpdate = 1.0/bestAnt->price;
        ptAntNode=bestAnt->nodes;
    }
    else
    {
        pheroUpdate = 1.0/bestAnt->bestOptPrice;
        ptAntNode=bestAnt->bestOptNodes;
    }
    for(i=0; i<bestAnt->nodeCounter-1; i++, ptAntNode++)
    {
        ptAntNode->curNode->pheroNeighbours[ptAntNode->nextNode->index]+=pheroUpdate;
        ptAntNode->curNode->pheroCars[ptAntNode->carOut->index]+=pheroUpdate;
    }

    return true;
}

void limitPheromoneTraces()
{
    uint i, j;
    node *ptNode;
    float *ptFloat;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
            if(i!=j)
            {
                if(*ptFloat > bPath.pheroMax)
                    *ptFloat = bPath.pheroMax;
                if(*ptFloat < bPath.pheroMin)
                    *ptFloat = bPath.pheroMin;
            }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
        {
            if(*ptFloat > bPath.pheroMax)
                *ptFloat = bPath.pheroMax;
            if(*ptFloat < bPath.pheroMin)
                *ptFloat = bPath.pheroMin;
        }
    }
}

bool updateBestPath(uint iteration, ant *bestAnt)
{
    uint i;
    float bestPrice = numeric_limits<float>::max();
    antNode *ptAntNode1, *ptAntNode2;
    if(bestAnt == nullptr || bestAnt->nodeCounter < prData.dim || bestAnt->closedPath == false)
        return false; //no updates
    if(bestAnt->price < bestPrice) //try non-opt price
        bestPrice = bestAnt->price;
    if(bestAnt->bestOptPrice < bestPrice) //try opt price
        bestPrice = bestAnt->bestOptPrice;
    if(bestPrice >= bPath.price || bestPrice >= bPath.optPrice)
        return false; //no better price
    bPath.iteration = iteration;

    /* non-opt path */
    bPath.nodeCounter = bestAnt->nodeCounter;
    bPath.price = bestAnt->price;
    for(i=0, ptAntNode1=bPath.nodes, ptAntNode2=bestAnt->nodes; i<bPath.nodeCounter; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode1, ptAntNode2, sizeof(antNode));

    /* opt path */
    bPath.optNodeCounter = bestAnt->optNodeCounter;
    bPath.optPrice = bestAnt->bestOptPrice;
    if(bPath.optNodeCounter == bPath.nodeCounter)
        for(i=0, ptAntNode1=bPath.optNodes, ptAntNode2=bestAnt->bestOptNodes; i<bPath.optNodeCounter; i++, ptAntNode1++, ptAntNode2++)
            memcpy(ptAntNode1, ptAntNode2, sizeof(antNode));

    return true;      
}


float calculatePathCost(antNode *nodes, uint nodeCounter)
{
    uint i;
    antNode *ptAntNode;
    node *carPickNode;
    float value, price = 0.0;
    if(nodeCounter == 0)
        return price;
    carPickNode=nodes->curNode;
    for(i=1, ptAntNode=nodes+1; i<nodeCounter; i++, ptAntNode++)
    {
        price += prData.edgeWeightMatrices[ptAntNode->carIn->index][ptAntNode->prevNode->index][ptAntNode->curNode->index];
        if(ptAntNode->carOut->index != ptAntNode->carIn->index)
        {
            price += prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index];
            carPickNode = ptAntNode->curNode;
        }
    }
    value = prData.edgeWeightMatrices[(ptAntNode-1)->carOut->index][(ptAntNode-1)->curNode->index][nodes->curNode->index];
    price += value;
    if(value != 0.0 && value < 9999)
        price += prData.returnRateMatrices[(ptAntNode-1)->carOut->index][nodes->curNode->index][carPickNode->index];
    return price;
}


ant* findBestAnt()
{
    uint i;
    ant* ptAnt, *bestAnt=nullptr;
    float minPrice = numeric_limits<float>::max();
    for(i=0, ptAnt=ants; i<parData.nAnts; i++, ptAnt++)
    {
        if(ptAnt->nodeCounter == prData.dim && ptAnt->closedPath == true)
        {
            if(ptAnt->price < minPrice) //try non opt price
            {
                minPrice = ptAnt->price;
                bestAnt = ptAnt;
            }
            if(ptAnt->bestOptPrice < minPrice) //try opt price
            {
                minPrice = ptAnt->bestOptPrice;
                bestAnt = ptAnt;
            }
        }
    }
    return bestAnt;
}

void resetAnts()
{
    uint i, j;
    ant *ptAnt;
    bool *ptBool;
    for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
    {
        ptAnt->price = 0.0;
        ptAnt->bestOptPrice = numeric_limits<float>::max();
        ptAnt->nodeCounter = 0;
        ptAnt->optNodeCounter = 0;
        for(i=0, ptBool=ptAnt->nodesVisited; i<prData.dim; i++, ptBool++)
            *ptBool = false;
        for(i=0, ptBool=ptAnt->carsRented; i<prData.nCars; i++, ptBool++)
            *ptBool = false;
    }
}



