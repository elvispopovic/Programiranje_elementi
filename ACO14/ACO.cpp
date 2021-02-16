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
    currentAnt->nodeCounter++;


    /* reset ant price */
     currentAnt->price = 0.0;

    do /* node traversal loop starts here */
    {
        pickedNode = PickNode(currentAnt, currentNode, currentCar);
        if(pickedNode >= 0)
        {
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
                currentAnt->price += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                currentCar = cars+pickedCar;
                currentAnt->carsRented[currentCar->index] = true;
                currentAnt->carPickedNode = currentNode;
            }
            currentAnt->nodes[currentAnt->nodeCounter].carOut = currentCar;
            currentAnt->nodeCounter++;
                  
        }
        else //try to connect to start node (if link exists)
        {
            value = prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];
            if(value != 0.0 && value < 9999)
            {
                currentAnt->price += (float)prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];
                currentAnt->nodes[currentAnt->nodeCounter].prevNode = currentNode;
                currentNode = startNode;
                currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = currentNode;
                currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;
                currentAnt->nodes[currentAnt->nodeCounter].nextNode = currentAnt->nodes->curNode;

                currentAnt->nodes[currentAnt->nodeCounter].carIn = currentCar;
                pickedCar = PickCar(currentAnt, currentNode, currentCar);
                if(pickedCar >= 0)
                {
                    currentAnt->price += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                    currentCar = cars+pickedCar;
                    currentAnt->carsRented[currentCar->index] = true;
                    currentAnt->carPickedNode = currentNode;
                }
                currentAnt->nodes[currentAnt->nodeCounter].carOut = currentCar;
                currentAnt->nodeCounter++;
            }
        }
    } while (pickedNode >= 0);
    if(currentAnt->nodeCounter == prData.dim+1)
    {
        currentAnt->price += prData.returnRateMatrices[currentCar->index][currentAnt->nodes->curNode->index][currentAnt->carPickedNode->index];
        return true;
    }
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
    if(bestAnt->nodeCounter <= prData.dim)
        return false;
    if(bestAnt->price>0.001)
        pheroUpdate = 1.0/bestAnt->price;
    else
        return false;
    
    for(i=0, ptAntNode=bestAnt->nodes; i<bestAnt->nodeCounter-1; i++, ptAntNode++)
    {
        ptAntNode->curNode->pheroNeighbours[ptAntNode->nextNode->index]+=pheroUpdate;
        ptAntNode->curNode->pheroCars[ptAntNode->carOut->index]+=pheroUpdate;
    }

    return true;
}

void updateBestPath(uint iteration, uint bestAntIndex)
{
    uint i;
    ant *bestAnt;
    antNode *ptAntNode1, *ptAntNode2;
    bestAnt = ants+bestAntIndex;
    if(bestAnt == nullptr || bestAnt->nodeCounter < prData.dim || bestAnt->price >= bPath.price)
        return;
    bPath.iteration = iteration;
    bPath.antIndex = bestAntIndex;
    bPath.nodeCounter = prData.dim;
    bPath.price = bestAnt->price;
    for(i=0, ptAntNode1=bPath.nodes, ptAntNode2=bestAnt->nodes; i<prData.dim; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode1, ptAntNode2, sizeof(antNode));
}

/* redundant calculation */
float calculatePathCost()
{
    uint i;
    antNode *ptAntNode;
    node *carPickNode;
    float price = 0.0;
    if(bPath.nodeCounter == 0)
        return price;
    for(i=0, ptAntNode=bPath.nodes, carPickNode=bPath.nodes->curNode; i<bPath.nodeCounter; i++, ptAntNode++)
    {
        price += prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index];
        if(i>0 && ptAntNode->carOut->index != ptAntNode->carIn->index)
        {
            price+=prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index];
            carPickNode = ptAntNode->curNode;
        }
    }
    price+=prData.returnRateMatrices[(ptAntNode-1)->carOut->index][bPath.nodes->curNode->index][carPickNode->index];
    return price;
}

int findBestAnt()
{
    int result = -1;
    uint i;
    ant* ptAnt;
    float minPrice = numeric_limits<float>::max();
    for(i=0, ptAnt=ants; i<parData.nAnts; i++, ptAnt++)
        if(ptAnt->nodeCounter == prData.dim+1 && ptAnt->price < minPrice)
        {
            minPrice = ptAnt->price;
            result = i;
        }
    return result;
}

void resetAnts()
{
    uint i, j;
    ant *ptAnt;
    bool *ptBool;
    for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
    {
        ptAnt->price = 0.0;
        ptAnt->nodeCounter = 0;
        for(i=0, ptBool=ptAnt->nodesVisited; i<prData.dim; i++, ptBool++)
            *ptBool = false;
        for(i=0, ptBool=ptAnt->carsRented; i<prData.nCars; i++, ptBool++)
            *ptBool = false;
    }
}



