#include "ACO.h"

using namespace std;

void setNodes(); //nodes setup
void setCars(); //cars setup
void setArcs(); // node arcs setup
void setAnts(); // ant structure setup
void setPassengers(); // passenger structure setup
void setInverseVariables(); // inverse variables (to avoid divisions during algorithm execution)

void setBestTour();

void Init()
{
    cout << "Init..." << endl;
    mt19937::result_type seed = time(0);
    mersenneGenerator = new mt19937(seed);

    feroQ = 0.0;
    /* cars */
    setCars(); //updates feroQ also (car cost sum)

    feroQ=feroQ*100/(2.0*prData.nCars); //pheromone strength: average car cost
    cout << "FeroQ: " << feroQ << endl;

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
    setBestTour();

    /* inverse variables */
    setInverseVariables();

    cout << "Init done." << endl;
}

void Cleanup()
{
    uint i;
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
            delete[] ptNode->passengers;
        delete[] nodes;
    }
    if(carPassArray != nullptr)
        delete[] carPassArray;

       
    /* best cost tour */
    if(bestCostTour.arcs!=nullptr)
        delete[] bestCostTour.arcs; // tour arcs
    if(bestCostTour.carList!=nullptr)
        delete[] bestCostTour.carList; // tour cars

    if(selProbArray!=nullptr)
        delete[] selProbArray;
    if(arcProbArray!=nullptr)
        delete[] arcProbArray;
    if(carProbArray!=nullptr)
        delete[] carProbArray;
    if(passProbArray!=nullptr)
        delete[] passProbArray;

    /* ants */
    if(ants!=nullptr)
    {
        for(i=0, ptAnt = ants; i<N_ANTS; i++, ptAnt++)
        {
           delete[] ptAnt->t.arcs;
           delete[] ptAnt->t.carList;
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
        ptPass->tau=0.0;
    }

}

void setNodes()
{
    node* ptNode;
    uint j;
    nodes = new node[prData.dim];
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
    {
        sprintf(ptNode->name, "V%d", j);
        ptNode->n = j;
        ptNode->nPass = 0;
        ptNode->v = false;
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
        feroQ += ptCar->costSum;
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
    uint j;
    ant* ptAnt;
    ants = new ant[N_ANTS];
    for(j=0, ptAnt=ants; j<N_ANTS; j++, ptAnt++)
    {
        ptAnt->t.arcs = new tourArc[prData.dim+1]; //one more for null end 
        ptAnt->t.carList = new car*[prData.nCars+1]; //one more for null end
    }
}

void setBestTour()
{
    uint i;
    ushort j;
    tourArc* ptTourArc;
    car** pptCar;
    bestCostTour.arcs = new tourArc[prData.dim+1];
    for(i=0, ptTourArc=bestCostTour.arcs; i<prData.dim+1; i++, ptTourArc++)
    {
        ptTourArc->a = nullptr; // arcs in tour (for each node)
        ptTourArc->c = nullptr; // cars in tour (for each node)
    } 
    bestCostTour.carList = new car*[prData.nCars+1]; // cars in tour digestion (not for each node)
    for(j=0, pptCar=bestCostTour.carList; j<prData.nCars+1; j++,pptCar++)
        *pptCar = nullptr;
    bestCostTour.cost = numeric_limits<float>::max(); // tour cost
}