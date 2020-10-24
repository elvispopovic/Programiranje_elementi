#include "ACO.h"

using namespace std;

tour bestCostTour;
uchar maxCarPassLimit = 0;
uchar maxNodePassengers=0;
float maxBudget = 0.0;

node* nodes = nullptr;
arc** arcs = nullptr;
ant* ants = nullptr;
car* cars = nullptr;
pass** carPassArray = nullptr;

/* probability calculation arrays */
float* selProbArray = nullptr;
carProb* carProbArray = nullptr;
arcProb* arcProbArray = nullptr;
passProb* passProbArray = nullptr;


float feroQ; //ant pheromone potential

float invDim;
float invNPass=0;
float invMaxCarPassLimit=0.0; //max car passenger capacity (all cars)
float invCarAndPassPheroNorm=0.001;

std::mt19937* mersenneGenerator;

/* main solution loop */
bool Solution(uint startNode)
{
    register uint i;
    uint  j, k;
    register node *ptNode;
    register car *ptCar;
    pass* ptPass;
    ant* ptAnt;
    float carCost, arcCost, passCost;
    uint counter;
    if(startNode<0 || startNode>=prData.dim)
        return false;

    for(k=0, ptAnt = ants; k<N_ANTS; k++, ptAnt++)
    {
        counter = 0;
        /* reset car rented and nPassengers*/
        for(i=0, ptCar=cars; i<prData.nCars; i++, ptCar++)
        {
            ptCar->rented = false;
            ptCar->nPass = 0;
        }
            
        /* reset node visited */
        for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
        {
            ptNode->v = false;
        }

        /* reset ant tour, set first elements to nullptr only */
        ptAnt->t.arcs[0].a = nullptr;
        ptAnt->t.arcs[0].c = nullptr;
        ptAnt->t.carList[0] = nullptr;

        nodes[startNode].v = true; //start node visited
        /* separate costs: cars change cost and travelling cost */
        counter = nodeTraversal(ptAnt, startNode, arcCost, carCost);


        /* if tour found and cost is not 0 */
        if(counter == prData.dim && arcCost > 0.0)
        {
            /* reset passengers' pheromone trail */
            for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
                for(i=0, ptPass=ptNode->passengers; i<ptNode->nPass; i++, ptPass++)
                    ptPass->tau = 0.0;
            if(solvePassengers(ptAnt, passCost))
            {
                cout << "Passenger solution O.K. " << endl;
                cout << "Car cost: " << carCost << endl;
                cout << "Tour cost (without passengers): " << (carCost+arcCost) << ", cost (with passengers): " << (carCost+passCost) << endl;
                ptAnt->t.cost = carCost+passCost; //copy cost of tour into ant's tour
                
            }
            else
            {
                cout << "Passenger solution failed." << endl;
                ptAnt->t.cost = numeric_limits<float>::max(); // most expensive tour
            }
        }
        else
        {
            cout << "Tour not found." << endl;
            ptAnt->t.cost = numeric_limits<float>::max(); // most expensive tour
        }
        cout << "=====================" << endl;
    }//end of ant
    return true;
}


/* node traversal for one ant */
uint nodeTraversal(ant* currentAnt, uint startNode, float& arcCost, float& carCost)
{
    uint n, currentNodeIndex=startNode, counter=0;
    uint remainNodes = prData.dim;
    car* currentCar = nullptr; //no car selected yet

    car** antCars;
    tourArc* antTour;

    arcProb* arcSelected;
    carProb* carSelected;
    antTour = currentAnt->t.arcs;
    antCars = currentAnt->t.carList;
    arcCost = carCost = 0.0;
    while(1) //graph traversal
    {
        antTour->a = nullptr; //ant tour end
        antTour->c = nullptr; //ant tour end
        n = CalculateCarProbabilities(currentCar, remainNodes);
        if(n==0) //no cars
            break;
        carSelected = CarSelect(n);
        if(carSelected == nullptr) //no cars in selection
            break;

        /* change car if possible. Do not change car in very last node (return node)  */ 
        if(counter < prData.dim)
            carCost += changeCar(currentCar, antCars, carSelected->c, currentNodeIndex);

        /* arcs */
        n = CalculateArcProbabilities(arcs[currentNodeIndex], currentCar); //whole matrix row as argument
        if(n==0) // no arcs
            break;

        arcSelected = ArcSelect(n);
        if(arcSelected == nullptr) //no arcs in selection
            break;
    
        remainNodes--;
        antTour->a = arcSelected->a;
        antTour->c = currentCar;
    
        arcCost += prData.edgeWeightMatrices[currentCar->n][arcSelected->a->row][arcSelected->a->column]; // isto i u feromonskom updateu

        currentNodeIndex = arcSelected->a->column;
        nodes[currentNodeIndex].v = true;
        if(counter==prData.dim-2)
            nodes[startNode].v = false;
        counter++;
        antTour++;
    }
    antTour->a = nullptr; //current limit
    antTour->c = nullptr; //current limit
    return counter;
}

