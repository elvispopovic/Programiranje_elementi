#include "ACO.h"

using namespace std;


void displayProblemData()
{
    cout << prData.name << ", type: " << prData.type << ", nodes: " << prData.dim << ", cars: " << prData.nCars << endl;
    cout << "Iters: " << parData.nIter << ", ants: " << parData.nAnts << ", rho: " << parData.rho << 
    ", alpha: " << parData.alpha << ", beta: " << parData.beta << endl;
    cout << "Switches: " << (parData.opt?"-opt ":"") << (parData.writeData?"-write ":"") << endl; 
}

void displayBestPath()
{
    cout << "it. " << bPath.iteration << ", price: " << bPath.price << ", opt. price: " << bPath.optPrice << 
    ", phero Min: " << bPath.pheroMin << ", phero Max: " << bPath.pheroMax << ", ratio: " << bPath.pheroRatio << endl;
}

