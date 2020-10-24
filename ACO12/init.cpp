#include "ACO.h"

using namespace std;

void setNodes(); //nodes setup
void setCars(); //cars setup
void setArcs(); // node arcs setup
void setAnts(); // ant structure setup
void setPassengers(); // passenger structure setup
void setInverseVariables(); // inverse variables (to avoid divisions during algorithm execution)

void setBestTour(tour* ptTour);
void setRollBack();

void Init()
{
    cout << "Init..." << endl;
    mt19937::result_type seed = time(0);
    mersenneGenerator = new mt19937(seed);

    /* cars */
    setCars(); 

    /* nodes */
    setNodes();

    /* passengers */
    setPassengers();

    /* arcs */
    setArcs();

    /* probability arrays */
    uint max = prData.dim; //selProbArray used for cars, passengers and pass selection number
    if(prData.nCars > max)
        max = prData.nCars;
    if(maxNodePassengers > max) //for hypothetical node with all passengers and including select none
        max = prData.nPass;
    selProbArray = new float[max];
    arcProbArray = new arcProb[prData.dim];
    carProbArray = new carProb[prData.nCars];
    passProbArray = new passProb[maxNodePassengers];

    /* ants */
    setAnts();

    /* best tour */
    setBestTour(&localBestCostTour);
    setBestTour(&globalBestCostTour);

    /* rollback of passenger state */
    setRollBack();

    /* inverse variables */
    setInverseVariables();

    cout << "Init done." << endl;
}

void cleanupBestTours(tour* ptTour)
{
    uint j;
    tourArc* ptTourArc;
    if(ptTour->arcs!=nullptr)
    {
        for(j=0, ptTourArc=ptTour->arcs; j<prData.dim; j++, ptTourArc++)
            if(ptTourArc != nullptr)
                delete[] ptTourArc->p;
        delete[] ptTour->arcs; // tour arcs
    }
    if(ptTour->carList!=nullptr)
        delete[] ptTour->carList; // tour cars
    if(ptTour->passList!=nullptr)
        delete[] ptTour->passList; //tour passengers
    if(ptTour->choiceProbList!=nullptr)
        delete[] ptTour->choiceProbList; // tour choice probabilities
}

void Cleanup()
{
    uint i, j;
    tourArc* ptTourArc;
    arc** pptArc;
    node* ptNode;
    ant* ptAnt;
    cout << "Cleanup..." << endl;
    /* cars */
    if(cars!=nullptr)
    {
        delete[] cars;
    }
    /* arcs */
    if(arcs != nullptr)
    {
        for(i=0, pptArc=arcs; i<prData.dim; i++, pptArc++)
            delete[] *pptArc;
        delete[] arcs;
    }

    /* nodes */
    if(nodes != nullptr)
    {
        for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
        {
            delete[] ptNode->passengers;
            delete[] ptNode->tau_cars;
        }
        delete[] nodes;
    }
    if(carPassArray != nullptr)
        delete[] carPassArray;

       
    /* best cost tour */
    cleanupBestTours(&localBestCostTour);
    cleanupBestTours(&globalBestCostTour);

    if(selProbArray!=nullptr)
        delete[] selProbArray;
    if(arcProbArray!=nullptr)
        delete[] arcProbArray;
    if(carProbArray!=nullptr)
        delete[] carProbArray;
    if(passProbArray!=nullptr)
        delete[] passProbArray;
    if(rollBack.lastPassList!=nullptr)
        delete[] rollBack.lastPassList;
    
    /* ants */
    if(ants!=nullptr)
    {
        for(j=0, ptAnt = ants; j<N_ANTS; j++, ptAnt++)
        {  
            if(ptAnt->t.arcs != nullptr)
            {
                for(i=0, ptTourArc=ptAnt->t.arcs; i<prData.dim; i++, ptTourArc++)
                    if(ptTourArc->p != nullptr)
                        delete[] ptTourArc->p;
                delete[] ptAnt->t.arcs; // tour arcs
            }
            delete[] ptAnt->t.carList;
            delete[] ptAnt->t.passList;
            delete[] ptAnt->t.choiceProbList;
        }
        delete[] ants;
    }

    /* random generator */
    delete mersenneGenerator;
    cout << "Cleanup done." << endl;
}

void setInverseVariables()
{
    invDim = 1.0/(float)prData.dim; //inverse dimension variable
    if(prData.nPass != 0)
        invNPass = 1.0/(float)prData.nPass; //inverse number of passengers variable
    if(maxCarPassLimit != 0)
        invMaxCarPassLimit = 1.0/(float)maxCarPassLimit; //inverse max car passenger capacity variable
}

void setPassengers()
{
    uint i, j;
    dataPass* ptDataPass; //passengers in file data structure
    pass* ptPass; //passengers in nodes
    node* ptNode;
    maxNodePassengers = 0;

    for(i=0, ptDataPass = prData.passengers; i<prData.nPass; i++, ptDataPass++)
        nodes[ptDataPass->startNode].nPass++; //put number of passengers into nodes

    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
    {
        /* create passengers array */
        ptNode->passengers = new pass[ptNode->nPass];
        if(ptNode->nPass > maxNodePassengers)
            maxNodePassengers = ptNode->nPass; //setting maximal passenger number
        ptNode->nPass = 0; //set to null for next stage - start array element
    } 

    /* init tour node's passengers array */
    if(maxNodePassengers == 0)
        maxNodePassengers = 1; //minimum for array allocation

    /* fill passengers from file data */
    for(i=0, ptDataPass = prData.passengers; i<prData.nPass; i++, ptDataPass++)
    {
        ptNode = &nodes[ptDataPass->startNode];
        ptPass = &ptNode->passengers[(ptNode->nPass)++]; //next array element
        sprintf(ptPass->name,"P%d",i);
        ptPass->startNode = ptNode;
        ptPass->destinationNode = &nodes[ptDataPass->destinationNode];
        ptPass->budget = ptDataPass->budget;
        if(ptPass->budget > maxBudget)
            maxBudget = ptPass->budget;
        ptPass->tau=TAU_0;
    }

}

