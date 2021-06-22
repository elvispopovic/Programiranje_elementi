#ifndef ACO_H
#define ACO_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <random>
#include <limits>
#include <algorithm>

/* default only - if value from command line not available */
#define TAU_0 100.0 //initial pheromone trail
#define RHO 0.1 //pheromone evaporation
#define ALPHA 2 //pheromone exponent >= 0
#define BETA 1 //heuristic exponent >= 1
#define N_ANTS 5 //number of ants
#define N_ITER 10000 //number of iterations

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

enum symmetry:unsigned char {SYMMETRIC, ASYMMETRIC};

struct dataPass;    //passengers from readed data

/* parameters */
struct parameters
{
    uint argc;
    float tau; //initial pheromone trail
    float rho;
    float alpha, beta;
    uint nAnts;
    uint nIter;
    bool writeData;
    bool opt;
    char fileName[128];
};


/* file data structure */
struct problemData
{
    char name[32]; //problem name
    char type[12]; //type
    char comment[64]; //comment
    uint dim; //dimensionality (number of nodes)
    ushort nCars; //number of cars
    ushort nPass; //number of passengers
    symmetry edgeWeight; //symmetry of edge weight matrices
    symmetry returnRate; //symmetry of return rate matrices
    float*** edgeWeightMatrices = nullptr; //array of edge weight matrices
    float*** returnRateMatrices = nullptr; //array of return rate matrices
    uchar* carPassLimit = nullptr; //array of car capacities
    dataPass* passengers = nullptr; //array of passengers (file data)
    /* not in original paper */
    float rentFee = 0.0;
    /* used by ants during passengers pick */
    uint maxNodePassengers = 0;
    uint maxCarPassengers = 0;
};

/* file data passenger structure */
struct dataPass
{
    uint startNode;
    uint destinationNode;
    float budget;
};

/* algorithm passengers structure */
struct pass
{
    char name[6];
    uint index;
    uint startNode;
    uint endNode;
    float phero;
};

/* problem node structure - static */
struct node
{
    char name[6];
    uint index;
    float *pheroNeighbours;
    float *pheroCars;
    uint nPassengers;
    pass** passengers;
};

struct car
{
    char name[6];
    uint index;
    uchar carPassLimit;
};

/* ant node structure - dynamic */
struct antNode
{
    node *curNode;     // pointer to problem node
    node *prevNode, *nextNode; // previous and next node
    car *carIn, *carOut;
    float prob;
    float choices; 
};

struct contextNode
{
    uint index;
    node *currentNode;
};

/* ant structure */
struct ant
{
    uint nodeCounter;  //passed nodes counter
    antNode *nodes;    //passed nodes
    bool closedPath;   //if this ant achieved closed path
    float bestOptPrice;
    bool *nodesVisited; //all nodes visited flags
    bool *carsRented;   //all cars rented flags
    node *carPickedNode; // where a particular car has been picked
    pass **passOnBoard = nullptr; //passengers on board (current car)
    uint optNodeCounter; //number of opt nodes
    antNode *optNodes;  //array of opt nodes
    antNode *bestOptNodes; // array of best opt nodes
    contextNode *contextNodes; //array of context nodes (during passengers calculation)
    float price;
    float pheroUpdate;
    bool *passPicked = nullptr;   //temporary during node traversal - current node picked passengers flags
};

struct bestPath
{
    uint iteration;
    uint nodeCounter;
    antNode *nodes;
    uint optNodeCounter;
    antNode *optNodes;
    float price;
    float optPrice;
    float pheroMin;
    float pheroMax;
    float pheroRatio;
};

/* all ants use it - make it thread safe if ants uses threads */
struct probabilityArrays
{
    uint n;
    uint *indices; // elements indices (prob. arrays have elements and their probabilities)
    int selected; // 
    float *probs; // probabilities
    float *cumulatives; // cumulative probabilities
    float sum;
};


extern std::mt19937* mersenneGenerator;
extern parameters parData;
extern problemData prData;
extern probabilityArrays probArrays;
extern bestPath bPath;
/* declaration in ACO.cpp */
extern node* nodes;
extern car* cars;
extern pass* passengers;
extern ant* ants;




/* parser functions */
bool setParameters(int argc, char** argv);
bool loadData(const char* filename);
void freeData();
int parseHeaderToken(const char* name);
bool parseMatrix(std::ifstream& dataFile, float*** matrix, int nMatrix, int dim);


/* initializer */
void init();
void cleanup();

/* algorithm part */
bool Solution(uint iter, node *startNode);
void opt2_5();
void calculatePassengers(ant *currentAnt);
void PheromoneEvaporation();
bool updatePheromones(ant *bestAnt);
void limitPheromoneTraces();
ant* findBestAnt();
bool updateBestPath(uint iteration, ant *bestAnt);
float calculatePathCost(antNode *nodes, uint nodeCounter);

/* probabilitiy */
int PickNode(ant *currentAnt, node *currentNode, car *currentCar);
int PickCar(ant *currentAnt, node *currentNode, car *currentCar);
int PickPassengers(node *currentNode, uint availablePlaces, bool *passPicked);
uint selectFromFreqArray(float sum, uint n, float *probabilities);
void calculateMaxMin();


/* display part */
void displayProblemData();
void displayBestPath();

#endif // ACO_H_INCLUDED
