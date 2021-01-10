#ifndef ACO_H
#define ACO_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <random>
#include <limits>
#include <algorithm>

#define TAU_0 100.0 //initial pheromone trail, to large value
#define RHO 0.1 //pheromone evaporation
#define ALPHA 1 //pheromone exponent >= 0
#define BETA 1 //heuristic exponent >= 1
#define N_ANTS 17 //number of ants
#define N_ITER 1000 //number of iterations

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

enum symmetry:unsigned char {SYMMETRIC, ASYMMETRIC};

struct dataPass;    //passengers from readed data


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

/* algorithm node structure */
struct node
{
    char name[6];
    uint index;
    bool visited;
    float *pheroNeighbours;
    float *pheroCars;
    uint nPassengers;
    pass** passengers;
};




extern std::mt19937* mersenneGenerator;
extern problemData prData;
extern node* nodes;
extern pass* passengers;



/* parser functions */
int parseHeaderToken(const char* name);
bool parseMatrix(std::ifstream& dataFile, float*** matrix, int nMatrix, int dim);
bool loadData(const char* filename);
void freeData();


/* initializer */
void init();
void cleanup();


/* display part */
void displayProblemData();

#endif // ACO_H_INCLUDED