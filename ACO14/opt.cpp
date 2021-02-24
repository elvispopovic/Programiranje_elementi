#include "ACO.h"

using namespace std;

void createOptPath(uint n0, uint n1, ant *ptAnt, short variant);
float calculateAntCost(uint nodeCount, antNode *nodes);
void updateAntOpt(ant *currentAnt, float currentOptPrice);

void opt2_5()
{
    uint i, j, k, v;
    ant *ptAnt;
    float optPrice;
    if(prData.dim < 5 || parData.opt == false)
        return;
    for(k=0, ptAnt=ants; k<parData.nAnts; k++, ptAnt++)
    {
        if(ptAnt->nodeCounter < prData.dim || ptAnt->closedPath == false)
            continue;

        ptAnt->bestOptPrice = numeric_limits<float>::max();
        /*
        cout << "Ant " << k << ", nodes: " << ptAnt->nodeCounter << ", dim: " << prData.dim << ", cost: " << price << endl;
        for(i=0; i<ptAnt->nodeCounter-1; i++)
            cout << ptAnt->nodes[i].curNode->name << "(" << 
            (ptAnt->nodes[i].carIn==nullptr?"-":ptAnt->nodes[i].carIn->name) << ", " << ptAnt->nodes[i].carOut->name << "), ";
        cout << ptAnt->nodes[i].curNode->name << "(" << 
            (ptAnt->nodes[i].carIn==nullptr?"-":ptAnt->nodes[i].carIn->name) << ", " << ptAnt->nodes[i].carOut->name << ")" << endl;
        */
        for(j=0; j<ptAnt->nodeCounter-3; j++)
        {
            //cout << "j: " << j << ", granica: " << (j==0?ptAnt->nodeCounter-1:ptAnt->nodeCounter) << endl;
            for(i=j+3; i<(j==0?ptAnt->nodeCounter-1:ptAnt->nodeCounter); i++)
            {
                for(v=0; v<2; v++)
                {
                    //cout << "j: " << j << ", i: " << i << ", v: " << v << endl;
                    createOptPath(j,i, ptAnt, v);
                    optPrice = calculateAntCost(ptAnt->nodeCounter, ptAnt->optNodes);
                    //cout << "Price: Ant: " << ptAnt->price << ", opt: " << optPrice << (optPrice<ptAnt->price?" BETTER":"") << endl;
                    if(optPrice < ptAnt->bestOptPrice)
                        updateAntOpt(ptAnt, optPrice);
                }
            }
        }
    }
}

void createOptPath(uint n0, uint n1, ant *ptAnt, short variant)
{
    uint i;
    antNode *ptAntNode1, *ptAntNode2;
    /* nodes bypass */
    for(i=0, ptAntNode1=ptAnt->nodes, ptAntNode2=ptAnt->optNodes; i<=n0; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    ptAntNode1++;
    i++;
    for(;i<=n1; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    memcpy(ptAntNode2++, (ptAnt->nodes)+n0+1, sizeof(antNode));
    for(;i<ptAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    /* update links */
    for(i=0, ptAntNode1=ptAnt->optNodes, ptAntNode2=ptAnt->optNodes+1; i<ptAnt->nodeCounter-1; i++, ptAntNode1++, ptAntNode2++)
    {
        ptAntNode1->nextNode = ptAntNode2->curNode;
        ptAntNode2->prevNode = ptAntNode1->curNode;
    }
    ptAntNode1->nextNode = ptAnt->optNodes->curNode;
    /* cars bypass */
    ptAnt->optNodes[n0+1].carIn=ptAnt->optNodes[n0].carOut;
    /* shifted one left, n1 becomes n1-1, n1+1 becomes n1 */
    if(variant==0)
    {
        ptAnt->optNodes[n1].carIn=ptAnt->optNodes[n1].carOut=ptAnt->optNodes[n1-1].carOut; 
        if(n1+1<(ptAnt->nodeCounter))
            ptAnt->optNodes[n1+1].carIn=ptAnt->optNodes[n1].carOut;
    }
    else
    {
        ptAnt->optNodes[n1].carIn=ptAnt->optNodes[n1].carOut;
        if(n1+1<(ptAnt->nodeCounter))
            ptAnt->optNodes[n1].carOut=ptAnt->optNodes[n1+1].carIn;
        else
            ptAnt->optNodes[n1].carOut=ptAnt->nodes[ptAnt->nodeCounter-1].carOut;
    }
}

float calculateAntCost(uint nodeCount, antNode *nodes)
{
    uint i;
    antNode *ptAntNode;
    node *carPickNode;
    float price = 0.0;
    if(nodeCount == 0)
        return price;
    for(i=0, ptAntNode=nodes, carPickNode=nodes->curNode; i<nodeCount; i++, ptAntNode++)
    {
        price += prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index];
        if(i>0 && ptAntNode->carOut->index != ptAntNode->carIn->index)
        {
            price+=prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index];
            carPickNode = ptAntNode->curNode;
        }
    }
    price+=prData.returnRateMatrices[(ptAntNode-1)->carOut->index][nodes->curNode->index][carPickNode->index];
    return price;
}

void updateAntOpt(ant *currentAnt, float currentOptPrice)
{
    uint i;
    antNode *ptAntNode1, *ptAntNode2;
    for(i=0, ptAntNode1=currentAnt->optNodes, ptAntNode2=currentAnt->bestOptNodes; i<currentAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    currentAnt->bestOptPrice = currentOptPrice;
    currentAnt->optNodeCounter = currentAnt->nodeCounter;
}

