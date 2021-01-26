#include "ACO.h"

using namespace std;


pass* passengers = nullptr;
node* nodes = nullptr;
car* cars = nullptr;
ant* ants = nullptr;



bool nodeTraversal(node *startNode, ant *currentAnt);

bool Solution(uint iter)
{
    bool success;
    uint antIndex;

    /* add ant loop here */

    for(antIndex = 0; antIndex < 1; antIndex++)
    {
        success = nodeTraversal(&nodes[0], &ants[antIndex]);

    
        cout << "Passed nodes: " << ants[antIndex].nodeCounter << ", success: " << (success?"SUCCESS":"FAIL") << endl;

        for(uint i=0; i<=prData.dim; i++)
        {
            if(ants[0].nodes[i].curNode == nullptr)
                break;
            cout << "Node: " << ants[0].nodes[i].curNode->name << 
            ", prev: " << (ants[0].nodes[i].prevNode==nullptr?"-":ants[0].nodes[i].prevNode->name) <<
            ", next: " << (ants[0].nodes[i].nextNode==nullptr?"-":ants[0].nodes[i].nextNode->name) << endl;
        }
    }
    return true;
}

bool nodeTraversal(node *startNode, ant *currentAnt)
{
    int picked;
    float value;
    node *currentNode;
    car *currentCar;
    
    cout << "Node " << startNode->index << ": " << startNode->name << ", neighbours:" << endl;

    /* start node is visited, and counted as passed */
    currentNode = startNode;
    currentAnt->nodes[currentAnt->nodeCounter].prevNode = nullptr;
    currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;   
    currentAnt->nodeCounter++;
    currentAnt->nodeVisited[currentNode->index] = true;

    /* temporary */
    currentCar = &cars[0];

    /* reset ant price */
     currentAnt->price = 0.0;

    do /* node traversal loop starts here */
    {
        picked = CalculateNodeProbabilities(currentAnt, currentNode, &cars[0]);
        cout << "Picked: " << picked << endl;
        if(picked >= 0)
        {
            currentAnt->price += (float)prData.edgeWeightMatrices[currentCar->index][currentNode->index][nodes[picked].index];
            currentAnt->nodes[currentAnt->nodeCounter].prevNode = currentNode;
            currentNode = nodes+picked;
            currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = currentNode;
            currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;
            currentAnt->nodeCounter++;
            currentAnt->nodeVisited[picked] = true;            
        }
        else //try to connect to start node (if link exists)
        {
            value = prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];
            cout << "Current node: " << currentNode->index << ", last value: " << value << endl;
            if(value != 0.0 && value < 9999)
            {
                currentAnt->price += (float)prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];
                currentAnt->nodes[currentAnt->nodeCounter].prevNode = currentNode;
                currentNode = startNode;
                currentAnt->nodes[currentAnt->nodeCounter-1].nextNode = currentNode;
                currentAnt->nodes[currentAnt->nodeCounter].curNode = currentNode;
                currentAnt->nodes[currentAnt->nodeCounter].nextNode = nullptr;
                currentAnt->nodeCounter++;
            }
        }
        
        cout << "Selected: " << currentNode->index << ": " << currentNode->name << endl;
        cout << "Price: " << currentAnt->price << endl;
        
    } while (picked >= 0);
    cout << "Ant price: " << currentAnt->price << endl;
    if(currentAnt->nodeCounter == prData.dim+1)
        return true;
    return false;
}

bool updatePheromones(ant *bestAnt)
{
    uint i;
    float pheroUpdate;
    antNode *ptAntNode;
    if(bestAnt->nodeCounter <= prData.dim)
        return false;
    if(bestAnt->price>0.001)
        pheroUpdate = 1.0/bestAnt->price;
    else
        return false;
    
    for(i=0, ptAntNode=bestAnt->nodes; i<bestAnt->nodeCounter-1; i++, ptAntNode++)
    {
        ptAntNode->curNode->pheroNeighbours[ptAntNode->nextNode->index]+=pheroUpdate;
    }

    return true;
}

