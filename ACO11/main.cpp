#include "ACO.h"

using namespace std;

int main(int argc, char* argv[])
{
    uint i;
    if(argc < 2)
    {
        cout << "No filename parameter." << endl;
        cout << "Usage: " << argv[0] << " " << "filename" << endl;
        return 0;
    }
    if(!loadData(argv[1]))
    {
        cout << "Data file error." << endl;
        return 0;
    }
    //displayProblemData();
    Init();
    //displayNodesAndPassengers();

    #ifdef ACO_VERBOSE
    cout << "ACO VERBOSE defined." << endl;
    #endif

    /* main ACO loop */
    for(i=0; i<N_ITER; i++)
    {
        PheromoneEvaporation();
        Solution(i); //solution, starting from arbitrary node
        UpdatePheromoneTrails();
        
        //if(i%2 == 1)
        {
            displayBestTour();
        }
    }


    Cleanup();
    freeData();
    return 0;
}







