#include "ACO.h"

using namespace std;

void initNodes();
void initCars();
void initPassengers();
void initAnts();


/* initialize and cleanup */
void init()
{
    mt19937::result_type seed = time(0);
    mersenneGenerator = new mt19937(seed);


    initPassengers();
    initCars();
    initNodes();
    initAnts();
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
        for(j=0, ptAnt=ants; j<N_ANTS; j++, ptAnt++)
        {
            delete[] ptAnt->nodes;
            delete[] ptAnt->nodeCandidateProbs;
            delete[] ptAnt->nodeCandidatesIndices;
            delete[] ptAnt->nodeVisited;
        }
        delete[] ants;
    }
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
        return;
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
                *ptFloat = TAU_0;            
        }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = TAU_0;
        ptNode->nPassengers = 0;
    }
    /* number of passengers */
    for(i=0, ptPass = passengers; i<prData.nPass; i++, ptPass++)
        nodes[ptPass->startNode].nPassengers++;
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
    ants = new ant[N_ANTS];
    for(j=0, ptAnt=ants; j<N_ANTS; j++, ptAnt++)
    {
        ptAnt->nodes = new antNode[prData.dim+1]; //first and last node are same

        ptAnt->nodeCounter = 0;
        ptAnt->nodeVisited = new bool[prData.dim];
        for(i=0, ptBool=ptAnt->nodeVisited; i<prData.dim; i++, ptBool++)
        {
            *ptBool = false;
        }
        ptAnt->nodeCandidatesIndices = new uint[prData.dim]; 
        ptAnt->nodeCandidateProbs = new float[prData.dim];
    }
}
