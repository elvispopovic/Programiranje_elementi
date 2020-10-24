#ifndef ACO_H_INCLUDED
#define ACO_H_INCLUDED

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <random>
#include <limits>
#include <algorithm>

#define TAU_0 0.1 //initial pheromone trail
#define RHO 0.01 //pheromone evaporation
#define ALPHA 0.5 //pheromone exponent >= 0
#define BETA 1.0 //heuristic exponent >= 1
#define N_ANTS 2
#define N_ITER 2
#define PASS_ITER 3
#define CAR_PERSIST 10 //current car probability factor


typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

enum symmetry:unsigned char {SYMMETRIC, ASYMMETRIC};

struct dataPass;
struct pass;

/* file data structure */
struct problemData
{
    char name[32]; //name od problem
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

/* graph node */
struct node
{
    uint n;
    char name[8];
    bool v; //visited
    ushort nPass=0; //passengers in node
    pass* passengers; //fixed passengers (belong to nodes)  
};

/* graph arc */
struct arc
{
    uint row;
    uint column;
    float tau; //pheromone deposition
};

/* car structure */
struct car
{
    char name[10];
    ushort n;
    uchar carPassLimit; //max passengers in car
    uchar nPass; //current passengers in car
    float tau; //pheromone deposition
    float costSum; // estimated max cost
    float invCostSum; //inverse (1/costSum)
    bool rented;
    uint rentedNode;
};

/* file data passenger structure */
struct dataPass
{
    uint startNode;
    uint destinationNode;
    float budget;
};

/* algorithm passenger structure */
struct pass
{
    char name[10];
    node* startNode;
    node* destinationNode;
    float tau; //pheromone deposition
    float budget;
    float currentBudget;
};

/* arc probaility array element */
struct arcProb
{
    arc* a; //selected arc
    float p; //probability
};

/* car probaility array element */
struct carProb
{
    car* c; //selected arc
    float p; //probability
};

struct passProb
{
    pass* pa; //selected passenger
    float p; //probability
};

/* arc, car and pass list pointer triad on tour */
struct tourArc
{
    arc* a; //arc pointer to one arc
    car* c; //car pointer to one car
    pass** p = nullptr; //array of pointers to passengers currently in the car
    float cost; //shared cost
    float carReturnCost;
};


struct rollback
{
    bool dontPick; //after some failed iterations won't pick any passenger in that node
    bool rollbacked;
    uint counter; //iterations counter
    tourArc* lastTourArc; //last success arc and car (with zero passengers on board and zero shortage)
    pass** lastPassList = nullptr; //passengers in last success arc
    float lastCost; //cost in last success arc
};

/* used in ants as tour and as best cost tour */
struct tour
{
    float cost; //cost with passengers
    float costNoPass; //cost without passengers
    tourArc* arcs = nullptr;
    car** carList = nullptr;
    pass** passList = nullptr;
};

/* ant structure */
struct ant
{
    tour t;
};

/* extern variables and arrays */
extern problemData prData;
extern uchar maxCarPassLimit;
extern uchar maxNodePassengers;
extern float maxBudget;

extern node* nodes;
extern arc** arcs; //matrix dim x dim
extern ant* ants;
extern car* cars;
extern pass** carPassArray; //passengers in current car

extern rollback rollBack;

extern float* selProbArray; //probability distribution frequencies array
extern carProb* carProbArray;
extern arcProb* arcProbArray;
extern passProb* passProbArray;

/* ant pheromone potential */
extern float feroQ; //ant pheromone potential

/* inverse variables */
extern float invDim;
extern float invNPass;
extern float invMaxCarPassLimit;

extern tour bestCostTour;
extern std::mt19937* mersenneGenerator;

/* parser functions */
int parseHeaderToken(const char* name);
bool parseMatrix(std::ifstream& dataFile, float*** matrix, int nMatrix, int dim);
bool loadData(const char* filename);
void freeData();

/* initialization and cleanup */
void Init();
void Cleanup();

/* solution part */
bool Solution(uint startNode);
uint nodeTraversal(ant* currentAnt, uint startNode, float& arcCost, float& carCost);
bool nodeTraversal2(ant* currentAnt, float& cost); 
float changeCar(car*& currentCar, car**& antCars, car* newCar, uint currentNodeIndex);
void updateBestCostTour(ant* ptAnt);
bool solvePassengers(ant *currentAnt, float& cost);


/* pheromone trail dynamics */
void PheromoneEvaporation();
void UpdatePheromoneTrails();

/* probabilities for nodes, cars, passengers */
uint selectFromFreqArray(float sum, uint n);
uint CalculateCarProbabilities(car* currentCar, uint remainNodes);
uint CalculateArcProbabilities(arc* arcs, car* currentCar);

/* selecting arcs, cars, passengers */
carProb* CarSelect(uint n);
arcProb* ArcSelect(uint n);

/* costs */
void CalculateCarCostSum(car* c);



/* display part */
void displayProblemData();
void displayNodes();
void displayBestTour();

#endif // ACO_H_INCLUDED
