#include "ACO.h"
#include "supervisor.h"

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

    /* opens output data file, writes header */
    beginSupervisor("./outputData/stat");
    writeHeaders();
    /* main ACO loop */
    for(i=0; i<N_ITER; i++)
    {
        PheromoneEvaporation();
        Solution(i); //solution, starting from arbitrary node
        UpdatePheromoneTrails();
        /* supervisor */
        
        writeAntsData();
        writeArcsData(i);
        writeCarsData(i);
        writePassData(i);
        writeBestData();
        //if(i%2 == 1)
        {
            displayBestTour(&localBestCostTour);
        }
    }
    cout << "Global best tour" << endl;
    displayBestTour(&globalBestCostTour);
    writeResult("./outputData/results");
    /* close output data file */
    endSupervisor();
    /* cleans algorithm memory space */
    Cleanup();
    /* cleans parser memory space */
    freeData();
    return 0;
}







