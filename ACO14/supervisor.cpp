#include "supervisor.h"

char bestHeader[][14]={"Iter","BestAnt","BestAntOpt","BestPath1","BestPath2","BestOptPath1","BestOptPath2"};

using namespace std;

char nodeHeader[][8]={"Index","Name","nPass"};
char nodePassHeader[][8]={"Node","Index","Name","Start","End"};


ofstream problemDataStr, initDataStr, bestFileStream, resultStream;

void writeHeaders();
void displayMatrix(float*** matrix, int nMatrix, int dim);


/* starts all supervisor stuff e.g. opening files */
void beginSupervisor(const char* filename)
{
    char name[64];
    if(parData.writeData == false)
        return;
    sprintf(name,"%s_input_data.txt", filename);
    problemDataStr.open(name, ios::out);
    sprintf(name,"%s_init_data.txt", filename);
    initDataStr.open(name, ios::out);
    sprintf(name,"%s_results.txt", filename);
    resultStream.open(name, ios::out);
    sprintf(name,"%s_best.txt", filename);
    bestFileStream.open(name, ios::out);
}

/* ends supervisor */
void endSupervisor()
{   
    if(parData.writeData == false)
        return;
    resultStream.close();
    bestFileStream.close();
    initDataStr.close();
    problemDataStr.close();
}

/* supervisor functions */

/* supervisor functions */
void writeProblemData()
{
    uint i;
    uchar *pt1;
    dataPass *ptPass;
    problemDataStr << "Parameters (argc=" << parData.argc << "):" << endl;
    problemDataStr << "Data file: " << parData.fileName << endl;
    problemDataStr << "tau: " << parData.tau << endl;
    problemDataStr << "rho: " << parData.rho << endl;
    problemDataStr << "alpha: " << parData.alpha << endl;
    problemDataStr << "beta: " << parData.beta << endl;
    problemDataStr << "nAnts: " << parData.nAnts << endl;
    problemDataStr << "opt: " << (parData.opt?"yes":"no") << endl;
    problemDataStr << "nIter: " << parData.nIter << endl;
    problemDataStr << "------------------------\nProblem data parameters:" << endl;
    problemDataStr << "Name: " << prData.name << endl;
    problemDataStr << "Type: " << prData.type << endl;
    problemDataStr << "Comment: " << prData.comment << endl;
    problemDataStr << "Dimension: " << (int)prData.dim << endl;
    problemDataStr << "Cars: " << (int)prData.nCars << ", passengers: " << (int)prData.nPass << endl;
    problemDataStr << "Edge Weight Matrices: " << endl;

    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.edgeWeightMatrices, i, prData.dim);
    problemDataStr << "Return rate Matrices: " << endl;
    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.returnRateMatrices, i, prData.dim);

    if(prData.nPass > 0) // we can not write passengers if there are no any
    {
        problemDataStr << "Passenger's limits (car capacities):" << endl;
        for(i=0, pt1=prData.carPassLimit; i<prData.nCars; i++, pt1++)
            problemDataStr << "(car:" << i << ", cap:" << (int)*pt1 << ") ";

        problemDataStr << endl << "Passengers: " << endl;
        for(i=0, ptPass=prData.passengers; i<prData.nPass; i++, ptPass++)
            problemDataStr << ptPass->startNode << " " << ptPass->destinationNode << " " << ptPass->budget << endl;
    }
    problemDataStr << "END" << endl;

}

void writeInitData()
{
    uint i,j;
    unsigned long strSize;
    node* ptNode;
    pass** pptPass;
    strSize = sizeof(nodeHeader)/(8*sizeof(char));
    initDataStr << "Nodes:" << endl;
    for(i=0; i<strSize-1; i++)
        initDataStr << nodeHeader[i] << " ";
    initDataStr << nodeHeader[i] << endl;
    for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
    {
        initDataStr << ptNode->index << " " << 
        ptNode->name << " " << 
        ptNode->nPassengers << endl;
    }

    strSize = sizeof(nodePassHeader)/(8*sizeof(char));
    initDataStr << "Node passengers:" << endl;
    for(i=0; i<strSize-1; i++)
        initDataStr << nodePassHeader[i] << " ";
    initDataStr << nodePassHeader[i] << endl;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, pptPass=ptNode->passengers; i<ptNode->nPassengers; i++, pptPass++)
        {
            initDataStr << ptNode->name << " " << 
            (*pptPass)->index << " " <<
            (*pptPass)->name << " " <<
            (*pptPass)->startNode << " " <<
            (*pptPass)->endNode << endl;
        }
    }
}

