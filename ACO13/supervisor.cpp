#include "supervisor.h"

using namespace std;

char bestHeader[][8]={"Iter","Rbacks","Cost","CostN/P","Best","BestN/P","Min","Max","fc","Ncar","Npas"};
char antHeader[][8]={"Iter","Ant", "Rbacks", "TauMin", "TauMax", "Cost", "CostN/P", "Best", "BestN/P", "Nnodes", "Ncar","Npass", "Best"};
char arcHeader[][8]={"Iter","Row", "Column","SrcNode", "DstNode", "PhMin", "PhMax", "Phero"};
char carHeader[][9]={"Iter","Car","RentNode","Phero"};
char passHeader[][8]={"Iter","Node","Pass","Budg","RemBudg","Phero"};

ofstream bestFileStream, antFileStream, arcFileStream, carFileStream, passFileStream, resultStream;


/* starts all supervisor stuff e.g. opening files */
void beginSupervisor(const char* filename)
{
    char name[32];
    sprintf(name,"%s_best.txt", filename);
    bestFileStream.open(name, ios::out);
    sprintf(name,"%s_ant.txt", filename);
    antFileStream.open(name, ios::out);
    sprintf(name,"%s_arc.txt", filename);
    arcFileStream.open(name, ios::out);
    sprintf(name,"%s_car.txt", filename);
    carFileStream.open(name, ios::out);
    sprintf(name,"%s_pass.txt", filename);
    passFileStream.open(name, ios::out);
}

/* ends supervisor */
void endSupervisor()
{
    bestFileStream.close();
    arcFileStream.close();
    antFileStream.close();
    carFileStream.close();
    passFileStream.close();
}

/* supervisor functions */
void writeHeaders()
{
    unsigned long i, strSize;
    strSize = sizeof(bestHeader)/(8*sizeof(char));

    for(i=0; i<strSize-1; i++)
        bestFileStream << bestHeader[i] << " ";
    bestFileStream << bestHeader[i] << endl;
    strSize = sizeof(antHeader)/(8*sizeof(char));
    for(i=0; i<strSize-1; i++)
        antFileStream << antHeader[i] << " ";
    antFileStream << antHeader[i] << endl;
    strSize = sizeof(arcHeader)/(8*sizeof(char));
    for(i=0; i<strSize-1; i++)
        arcFileStream << arcHeader[i] << " ";
    arcFileStream << arcHeader[i] << endl;
    strSize = sizeof(carHeader)/(9*sizeof(char));
    for(i=0; i<strSize-1; i++)
        carFileStream << carHeader[i] << " ";
    carFileStream << carHeader[i] << endl;
    strSize = sizeof(passHeader)/(8*sizeof(char));
    for(i=0; i<strSize-1; i++)
        passFileStream << passHeader[i] << " ";
    passFileStream << passHeader[i] << endl;
}

/* supervisor functions */

/* global best tour data */
void writeBestData()
{
    uint i, j;
    car **pptCar;
    pass **pptPass;
    for(i=0, pptCar=localBestCostTour.carList; i<prData.nCars && (*pptCar)!=nullptr; i++, pptCar++);
    for(j=0, pptPass=localBestCostTour.passList; j<prData.nPass && (*pptPass)!=nullptr; j++, pptPass++);
    bestFileStream << (localBestCostTour.iteration) << " " <<
            (localBestCostTour.rollbacks) << " " <<
            (localBestCostTour.cost) << " " <<
            (localBestCostTour.costNoPass) << " " <<
            (globalBestCostTour.cost) << " " << (globalBestCostTour.costNoPass) << " " <<
            (localBestCostTour.tauMin) << " " <<
            (localBestCostTour.tauMax) << " " <<
            (localBestCostTour.fc) << " " <<
            i << " " << j << endl;
}

/* ant data */
void writeAntsData()
{
    uint i, j, k, l;
    ant* ptAnt;
    car** pptCar;
    pass** pptPass;
    tourArc* ptTourArc;
    j = k = l = 0;
    for(i=0, ptAnt=ants; i < N_ANTS; i++, ptAnt++)
    {
        
        for(j=0, ptTourArc=ptAnt->t.arcs; j<=prData.dim && ptTourArc->a!=nullptr; j++, ptTourArc++);
        for(k=0, pptCar=ptAnt->t.carList; k<prData.nCars && (*pptCar)!=nullptr; k++, pptCar++);
        for(l=0, pptPass=ptAnt->t.passList; l<prData.nPass && (*pptPass)!=nullptr; l++, pptPass++);
        
        /* valgrind may show error due to file write */
        
        antFileStream << (ptAnt->t.iteration) << " " <<
        (ptAnt->t.ant) << " " <<
        (ptAnt->t.rollbacks) << " " <<
        (ptAnt->t.tauMin) << " " << (ptAnt->t.tauMax) << " " <<
        (ptAnt->t.cost) << " " <<
        (ptAnt->t.costNoPass) << " " << 
        (globalBestCostTour.cost) << " " << (globalBestCostTour.costNoPass) << " " <<
        j << " " << k << " " << l << " " << 
        (ptAnt->status) << endl;        
    }
}

