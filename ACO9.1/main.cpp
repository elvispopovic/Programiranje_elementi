#include "ACO.h"

using namespace std;

int main(int argc, char* argv[])
{
    int i;
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

    /* main ACO loop */
    for(i=0; i<N_ITER; i++)
    //for(i=0; 1; i++)
    {
        PheromoneEvaporation();
        Solution(0);
        UpdatePheromoneTrails();
        
        if(i%50 == 0)
        {
            cout << "Iteration: " << i << endl;
            displayBestTour();
        }
    }


    Cleanup();
    freeData();
    return 0;
}







