#include "ACO.h"
#include "supervisor.h"

using namespace std;

tour localBestCostTour, globalBestCostTour;
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

/* max-min pheromone limits */
float tau_max = TAU_0;
float tau_min = 0.0;

float invDim;
float invNPass=0;
float invMaxCarPassLimit=0.0; //max car passenger capacity (all cars)

std::mt19937* mersenneGenerator;

/* main solution loop */
bool Solution(uint iter)
{
    register uint i;
    uint  k;
    int startNode;
    register node *ptNode;
    register car *ptCar;
    ant* ptAnt;
    float carCost, arcCost, passCost;
    uint counter;
    uniform_int_distribution<int> distribution(0,prData.dim-1);
    //startNode = distribution(*mersenneGenerator);
    startNode = 0;
    
    for(k=0, ptAnt = ants; k<N_ANTS; k++, ptAnt++)
    {
        counter = 0;

        #ifdef ACO_VERBOSE
        cout << "Iteration: " << iter << ", ant: " << k << ", start node: " << startNode << endl;
        #endif
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
        /* reset ant best flag */
        ptAnt->status = 0;

        /* reset ant tour, set first elements to nullptr only */
        ptAnt->t.arcs[0].a = nullptr;
        ptAnt->t.arcs[0].c = nullptr;
        ptAnt->t.carList[0] = nullptr;
        ptAnt->t.ant = k;
        ptAnt->t.iteration = iter;
        ptAnt->t.rollbacks = 0;

        nodes[startNode].v = true; //start node visited
        /* separate costs: cars change cost and travelling cost */
        counter = nodeTraversal(ptAnt, startNode, arcCost, carCost);

        /* if tour found and cost is not 0 */
        if(counter == prData.dim && arcCost > 0.0)
        {
            if(solvePassengers(ptAnt, passCost))
            {
                #ifdef ACO_VERBOSE
                cout << "Passenger solution O.K. " << endl;
                cout << "Car cost: " << carCost << endl;
                cout << "Tour cost (with passengers): " << (carCost+passCost) << ", cost (without passengers): " << (carCost+arcCost) << endl;
                #endif
                /* no car cost in passenger calculations. Same for arc calculations (without passengers) */
                ptAnt->t.costNoPass = carCost+arcCost;
                ptAnt->t.cost = carCost+passCost; //copy cost of tour into ant's tour
            }
            else
            {
                #ifdef ACO_VERBOSE
                cout << "Passenger solution failed." << endl;
                #endif
                ptAnt->t.cost = numeric_limits<float>::max(); // most expensive tour
                ptAnt->status = -1;
            }
        }
        else
        {
            #ifdef ACO_VERBOSE
            cout << "Tour not found." << endl;
            #endif
            ptAnt->t.cost = numeric_limits<float>::max(); // most expensive tour
        }
        #ifdef ACO_VERBOSE
        cout << "------------------------ (eoa)" << endl;
        #endif
    }//end of ant
    return true;
}


