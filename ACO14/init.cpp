#include "ACO.h"

using namespace std;

void initNodes();
void initCars();
void initPassengers();
void initAnts();
void initProbArrays();
void initBestPath();


/* initialize and cleanup */
void init()
{
    mt19937::result_type seed = time(0);
    mersenneGenerator = new mt19937(seed);
    initPassengers();
    initCars();
    initNodes();
    initAnts();
    initProbArrays();
    initBestPath();
}


void cleanup()
{
    uint j;
    node* ptNode;
    ant* ptAnt;
    if(nodes != nullptr)
    {
        for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
        {
            delete[] ptNode->pheroNeighbours;
            delete[] ptNode->pheroCars;
            delete[] ptNode->passengers;
        }
        delete[] nodes; 
    }
    if(passengers != nullptr)
        delete[] passengers; 

    if(cars != nullptr)
        delete[] cars;
    
    if(ants != nullptr)
    {
        for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
        {
            delete[] ptAnt->bestOptNodes;
            delete[] ptAnt->optNodes;
            delete[] ptAnt->nodes;
            delete[] ptAnt->nodesVisited;
            delete[] ptAnt->carsRented;
            if(ptAnt->passPicked !=nullptr)
                delete[] ptAnt->passPicked;
        }
        delete[] ants;
    }
    delete[] probArrays.indices;
    delete[] probArrays.probs;
    delete[] probArrays.cumulatives;
    delete[] bPath.nodes;
    delete[] bPath.optNodes;
    bPath.nodeCounter = 0;
    bPath.optNodeCounter = 0;
    delete mersenneGenerator;
}

/* separated elements */
void initNodes()
{
    uint i, j;
    node* ptNode;
    pass* ptPass;
    float* ptFloat;
    if(passengers == nullptr)
    {
        nodes = nullptr;
        return;
    }
    nodes = new node[prData.dim];
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)     //for each node
    {
        sprintf(ptNode->name, "V%d", j);
        ptNode->index = j;
        ptNode->pheroNeighbours = new float[prData.dim];
        ptNode->pheroCars = new float[prData.nCars];
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
        {
            if(i==j)
                *ptFloat = 0.0;
            else
                *ptFloat = parData.tau;            
        }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = parData.tau;
        ptNode->nPassengers = 0;
    }
    /* number of passengers */
    for(i=0, ptPass = passengers; i<prData.nPass; i++, ptPass++)
        nodes[ptPass->startNode].nPassengers++;

    /* max node pasengers in prData */
    for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
        if(ptNode->nPassengers > prData.maxNodePassengers)
            prData.maxNodePassengers = ptNode->nPassengers;

    /* init array of passengers */
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++) 
    {
        ptNode->passengers = new pass*[ptNode->nPassengers];
        ptNode->nPassengers = 0;
    }
    /* fill node's arrays of passengers */
    for(i=0, ptPass = passengers; i<prData.nPass; i++, ptPass++)
    {
        ptNode = &nodes[ptPass->startNode];
        ptNode->passengers[(ptNode->nPassengers)++] = ptPass;
    }
}

void initCars()
{
    uint i;
    car *ptCar;
    uchar *ptUchar;
    cars = new car[prData.nCars];
    for(i=0, ptCar = cars, ptUchar = prData.carPassLimit; i<prData.nCars; i++, ptCar++, ptUchar++)
    {
        ptCar->index = i;
        sprintf(ptCar->name, "C%d", i);
        
    }
}

void initPassengers()
{
    uint i;
    pass* ptPass;
    dataPass* ptDataPass;
    passengers = new pass[prData.nPass];
    /* copy data from dataPass structures to pass structures */
    for(i=0, ptPass=passengers, ptDataPass=prData.passengers; i<prData.nPass; i++, ptPass++, ptDataPass++)
    {
        ptPass->index = i;
        sprintf(ptPass->name, "P%d", i);
        ptPass->startNode = ptDataPass->startNode;
        ptPass->endNode = ptDataPass->destinationNode;
    }
}

void initAnts()
{
    uint i, j;
    ant* ptAnt;
    bool* ptBool;

    ants = new ant[parData.nAnts];
    for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
    {
        ptAnt->nodes = new antNode[prData.dim];
        ptAnt->optNodes = new antNode[prData.dim];
        ptAnt->bestOptNodes = new antNode[prData.dim];

        ptAnt->nodeCounter = 0;
        ptAnt->nodesVisited = new bool[prData.dim];
        for(i=0, ptBool=ptAnt->nodesVisited; i<prData.dim; i++, ptBool++)
            *ptBool = false;

        ptAnt->carsRented = new bool[prData.nCars];
        for(i=0, ptBool=ptAnt->carsRented; i<prData.nCars; i++, ptBool++)
            *ptBool = false;

        if(prData.maxNodePassengers > 0)
        {
            ptAnt->passPicked = new bool[prData.maxNodePassengers];
            for(i=0, ptBool=ptAnt->passPicked; i<prData.maxNodePassengers; i++, ptBool++)
                *ptBool = false;
        }
        else
            ptAnt->passPicked = nullptr;
    }
}

void initProbArrays()
{
    uint i, max;
    max = prData.dim;
    if(prData.nCars > max)
        max = prData.nCars;
    for(i=0; i<prData.dim; i++)
        if(nodes[i].nPassengers > max)
            max = nodes[i].nPassengers;
    probArrays.n = 0;
    probArrays.indices = new uint[max];
    probArrays.probs = new float[max];
    probArrays.cumulatives = new float[max];

}

void initBestPath()
{
    bPath.nodeCounter = 0;
    bPath.nodes = new antNode[prData.dim];
    bPath.optNodeCounter = 0;
    bPath.optNodes = new antNode[prData.dim];
    bPath.price = numeric_limits<float>::max();
    bPath.optPrice = numeric_limits<float>::max();
    bPath.pheroMax = TAU_0;
    bPath.pheroMin = 0.0;
}
