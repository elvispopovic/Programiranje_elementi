#include "ACO.h"
#include "supervisor.h"

using namespace std;

int main(int argc, char* argv[])
{
    bool result;
    uint iter;
    ant *bestAnt;
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

    displayProblemData();
    writeProblemData();
    cout << "Problem data written." << endl;
    init();
    writeInitData();
    cout << "Init data written" << endl;
    cout << "Ant simulation will commence..." << endl;

    if(parData.writeData == true)
        writeHeaders();

    /* put ACO loop here */
    for(iter=0; iter < parData.nIter; iter++)
    {
        PheromoneEvaporation();
        Solution(0, &nodes[0]);
        opt2_5();
        bestAnt = findBestAnt();
        if(bestAnt != nullptr)
        {
            result = updateBestPath(iter, bestAnt);
            if(result)
            {
                calculateMaxMin();
                displayBestPath();
            }
            updatePheromones(bestAnt);
            limitPheromoneTraces();
            if(parData.writeData == true)
                writeBestData(iter, bestAnt);  
        }
        

  /*      
        uint i, j,k;
        cout << "Iter: " << iter << endl;
        for(k=0; k<parData.nAnts; k++)
        {
            cout << "Ant: " << k << endl;
            
            for(j=0; j<ants[k].nodeCounter-1; j++)
            {
                uint i;
                cout << "Node " << ants[k].nodes[j].curNode->name << ", car " << ants[k].nodes[j].carOut->name <<
                ", price: " << prData.edgeWeightMatrices[0][ants[k].nodes[j].curNode->index][ants[k].nodes[j].nextNode->index] << ":" << endl;
                cout << "Neighbours' pheromones:" << endl;
                for(i=0; i<prData.dim-1; i++)
                    cout << ants[k].nodes[j].curNode->pheroNeighbours[i] << ", ";
                cout << ants[k].nodes[j].curNode->pheroNeighbours[i] << endl;
                cout << "Cars' pheromones:" << endl;
                for(i=0; i<prData.nCars-1; i++)
                    cout << ants[k].nodes[j].curNode->pheroCars[i] << ", ";
                 cout << ants[k].nodes[j].curNode->pheroCars[i] << endl;
            }
        
           cout << "Ant " << k << ": nodecounter: " << ants[k].nodeCounter << ", price: " << ants[k].price << endl;
            for(j=0; j<ants[k].nodeCounter-1; j++)
                cout << ants[k].nodes[j].curNode->name << ", ";

            cout << ants[k].nodes[j].curNode->name << ", price: " << ants[k].price << endl;
        }
        
        cout << "Best ant: " << bestAnt << ", price: " << ants[bestAnt].price << ", best: " << bPath.price << endl;
    */
    }
    writeResult();
  

    cout << "Ant simulation ended." << endl;
    cleanup();
    cout << "Init cleanup done." << endl;
    endSupervisor();

    
    /* cleans parser memory space */
    freeData();
    cout << "Loaded data freeing done." << endl;
    return 0;
}