/* arc data */
void writeArcsData(uint iter)
{
    uint i, j;
    arc** pptArc;
    arc* ptArc;
    for(j=0, pptArc=arcs; j<prData.dim; j++, pptArc++)
    {
        for(i=0, ptArc=*pptArc; i<prData.dim; i++, ptArc++)
        {
            /* valgrind may show error due to file write */
            
            arcFileStream << iter << " " << 
            j << " " << i << " " <<
            nodes[ptArc->row].name << " " << 
            nodes[ptArc->column].name << " " <<  
            tau_min << " " << tau_max << " " <<
            ptArc->tau << " " <<
            endl;
            
        }
        
    }  
}

/* car data */
void writeCarsData(uint iter)
{
    uint i;
    car* ptCar;
    for(i=0, ptCar=cars; i<prData.nCars; i++, ptCar++)
    {
        /* valgrind may show error due to file write */
        
        carFileStream << iter << " " << 
        ptCar->name << " " << 
        nodes[ptCar->rentedNode].name << " " << 
        endl;  
    }  
}

/* passengers data */
void writePassData(uint iter)
{
    uint i, j;
    pass* ptPass;
    node* ptNode;
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, ptPass = ptNode->passengers; i<ptNode->nPass && ptPass!=nullptr; i++, ptPass++)
        {
            /* valgrind may show error due to file write */
            
            passFileStream << iter << " " <<
            ptNode->name << " " <<
            ptPass->name << " " <<
            ptPass->budget << " " <<
            ptPass->currentBudget << " " <<
            ptPass->tau << " " <<
            endl;
            
        }
    }
}

void writeResult(const char* filename)
{
    char name[32];
    uint i;
    tourArc* ptTour;
    car** pptCar;
    pass** pptPass;
    sprintf(name,"%s.txt", filename);
    resultStream.open(name, ios::out);
    if(globalBestCostTour.arcs->a == nullptr)
    {
        /* valgrind may show error due to file write */
        
        resultStream << "No best cost tour." << endl;
        return;
    }
    /* valgrind may show error due to file write */
    
    resultStream << "Best: iteration: " << globalBestCostTour.iteration << ", ant: " << globalBestCostTour.ant << 
    ", start node: " << nodes[globalBestCostTour.arcs->a->row].name << 
    //", avg: " << ants[globalBestCostTour.ant].avg << ", nRtProb: " << ants[globalBestCostTour.ant].nRtProb <<
    ", tau_max: " << tau_max << ", tau_min: " << tau_min << ", rollbacks: " << globalBestCostTour.rollbacks << endl;
    resultStream << "Nodes (name, pheromone, car name, picks, arc cost, shared cost, car cost): " << endl;
    for(i=0, ptTour=globalBestCostTour.arcs; i<=prData.dim && ptTour->a!=nullptr; i++, ptTour++)
    {
        resultStream << "(" << nodes[ptTour->a->row].name << ", " << ptTour->a->tau << ", " << ptTour->c->name << ", " << 
        nodes[ptTour->a->row].nPass << ", " <<
        prData.edgeWeightMatrices[ptTour->c->n][ptTour->a->row][ptTour->a->column] << 
        ", " << ptTour->cost << ", " << ptTour->carReturnCost << ")" << endl;
        resultStream << "  ";
        for(pptPass = ptTour->p; (*pptPass)!=nullptr; pptPass++)
            resultStream << (*pptPass)->name << "(tau=" << (*pptPass)->tau << ")" << (*pptPass)->startNode->name << "->" << (*pptPass)->destinationNode->name << ", ";
        resultStream << "; " << endl;
    }

    if(ptTour > globalBestCostTour.arcs)
        resultStream << "(" << nodes[(ptTour-1)->a->column].name << ",-)" << endl;
    resultStream << "Cars (name, pheromone, passengers): " << endl;
    for(i=0, pptCar=globalBestCostTour.carList; i<prData.nCars && *pptCar!=nullptr; i++, pptCar++)
        resultStream << "(" << (*pptCar)->name << ", " << 
        (int)(*pptCar)->nPass << "/" << (int)(*pptCar)->carPassLimit << "), ";
    resultStream << endl;
    resultStream << "Passengers (name, tau):" << endl;
    for(i=0, pptPass=globalBestCostTour.passList; i<prData.nPass && *pptPass!=nullptr; i++, pptPass++)
        resultStream << (*pptPass)->name << ", " << (*pptPass)->tau << ", ";
    resultStream << endl;
    resultStream << ">>> Cost: " << globalBestCostTour.cost << ", (" << globalBestCostTour.costNoPass << " without passengers)" << endl;
    
    resultStream.close();
}

