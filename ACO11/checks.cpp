#include "ACO.h"

using namespace std;

bool calculateCostsFromTour(tour* t, float& writtenCost, float& calculatedCost)
{
    uint i, j;
    uchar l;
    tourArc *ptTourArc;
    pass **pptPass;
    car *ptCurrentCar;
    writtenCost = 0.0;
    calculatedCost = 0.0;
    /*
    #ifdef ACO_VERBOSE
    cout << "Calculation (name, car, l, cost, total cost):" << endl;
    #endif
    */
    for(i=0, ptTourArc=t->arcs; i<prData.dim && ptTourArc->a!=nullptr; i++, ptTourArc++)
    {
        writtenCost += ptTourArc->cost+ptTourArc->carReturnCost;
        ptCurrentCar = ptTourArc->c;
        for(l=0, pptPass=ptTourArc->p; (*pptPass)!=nullptr; j++, pptPass++)
        {
            l++;
        }
        l++; //for driver
        calculatedCost+=prData.edgeWeightMatrices[ptCurrentCar->n][ptTourArc->a->row][ptTourArc->a->column]/l+ptTourArc->carReturnCost;
        
        /*
        #ifdef ACO_VERBOSE
        cout << "(" << nodes[ptTourArc->a->row].name << ", " << ptCurrentCar->name << ", " << (int)l << ", " <<
        prData.edgeWeightMatrices[ptCurrentCar->n][ptTourArc->a->row][ptTourArc->a->column]/l+ptTourArc->carReturnCost << ", " << 
        calculatedCost << ")" << endl;
        #endif
        */
    }

    #ifdef ACO_VERBOSE
    cout << "Written cost: " << writtenCost << ", calculated cost: " << calculatedCost << " (written in tour: " << t->cost << ")" << endl;
    #endif
    if(i<prData.dim)
        return false;
    else
        return true;
}