/* node traversal for one ant */
uint nodeTraversal(ant* currentAnt, uint startNode, float& arcCost, float& carCost)
{
    uint n1, n2, currentNodeIndex=startNode, counter=0;
    uint remainNodes = prData.dim;
    float c, *antChoices;
    car* currentCar = nullptr; //no car selected yet

    car** antCars;
    tourArc* antTour;

    arcProb* arcSelected;
    carProb* carSelected;
    antTour = currentAnt->t.arcs;
    antCars = currentAnt->t.carList;
    antChoices = currentAnt->t.choiceProbList;
    arcCost = carCost = 0.0;
    uniform_int_distribution<int> distributionCar(1, prData.nCars-1);
    while(1) //graph traversal
    {
        antTour->a = nullptr; //ant tour end
        antTour->c = nullptr; //ant tour end
        n1 = CalculateCarProbabilities(currentCar, &nodes[currentNodeIndex], remainNodes, distributionCar(*mersenneGenerator));
        if(n1 == 0) //no cars
            break;
        carSelected = CarSelect(n1);
        if(carSelected == nullptr) //no cars in selection
            break;

        /* change car if possible. Do not change car in very last node (return node)  */ 
        if(counter < prData.dim)
        {
            c = changeCar(currentCar, antCars, carSelected->c, currentNodeIndex);
            antTour->carReturnCost = c;
            carCost += c;
        }

        /* arcs */
        n2 = CalculateArcProbabilities(arcs[currentNodeIndex], currentCar); //whole matrix row as argument
        if(n2==0) // no arcs
            break;

        arcSelected = ArcSelect(n2);
        if(arcSelected == nullptr) //no arcs in selection
            break;

        *(antChoices++) = (float)(n2*n1);
  
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

/* transfers data from best ant to best cost tour (wheter local or global)*/
void updateBestCostTour(ant* ptAnt, tour* ptTour)
{
    uint i;
    ushort j;
    float *ptFloat1, *ptFloat2;
    car **pptCar1, **pptCar2;
    pass **pptPass1, **pptPass2;
    tourArc *ptTourArc1, *ptTourArc2;
 
    ptTour->ant = ptAnt->t.ant;
    ptTour->iteration = ptAnt->t.iteration;
    ptTour->tauMax = ptAnt->t.tauMax;
    ptTour->tauMin = ptAnt->t.tauMin;
    ptTour->fc = ptAnt->t.fc;
    ptTour->rollbacks = ptAnt->t.rollbacks;

    /* update arc and car list in best tour arcs */
    for(i=0, ptTourArc1=ptAnt->t.arcs, ptTourArc2=ptTour->arcs; i<=prData.dim && ptTourArc1->a !=nullptr; i++, ptTourArc1++, ptTourArc2++)
    {
        ptTourArc2->a = ptTourArc1->a; //best tour tour arc
        ptTourArc2->c = ptTourArc1->c; //best tour tour car
        ptTourArc2->cost = ptTourArc1->cost; //best tour arc cost
        ptTourArc2->carReturnCost = ptTourArc1->carReturnCost; //best tour arc car return cost
        for(pptPass1 = ptTourArc1->p, pptPass2 = ptTourArc2->p; (*pptPass1)!=nullptr; pptPass1++, pptPass2++)
            (*pptPass2) = (*pptPass1);
        (*pptPass2) = nullptr;
    }
    ptTourArc2->a = nullptr;
    ptTourArc2->c = nullptr;

    /* update external tour car list */
    for(i=0, pptCar1=ptAnt->t.carList, pptCar2=ptTour->carList; i<prData.nCars && *pptCar1!=nullptr; i++, pptCar1++, pptCar2++)
    {
        *pptCar2 = *pptCar1; //best tour car
    }
    *pptCar2 = nullptr;

    /* update external passengers' list */
    for(j=0, pptPass1=ptAnt->t.passList, pptPass2=ptTour->passList; j<prData.nPass && *pptPass1!=nullptr; j++, pptPass1++, pptPass2++)
    {
        *pptPass2 = *pptPass1; //list of passengers
    }
    *pptPass2 = nullptr;

    /* update choice probability list */
    for(j=0, ptFloat1=ptAnt->t.choiceProbList, ptFloat2=ptTour->choiceProbList; j<prData.dim; j++, ptFloat1++, ptFloat2++)
    {
        *ptFloat2 = *ptFloat1;
    }
    ptTour->cost = ptAnt->t.cost;
    ptTour->costNoPass = ptAnt->t.costNoPass;

}

/* when pheromone max-min range changes or evaporation occurs,
   we update all arcs and cars
   Also, we don's apply max-min for passengers 'cos they have been resetting
   in each iteration 
*/
void updateAllTrails()
{
    node *ptNode;
    arc **pptArc, *ptArc;

    float *ptFloat;
    uint i, j;

    /* all cars picked in nodes */
    for(j=0, ptNode = nodes; j < prData.dim; j++, ptNode++)
        for(i=0, ptFloat=ptNode->tau_cars; i<prData.nCars; i++, ptFloat++)
        {
            if(*ptFloat > tau_max)
                (*ptFloat) = tau_max;
            if(*ptFloat < tau_min)
                (*ptFloat) = tau_min;
        }

    /* all arcs */
    for(j=0, pptArc=arcs; j<prData.dim; j++, pptArc++)
        for(i=0, ptArc=*pptArc; i<prData.dim; i++, ptArc++)
        {
            if(ptArc->tau > tau_max)
                ptArc->tau = tau_max;
            else if(ptArc->tau < tau_min)
                ptArc->tau = tau_min;
        }
        
    /* all cars */
    /*
    for(j=0, ptCar = cars; j<prData.nCars; j++, ptCar++)
    {
        if(ptCar->tau > tau_max) 
            ptCar->tau = tau_max;
        else if(ptCar->tau < tau_min) 
            ptCar->tau = tau_min;
    }
    */
}

/* pheromone evaporation for arcs and cars */
void PheromoneEvaporation()
{
    register uint i, j;
    node *ptNode;
    arc **pptArc, *ptArc;
    float *ptFloat;
    /* arc select */
    for(j=0, pptArc=arcs; j<prData.dim; j++, pptArc++)
        for(i=0, ptArc=*pptArc; i<prData.dim; i++, ptArc++)
            ptArc->tau = (1.0-RHO) * (ptArc->tau);
    /* car select */
    /*
    for(j=0, ptCar=cars; j<prData.nCars; j++, ptCar++)
        ptCar->tau = (1.0-RHO) * (ptCar->tau);
    */
    /* car in particular node select */
    for(j=0, ptNode = nodes; j < prData.dim; j++, ptNode++)
        for(i=0, ptFloat=ptNode->tau_cars; i<prData.nCars; i++, ptFloat++)
            (*ptFloat) = (1.0-RHO) * (*ptFloat);

    updateAllTrails();
}

void UpdatePheromoneTrails()
{
    register uint j;
    float sum, fc, *ptFloat;
    ant *ptAnt, *ptBestAnt;
    node *ptNode;
    tourArc* ptTourArc;
    car **pptCar, *ptCar;
    
    /* best ant only deposes pheromons and updates best cost tour */
    for(j=0, ptBestAnt = ptAnt = ants; j<N_ANTS; j++, ptAnt++)
    {
        if(ptAnt->t.cost < ptBestAnt->t.cost)
            ptBestAnt = ptAnt;
    }
    ptBestAnt->status = 1;
    fc = 1.0/ptBestAnt->t.cost;

    /* global best ant changes min and max */
    if(ptBestAnt->t.cost < globalBestCostTour.cost ) 
    {
        tau_max = 1.0/RHO*fc; //fc, cost still divided
        for(j=0, sum=0.0, ptBestAnt->nRtProb=1.0, ptFloat=ptBestAnt->t.choiceProbList; j<prData.dim; j++, ptFloat++)
        {
            sum += (*ptFloat);
            ptBestAnt->nRtProb *= pow(1.0/(*ptFloat),1.0/prData.dim);
        }
        ptBestAnt->avg = sum/prData.dim;
        tau_min = tau_max*(1.0-ptBestAnt->nRtProb)/((ptBestAnt->avg-1.0)*ptBestAnt->nRtProb);
        if(tau_min < 0.0)
            tau_min = 0.0;
    }

    /* best ant's min and max */
    ptBestAnt->t.tauMin = tau_min;
    ptBestAnt->t.tauMax = tau_max;
    ptBestAnt->t.fc = fc;

    /* now, update pheromones on best ant tour */
    /* arcs on tour */
    for(ptTourArc=ptBestAnt->t.arcs, ptCar=nullptr, ptFloat = nullptr; ptTourArc->a != nullptr; ptTourArc++)
    {
        ptTourArc->a->tau += fc; //pheromone deposit
        if(ptTourArc->a->tau > tau_max)
            ptTourArc->a->tau = tau_max;
        else if(ptTourArc->a->tau < tau_min)
            ptTourArc->a->tau = tau_min;

        /* update node car picks pheromone */
        ptNode = &nodes[ptTourArc->a->row];
        if(ptTourArc->c != ptCar) // changed car
        {
            ptCar = ptTourArc->c;
            ptFloat = &(ptNode->tau_cars[ptCar->n]);          
        }
        if(ptFloat != nullptr)
        {
            (*ptFloat) += fc;
            if(*ptFloat > tau_max)
                (*ptFloat) = tau_max;
            else if(*ptFloat < tau_min)
                (*ptFloat) = tau_min; 
        } 
    }
    /* for each 100 iteration update pheromone for global best tour */
    if((ptBestAnt->t.iteration % GLOB_BEST_PHERO_UPDATE) == (GLOB_BEST_PHERO_UPDATE-1))
        for(ptTourArc = globalBestCostTour.arcs, ptCar=nullptr, ptFloat = nullptr; ptTourArc->a != nullptr; ptTourArc++)
        {
            ptTourArc->a->tau += fc; //pheromone deposit
            ptNode = &nodes[ptTourArc->a->row];
            if(ptTourArc->c != ptCar)
            {
                ptCar = ptTourArc->c;
                ptFloat = &(ptNode->tau_cars[ptCar->n]);
            }
            if(ptFloat != nullptr)
            {
                (*ptFloat) += fc;
                if(*ptFloat > tau_max)
                    (*ptFloat) = tau_max;
                else if(*ptFloat < tau_min)
                    (*ptFloat) = tau_min;
            }
        }

    /* cars on tour */
    for(pptCar=ptBestAnt->t.carList; *pptCar != nullptr; pptCar++)
    {
        (*pptCar)->tau += fc; //pheromone deposit
        if((*pptCar)->tau > tau_max)
            ((*pptCar)->tau) = tau_max;
        else if((*pptCar)->tau < tau_min) 
            ((*pptCar)->tau) = tau_min;
    }

    /* if max-min limits changes, we must update all arcs and cars 
        and best tour also
    */
    if(ptBestAnt->t.cost < globalBestCostTour.cost ) 
    {
        updateAllTrails();
        updateBestCostTour(ptBestAnt, &globalBestCostTour);
    }
    /* update local best tour also */
    updateBestCostTour(ptBestAnt, &localBestCostTour);
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