void setNodes()
{
    node* ptNode;
    float* ptFloat;
    uint i, j;
    nodes = new node[prData.dim];
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
    {
        sprintf(ptNode->name, "V%d", j);
        ptNode->n = j;
        ptNode->nPass = 0;
        ptNode->v = false;
        ptNode->tau_cars = new float[prData.nCars];
        for(i=0, ptFloat=ptNode->tau_cars; i<prData.nCars; i++, ptFloat++)
            (*ptFloat) = TAU_0;
    }
}

void setCars()
{
    uint j;
    car* ptCar;
    cars = new car[prData.nCars];
    maxCarPassLimit = 0;
    for(j=0, ptCar=cars; j<prData.nCars; j++, ptCar++)
    {
        sprintf(ptCar->name, "C%d", j);
        ptCar->n = j;
        ptCar->tau = TAU_0;
        CalculateCarCostSum(ptCar);
        ptCar->carPassLimit = prData.carPassLimit[ptCar->n];
        ptCar->nPass = 0;
        if(prData.carPassLimit[j] > maxCarPassLimit)
            maxCarPassLimit = prData.carPassLimit[j];
    }
    carPassArray = new pass*[maxCarPassLimit];
}

void setArcs()
{
    uint i, j;
    arc** pptArc;
    arc* ptArc;
    arcs = new arc*[prData.dim];
    for(j=0, pptArc=arcs; j<prData.dim; j++, pptArc++)
    {
        ptArc = *pptArc = new arc[prData.dim];
        for(i=0; i<prData.dim; i++, ptArc++)
        {
            ptArc->row = j;
            ptArc->column = i;
            if(i!=j)
                ptArc->tau = TAU_0;
            else
                ptArc->tau = 0.0;
        }
    }
}

void setAnts()
{
    uint i,j;
    ushort k;
    float* ptFloat;
    ant* ptAnt;
    pass** pptPass;
    tourArc* ptTourArc;
    ants = new ant[N_ANTS];
    for(j=0, ptAnt=ants; j<N_ANTS; j++, ptAnt++)
    {
        ptAnt->t.ant = j;
        ptAnt->t.arcs = new tourArc[prData.dim+1]; //one more for null end 
        for(i=0, ptTourArc=ptAnt->t.arcs; i<prData.dim+1; i++, ptTourArc++)
        {
            ptTourArc->a = nullptr; // arcs in tour (for each node)
            ptTourArc->c = nullptr; // cars in tour (for each node)
            if(i==prData.dim)
                ptTourArc->p = nullptr;
            else
            {
                ptTourArc->p = new pass*[maxCarPassLimit+1]; //passengers in ant's tour nodes
                for(k=0, pptPass=ptTourArc->p; k<maxCarPassLimit+1; k++, pptPass++)
                    (*pptPass) = nullptr;
            } 
        }
        ptAnt->t.carList = new car*[prData.nCars+1]; //one more for null end
        ptAnt->t.passList = new pass*[prData.nPass+1]; //one more for null end
        ptAnt->t.choiceProbList = new float[prData.dim];
        for(i=0, ptFloat=ptAnt->t.choiceProbList; i<prData.dim; i++, ptFloat++)
            (*ptFloat)=0.0;
    }
}

void setBestTour(tour* ptTour)
{
    uint i;
    ushort j;
    float* ptFloat;
    tourArc* ptTourArc;
    car** pptCar;
    pass** pptPass;
    ptTour->arcs = new tourArc[prData.dim+1];
    for(i=0, ptTourArc=ptTour->arcs; i<prData.dim+1; i++, ptTourArc++)
    {
        ptTourArc->a = nullptr; // arcs in tour (for each node)
        ptTourArc->c = nullptr; // cars in tour (for each node)
        if(i==prData.dim)
            ptTourArc->p = nullptr;
        else
        {
            ptTourArc->p = new pass*[maxCarPassLimit+1];
            for(j=0, pptPass=ptTourArc->p; j<maxCarPassLimit+1; j++, pptPass++)
                (*pptPass) = nullptr;
        }
    } 
    ptTour->carList = new car*[prData.nCars+1]; // cars in tour digestion (not for each node)
    ptTour->passList = new pass*[prData.nPass+1];
    for(j=0, pptCar=ptTour->carList; j<prData.nCars+1; j++,pptCar++)
        *pptCar = nullptr;
    for(j=0, pptPass=ptTour->passList; j<prData.nPass+1; j++,pptPass++)
        *pptPass = nullptr;
    ptTour->choiceProbList = new float[prData.dim];
    for(j=0, ptFloat=ptTour->choiceProbList; j<prData.dim; j++, ptFloat++)
        (*ptFloat)=0.0;
    ptTour->cost = numeric_limits<float>::max(); // tour cost to worst
}

void setRollBack()
{
    ushort i;
    pass** pptPass;
    rollBack.lastPassList = new pass*[prData.nPass+1];
    for(i=0, pptPass=rollBack.lastPassList; i<prData.nPass+1; i++, pptPass++)
        *pptPass = nullptr;
}