/* transfers data from best ant to best cost tour */
void updateBestCostTour(ant* ptAnt)
{
    register uint i;
    register car **pptCar1, **pptCar2;
    tourArc *ptTourArc1, *ptTourArc2;
 
    //if(cost < bestCostTour.bestCost)
    if(ptAnt->t.cost < bestCostTour.cost) //lower cost than best cost tour and ant is sucessful
    {
        for(i=0, ptTourArc1=ptAnt->t.arcs, ptTourArc2=bestCostTour.arcs; i<=prData.dim && ptTourArc1->a !=nullptr; i++, ptTourArc1++, ptTourArc2++)
        {
            ptTourArc2->a = ptTourArc1->a; //best tour tour arc
            ptTourArc2->c = ptTourArc1->c; //best tour tour car
        }

        ptTourArc2->a = nullptr;
        ptTourArc2->c = nullptr;
        for(i=0, pptCar1=ptAnt->t.carList, pptCar2=bestCostTour.carList; i<prData.nCars && *pptCar1!=nullptr; i++, pptCar1++, pptCar2++)
        {
            *pptCar2 = *pptCar1; //best tour car
        }
        *pptCar2 = nullptr;
        bestCostTour.cost = ptAnt->t.cost;
    }
}

void PheromoneEvaporation()
{
    register uint i, j;
    register arc **pptArc, *ptArc;
    car *ptCar;
    /* arc select */
    for(j=0, pptArc=arcs; j<prData.dim; j++, pptArc++)
        for(i=0, ptArc=*pptArc; i<prData.dim; i++, ptArc++)
            ptArc->tau = (1.0-RHO) * (ptArc->tau);
    /* car select */
    for(j=0, ptCar=cars; j<prData.nCars; j++, ptCar++)
        ptCar->tau = (1.0-RHO) * (ptCar->tau);

}

void UpdatePheromoneTrails()
{
    register int j;
    ant *ptAnt, *ptBestAnt;
    tourArc* ptTourArc;
    register car** pptCars;
    
    /* best ant only deposes pheromons and updates best cost tour */
    for(j=0, ptBestAnt = ptAnt = ants; j<N_ANTS; j++, ptAnt++)
    {
        if(ptAnt->t.cost < ptBestAnt->t.cost)
            ptBestAnt = ptAnt;
    }
    updateBestCostTour(ptBestAnt);
    
    for(ptTourArc=ptBestAnt->t.arcs; ptTourArc->a != nullptr; ptTourArc++)
    {
        /* 
        * arcs pheromone trails
        * pheromone strength divided by arc length
        * arc length is in car matrix (dim1), row(dim2), column(dim3)
        */
        ptTourArc->a->tau += feroQ/prData.edgeWeightMatrices[ptTourArc->c->n][ptTourArc->a->row][ptTourArc->a->column]; 
    }
    /* cars pheromone trails */
    for(pptCars=ptBestAnt->t.carList; *pptCars!=nullptr; pptCars++)
        (*pptCars)->tau += feroQ*invCarAndPassPheroNorm;  //???
      

}

arcProb* ArcSelect(uint n)
{
    register uint i;
    uint selection;
    register float sum, *ptFloat;
    arcProb* ptArcProb;
    if(n<1 || n>prData.dim)
        return nullptr;
    for(i=0, ptFloat=selProbArray, sum=0.0, ptArcProb=arcProbArray; i<n; i++, ptArcProb++, ptFloat++)
    {
        *ptFloat = sum;
        sum+=ptArcProb->p;
    }  
    selection = selectFromFreqArray(sum, n);
    return &arcProbArray[selection];
}






