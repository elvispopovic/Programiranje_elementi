#include "ACO.h"
#include "supervisor.h"

using namespace std;

int main(int argc, char* argv[])
{
    uint iter;
    int bestAnt = 0;
    if(argc < 2)
    {
        cout << "No filename parameter." << endl;
        cout << "Usage: " << argv[0] << " " << "[-d] filename (-tau | -t) tau (-alpha | -a) alpha  (-beta | -b) beta " <<
        "(-ants | -na) nAnts (-iter | -ni) nIter" << endl;
        return 0;
    }
    setParameters(argc, argv);
    if(!loadData(parData.fileName))
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
    for(iter=0; iter < parData.nIter; iter++)
    {
        PheromoneEvaporation();
        Solution(0, &nodes[0]);
        bestAnt = findBestAnt();
        if(bestAnt >= 0)
        {
            updateBestPath(bestAnt);
            updatePheromones(&ants[bestAnt]);
        }

        /*
        uint j,k;
        for(k=0; k<N_ANTS; k++)
        {
            cout << "Ant: " << k << endl;
            for(j=0; j<ants[k].nodeCounter-1; j++)
            {
                uint i;
                cout << "Node " << ants[k].nodes[j].curNode->name << 
                ", price: " << prData.edgeWeightMatrices[0][ants[k].nodes[j].curNode->index][ants[k].nodes[j].nextNode->index] << ":" << endl;
                for(i=0; i<prData.dim-1; i++)
                    cout << ants[k].nodes[j].curNode->pheroNeighbours[i] << ", ";
                cout << ants[k].nodes[j].curNode->pheroNeighbours[i] << endl;
            }
            for(j=0; j<ants[k].nodeCounter-1; j++)
                cout << ants[k].nodes[j].curNode->name << ", ";
            cout << ants[k].nodes[j].curNode->name << ", price: " << ants[k].price << endl;
        }
        */
        cout << "Best ant: " << bestAnt << ", price: " << ants[bestAnt].price << ", best: " << bPath.price << endl;
    }
    cout << "Best path:" << endl;
    for(uint j=0; j<bPath.nodeCounter-1; j++)
    {
        uint i;
        cout << "Node " << bPath.nodes[j].curNode->name << 
        ", price: " << prData.edgeWeightMatrices[0][bPath.nodes[j].curNode->index][bPath.nodes[j].nextNode->index] << ":" << endl;
        for(i=0; i<prData.dim-1; i++)
            cout << bPath.nodes[j].curNode->pheroNeighbours[i] << ", ";
        cout << bPath.nodes[j].curNode->pheroNeighbours[i] << endl;
    }
    cout << "Price: " << bPath.price << endl;



    cout << "Ant simulation ended." << endl;
    cleanup();
    cout << "Init cleanup done." << endl;
    endSupervisor();

    
    /* cleans parser memory space */
    freeData();
    cout << "Loaded data freeing done." << endl;
    return 0;
}







