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
    short carIn;    // car entering node
    short carOut;    // car leaving node
    uint nPickedPassengers; // passengers on board
    pass** pickedPassengers; 

};

/* ant structure */
struct ant
{
    uint nodeCounter;  //passed nodes counter
    antNode *nodes;    //passed nodes
    bool *nodesVisited; //all nodes visited flags
    float price;
};

struct bestPath
{
    uint nodeCounter;
    antNode *nodes;
    float price;
};

/* all ants use it - make it thread safe if ants uses threads */
struct probabilityArrays
{
    uint *nodeCandidatesIndices; //available nodes represented as indices
    float *nodeCandidateProbs;   //available nodes' probabilities
    uint *carCandidateIndices;   //available cars represented as indices
    float *carCandidateProbs;    //available cars' probabilities
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
void PheromoneEvaporation();
bool updatePheromones(ant *bestAnt);
int findBestAnt();
void updateBestPath(uint bestAntIndex);

/* probabilitiy */
int CalculateNodeProbabilities(ant *currentAnt, node *currentNode, car *currentCar);
uint selectFromFreqArray(float sum, uint n, float *probabilities);


/* display part */
void displayProblemData();

#endif // ACO_H_INCLUDED
