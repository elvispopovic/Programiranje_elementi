#include "ACO.h"
#include "supervisor.h"

using namespace std;

int main(int argc, char* argv[])
{
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
    cout << "Data loaded." << endl;
    beginSupervisor("./outputData/stat");

    writeProblemData();
    cout << "Problem data written." << endl;
    init();
    writeInitData();
    cout << "Init data written" << endl;
    cout << "Ant simulation will commence..." << endl;

    /* put ACO loop here */
    Solution(0);
    updatePheromones(&ants[0]);

    for(uint j=0; j<ants[0].nodeCounter-1; j++)
    {
        uint i;
        cout << "Node " << ants[0].nodes[j].curNode->name << ":" << endl;
        for(i=0; i<prData.dim-1; i++)
            cout << ants[0].nodes[j].curNode->pheroNeighbours[i] << ", ";
        cout << ants[0].nodes[j].curNode->pheroNeighbours[i] << endl;
    }


    cout << "Ant simulation ended." << endl;
    cleanup();
    cout << "Init cleanup done." << endl;
    endSupervisor();

    
    /* cleans parser memory space */
    freeData();
    cout << "Loaded data freeing done." << endl;
    return 0;
}







