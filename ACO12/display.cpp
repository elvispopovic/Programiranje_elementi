#include "ACO.h"

using namespace std;

void displayMatrix(float*** matrix, int nMatrix, int dim)
{
    register int i;
    int j;
    register float *pt1;
    float **pt2;
    cout << "Car: " << nMatrix << endl;
    for(j=0, pt2=*matrix; j<dim; j++,pt2++)
    {
        for(i=0, pt1=*pt2; i<dim; i++,pt1++)
            cout << *pt1 << " ";
        cout << endl;
    }
}
void displayProblemData()
{
    register uint i;
    register uchar *pt1;
    register dataPass *ptPass;
    cout << "Problem data parameters:" << endl;
    cout << "Name: " << prData.name << endl;
    cout << "Type: " << prData.type << endl;
    cout << "Comment: " << prData.comment << endl;
    cout << "Dimension: " << (int)prData.dim << endl;
    cout << "Cars: " << (int)prData.nCars << ", passengers: " << (int)prData.nPass << endl;
    cout << "Edge Weight Matrices: " << endl;
    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.edgeWeightMatrices, i, prData.dim);
    cout << "Return rate Matrices: " << endl;
    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.returnRateMatrices, i, prData.dim);
    cout << "Passenger's limits (car capacities):" << endl;
    for(i=0, pt1=prData.carPassLimit; i<prData.nCars; i++, pt1++)
        cout << "(car:" << i << ", cap:" << (int)*pt1 << ") ";
    cout << endl << "Passengers: " << endl;
    for(i=0, ptPass=prData.passengers; i<prData.nPass; i++, ptPass++)
        cout << ptPass->startNode << " " << ptPass->destinationNode << " " << ptPass->budget << endl;
    cout << "END" << endl;
}

void displayNodes()
{
    uint j;
    register node* ptNode;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        cout << "Node " << ptNode->n << ": " << ptNode->name << " (" << (int)ptNode->nPass << ")" << endl;
    }
}

/* global or local best */
void displayBestTour(tour* ptTour)
{
    uint i,j;
    float wcost, ccost, *ptFloat;
    tourArc* ptTourArc;
    car **pptCar, *ptCar;
    pass** pptPass; 
    if(ptTour->arcs->a == nullptr)
        return;
    cout << "Best: iteration: " << ptTour->iteration << ", ant: " << ptTour->ant;
    cout << ", start node: " << nodes[ptTour->arcs->a->row].name;
    //cout << ", avg: " << ants[ptTour->ant].avg << ", nRtProb: " << ants[ptTour->ant].nRtProb;
    cout << ", tau_min: " << tau_min << ", tau_max: " << tau_max;
    cout << ", rollbacks: " << ptTour->rollbacks << endl;
    cout << "Nodes (name, pheromone, car name, picks, arc cost, shared cost, car cost): " << endl;
    for(j=0, ptTourArc=ptTour->arcs; j<=prData.dim && ptTourArc->a!=nullptr; j++, ptTourArc++)
    {
        cout << "(" << nodes[ptTourArc->a->row].name << ", " << ptTourArc->a->tau << ", " << ptTourArc->c->name << ", " << 
        nodes[ptTourArc->a->row].nPass << ", " <<
        prData.edgeWeightMatrices[ptTourArc->c->n][ptTourArc->a->row][ptTourArc->a->column] << 
        ", " << ptTourArc->cost << ", " << ptTourArc->carReturnCost << ")" << endl;
        cout << "car pick pheromones: ";
        for(i=0, ptCar = cars, ptFloat = nodes[ptTourArc->a->row].tau_cars; i<prData.nCars; i++, ptCar++, ptFloat++)
            cout << ptCar->name << " " << (*ptFloat) << ", "; 
        cout << "; " << endl;
        cout << "passengers: ";
        for(pptPass = ptTourArc->p; (*pptPass)!=nullptr; pptPass++)
            cout << (*pptPass)->name << "(tau=" << (*pptPass)->tau << ")" << (*pptPass)->startNode->name << "->" << (*pptPass)->destinationNode->name << ", ";
        cout << "; " << endl;
    }

    if(ptTourArc > ptTour->arcs)
        cout << "(" << nodes[(ptTourArc-1)->a->column].name << ",-)" << endl;
    cout << "Cars (name, pheromone, passengers): " << endl;
    for(i=0, pptCar=ptTour->carList; i<prData.nCars && *pptCar!=nullptr; i++, pptCar++)
        cout << "(" << (*pptCar)->name << ", " << (*pptCar)->tau << ", " << 
        (int)(*pptCar)->nPass << "/" << (int)(*pptCar)->carPassLimit << "), ";
    cout << endl;
    cout << "Passengers (name, tau):" << endl;
    for(i=0, pptPass=ptTour->passList; i<prData.nPass && *pptPass!=nullptr; i++, pptPass++)
        cout << (*pptPass)->name << ", " << (*pptPass)->tau << ", ";
    cout << endl;
    cout << "Choices list:" << endl;
    for(i=0, ptFloat=ptTour->choiceProbList; i<prData.dim; i++, ptFloat++)
        cout << (*ptFloat) << ", ";
    cout << endl;
    cout << ">>> Cost: " << ptTour->cost << ", (" << ptTour->costNoPass << " without passengers)" << ", ";
    calculateCostsFromTour(ptTour, wcost, ccost); //check
    cout << "check - written: " << wcost << ", calculated: " << ccost << (wcost == ccost? " O.K.":" ERR") << endl;
    if(ptTour != &globalBestCostTour)
        cout << ">>> Global best cost: " << globalBestCostTour.cost << ", (" << globalBestCostTour.costNoPass << " without passengers)" << endl;
    cout << "========================= (eoi)" << endl;
}