/* help functions */
void writeHeaders()
{
    unsigned long i, strSize;
    if(parData.writeData == false)
        return;
    strSize = sizeof(bestHeader)/(14*sizeof(char));

    for(i=0; i<strSize-1; i++)
        bestFileStream << bestHeader[i] << " ";
    bestFileStream << bestHeader[i] << endl;

}

void writeResult()
{
    uint i, j;
    float *ptFloat;
    antNode *ptAntNode;
    car *ptCar;
    node *ptNode, *carPickNode;
    if(parData.writeData == false)
        return;
    if(bPath.nodeCounter == 0)
    {
        resultStream << "No best cost tour." << endl;
        return;
    }
    resultStream << "Best: iteration: " << bPath.iteration << 
    ", start node: " << bPath.nodes->curNode->name << ", cost: " << bPath.price << endl;
    resultStream << "Nodes (name, car in, car out, arc cost, car return cost): " << endl;
    for(i=0, ptAntNode=bPath.nodes, carPickNode=bPath.nodes->curNode; i<bPath.nodeCounter; i++, ptAntNode++)
    {
        resultStream << ptAntNode->curNode->name << " " << 
        (ptAntNode->carIn==nullptr?"C-":ptAntNode->carIn->name) << " " << ptAntNode->carOut->name << " " <<
        prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index] << " ";
        if(i==0)
            resultStream << 0 << endl;
        else if(ptAntNode->carIn->index == ptAntNode->carOut->index)
        {
            resultStream << 0 << endl;
        }
        else
        {
            resultStream << prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index] << endl;
            carPickNode = ptAntNode->curNode;
        }
    }
    resultStream << bPath.nodes->curNode->name << " " <<  (ptAntNode-1)->carOut->name << " - " << " - ";
    resultStream << prData.returnRateMatrices[(ptAntNode-1)->carOut->index][(ptAntNode-1)->nextNode->index][carPickNode->index] << endl;
    resultStream << "Pheromones: neighbours:" << endl;
    for(j=0, ptAntNode = bPath.nodes; j<bPath.nodeCounter; j++, ptAntNode++)
    {
        resultStream << (ptAntNode->prevNode==nullptr?"V-":ptAntNode->prevNode->name) << " -> " << 
        ptAntNode->curNode->name << " -> " << ptAntNode->nextNode->name << endl;
        for(i=0, ptNode=nodes, ptFloat=ptAntNode->curNode->pheroNeighbours; i<prData.dim; i++, ptNode++, ptFloat++)
            resultStream << "(" << ptNode->name << ";" << *ptFloat << ") ";
        resultStream << endl;
    }
    resultStream << "Pheromones: cars:" << endl;
    for(j=0, ptAntNode = bPath.nodes; j<bPath.nodeCounter; j++, ptAntNode++)
    {
        for(i=0, ptCar=cars, ptFloat=ptAntNode->curNode->pheroCars; i<prData.nCars; i++, ptCar++, ptFloat++)
            resultStream << "(" << ptCar->name << ";" << *ptFloat << ") ";
        resultStream << endl;
        resultStream << ptAntNode->curNode->name << ", " << 
        (ptAntNode->carIn==nullptr?"C-":ptAntNode->carIn->name) << " -> " << ptAntNode->carOut->name << endl;
    }
    resultStream << "Summary:" << endl;
    for(j=0, ptAntNode = bPath.nodes; j<bPath.nodeCounter-1; j++, ptAntNode++)
        resultStream << ptAntNode->curNode->name << ", ";
    resultStream << ptAntNode->curNode->name << endl;
    resultStream << bPath.nodes->carOut->name << (bPath.nodeCounter<=1?"\n":", ");
    for(j=1, ptAntNode = bPath.nodes+1; j<bPath.nodeCounter-1; j++, ptAntNode++)
        if(ptAntNode->carIn->index != ptAntNode->carOut->index)
            resultStream << ptAntNode->carOut->name << ", ";
    if(ptAntNode->carIn->index != ptAntNode->carOut->index)
            resultStream << ptAntNode->carOut->name << endl;


}

void writeBestData(uint iteration, ant *bestAnt)
{
    if(parData.writeData == false)
        return;
    bestFileStream << iteration << " " <<
    bestAnt->price << " " << bestAnt->bestOptPrice << " " << 
    bPath.price << " " << calculatePathCost() << " " << 
    bPath.optPrice << " " << calculateOptPathCost() << endl;;
}

void displayMatrix(float*** matrix, int nMatrix, int dim)
{
    register int i;
    int j;
    float *pt1;
    float **pt2;
    problemDataStr << "Car: " << nMatrix << endl;
    for(j=0, pt2=*matrix; j<dim; j++,pt2++)
    {
        for(i=0, pt1=*pt2; i<dim; i++,pt1++)
            problemDataStr << *pt1 << " ";
        problemDataStr << endl;
    }
}

