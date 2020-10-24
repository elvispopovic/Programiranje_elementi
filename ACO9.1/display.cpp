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

void displayBestTour()
{
    register uint i;
    register tourArc* ptTour;
    register car** pptCar;
    cout << "Nodes (name, pheromone, car name, picks, shared cost): " << endl;
    for(i=0, ptTour=bestCostTour.arcs; i<=prData.dim && ptTour->a!=nullptr; i++, ptTour++)
        cout << "(" << nodes[ptTour->a->row].name << ", " << ptTour->a->tau << ", " << ptTour->c->name << ", " << 
        nodes[ptTour->a->row].nPass << ", " <<
        prData.edgeWeightMatrices[ptTour->c->n][ptTour->a->row][ptTour->a->column] << "), ";
    cout << endl;
    if(ptTour > bestCostTour.arcs)
        cout << "(" << nodes[(ptTour-1)->a->column].name << ",-)" << endl;
    cout << "Cars (name, pheromone, passengers at end): " << endl;
    for(i=0, pptCar=bestCostTour.carList; i<prData.nCars && *pptCar!=nullptr; i++, pptCar++)
        cout << "(" << (*pptCar)->name << ", " << (*pptCar)->tau << ", " << 
        (int)(*pptCar)->nPass << "/" << (int)(*pptCar)->carPassLimit << "), ";
    cout << endl;
    cout << ">>> Cost: " << bestCostTour.cost << endl;
}