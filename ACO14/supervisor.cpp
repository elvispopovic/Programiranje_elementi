#include "supervisor.h"

using namespace std;

char nodeHeader[][8]={"Index","Name","nPass"};
char nodePassHeader[][8]={"Node","Index","Name","Start","End"};


ofstream problemDataStr, initDataStr;

void writeHeaders();
void displayMatrix(float*** matrix, int nMatrix, int dim);


/* starts all supervisor stuff e.g. opening files */
void beginSupervisor(const char* filename)
{
    char name[64];
    sprintf(name,"%s_input_data.txt", filename);
    problemDataStr.open(name, ios::out);
    sprintf(name,"%s_init_data.txt", filename);
    initDataStr.open(name, ios::out);

    
}

/* ends supervisor */
void endSupervisor()
{
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
    problemDataStr << "Problem data parameters:" << endl;
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
    problemDataStr << "Passenger's limits (car capacities):" << endl;
    for(i=0, pt1=prData.carPassLimit; i<prData.nCars; i++, pt1++)
        problemDataStr << "(car:" << i << ", cap:" << (int)*pt1 << ") ";
    problemDataStr << endl << "Passengers: " << endl;
    for(i=0, ptPass=prData.passengers; i<prData.nPass; i++, ptPass++)
        problemDataStr << ptPass->startNode << " " << ptPass->destinationNode << " " << ptPass->budget << endl;
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

