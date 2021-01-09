#include "ACO.h"

using namespace std;

void initNodes();
void initPassengers();



/* initialize and cleanup */
void init()
{
    initPassengers();
    initNodes();
}


void cleanup()
{
    uint j;
    node* ptNode;
    if(nodes == nullptr)
        return;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        delete[] ptNode->pheroNeighbours;
        delete[] ptNode->pheroCars;
        delete[] ptNode->passengers;
    }
    delete[] nodes; 
    delete[] passengers; 
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
