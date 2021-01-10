#include "ACO.h"

using namespace std;

char headerTokens[][48] =
{
    "NAME","TYPE","COMMENT","DIMENSION","CARS_NUMBER","PASSENGERS_NUMBER","EDGE_WEIGHT",
    "RETURN_RATE","EDGE_WEIGHT_TYPE","EDGE_WEIGHT_FORMAT","EDGE_WEIGHT_SECTION",
    "RETURN_RATE_SECTION","PASSENGERS_LIMIT","ORIGINS_DESTINATIONS_AND_FINANCIAL_LIMITS",
    "EOF"
};

problemData prData;

int parseHeaderToken(const char* name);
bool parseMatrix1(ifstream& dataFile, float*** matrix, int nMatrix, int dim);
bool parseMatrix2(ifstream& dataFile, float*** matrix, int dim);
void allocateMatrices(int nCars, int dim);
void fillReturnRates();


bool loadData(const char* filename)
{
    uint i, j;
    string line, element;
    uchar* uchPt;
    dataPass* ptPass;
    int token, pos, pos2;
    ifstream dataFile(filename);
    if(!dataFile.is_open())
        return false;
    prData.nCars = 0;   // if nCars unchanged, then data type no cars
    prData.nPass = 0;
    while(getline(dataFile, line))
    {
        /* try to find token with border ": " */
        token = parseHeaderToken(line.substr(0, pos=line.find(": ")).c_str());
        if(token==-1) // not found, so try " : "
            token = parseHeaderToken(line.substr(0, pos=line.find(" : ")).c_str());
        if(pos>0 && (int)line.size() > pos+2)
        {
            element = line.substr(pos+2);
            switch(token)
            {
            case 0: strncpy(prData.name, element.c_str(), element.size());
                    break;
            case 1: strncpy(prData.type, element.c_str(), element.size());
                    break;
            case 2: strncpy(prData.comment, element.c_str(), element.size());
                    break;
            case 3: prData.dim = stoi(element); // dimension
                    break;
            case 4: prData.nCars = stoi(element); // cars number
                    if(prData.nCars > 0 && prData.dim > 0)
                    {
                        allocateMatrices(prData.nCars, prData.dim);
                        prData.carPassLimit = new uchar[prData.nCars];
                    }
                    break;
            case 5: prData.nPass = stoi(element);
                    if(prData.nPass > 0)
                        prData.passengers = new dataPass[prData.nPass];
                    break;
            case 6: if(strncmp(element.c_str(), "SYMMETRIC", 10)==0)
                        prData.edgeWeight = symmetry::SYMMETRIC;
                    else
                        prData.edgeWeight = symmetry::ASYMMETRIC;
            case 7: if(strncmp(element.c_str(), "SYMMETRIC", 10)==0)
                        prData.returnRate = symmetry::SYMMETRIC;
                    else
                        prData.returnRate = symmetry::ASYMMETRIC;
            case 8: //edge weight type and edge weight format we don't use
            case 9: break;
            }
        }
        else if(pos < 0 && (int)line.find(" ")==-1)
            switch(token)
            {
            case 10:    // edge weight 
                if(prData.nCars > 0) // if we have car Number data
                    parseMatrix1(dataFile, prData.edgeWeightMatrices, prData.nCars, prData.dim);
                else                // if not we set car number as 1 and use different matrix parser
                {
                    prData.nCars = 1;
                    allocateMatrices(prData.nCars, prData.dim);
                    parseMatrix2(dataFile, prData.edgeWeightMatrices, prData.dim);
                }
                break;
            case 11:    // return rates
                if(prData.nCars > 0)
                    parseMatrix1(dataFile, prData.returnRateMatrices, prData.nCars, prData.dim);
                break;
            case 12:    // passenger limit
                getline(dataFile, line);
                pos = 0;
                for(i=0, uchPt=prData.carPassLimit; i<prData.nCars; i++, uchPt++)
                {
                    pos2 = line.find(" ", pos);
                    *uchPt = (uchar)stoi(line.substr(pos,pos2-pos));
                    pos=pos2+1;
                }
                break;
            case 13: // origins destination and financial limits
                for(j=0, ptPass=prData.passengers; j<prData.nPass; j++, ptPass++)
                {
                    pos = 0;
                    getline(dataFile, line);
                    pos2 = line.find(" ",pos);
                    ptPass->startNode = (uint)stoi(line.substr(pos,pos2-pos));
                    pos = pos2+1;
                    pos2 = line.find(" ",pos);
                    ptPass->destinationNode = (uint)stoi(line.substr(pos,pos2-pos));
                    pos = pos2+1;
                    ptPass->budget = stof(line.substr(pos));
                }
                break;
            case 14: // eof
                dataFile.close();
                return true;
            }
    };
    dataFile.close();
    return false;
}

void freeData()
{
    uint i, j;
    if(prData.edgeWeightMatrices != nullptr)
    {
        for(j=0; j<prData.nCars; j++)
        {
            for(i=0; i<prData.dim; i++)
            {
                delete[] prData.edgeWeightMatrices[j][i];
                delete[] prData.returnRateMatrices[j][i];
            }
            delete[] prData.edgeWeightMatrices[j];
            delete[] prData.returnRateMatrices[j];
        }
        delete[] prData.edgeWeightMatrices;
        delete[] prData.returnRateMatrices;
    }
    if(prData.carPassLimit != nullptr)
        delete[] prData.carPassLimit;
    if(prData.passengers != nullptr)
        delete[] prData.passengers;
}

int parseHeaderToken(const char* name)
{
    uchar length, tokenLength, i;

    tokenLength = sizeof(headerTokens[0]);
    length = sizeof(headerTokens)/tokenLength;
    for(i=0; i<length; i++)
    {
        if(strncmp((char*)name, headerTokens[i], tokenLength)==0)
            return i;
    }
    return -1;
}

/* parsing matrices in CARSP formatu */
bool parseMatrix1(ifstream& dataFile, float*** matrix, int nMatrix, int dim)
{
    int i,j,k, res, pos, pos2;
    float *pt1, **pt2, ***pt3;
    string line;
    for(k=0, pt3=matrix; k<nMatrix; k++, pt3++)
    {
        getline(dataFile, line);
        res = stoi(line);               //ucitava broj matrice (automobili)
        if(res < 0 || res >= nMatrix)   //mora biti manje od broja matrica
            return false;
        for(j=0, pt2=*pt3; j<dim; j++, pt2++)
        {
            getline(dataFile, line);
            pos=0;
            while(line[pos]==' ')
                pos++;
            for(i=0, pt1=*pt2; i<dim; i++, pt1++)
            {
                pos2 = line.find(" ", pos);
                *pt1 = (float)stoi(line.substr(pos,pos2-pos));
                pos=pos2+1;
            }
        }
    }
    return true;
}

/* parsiranje matrice u TSPLIB formatu */
bool parseMatrix2(ifstream& dataFile, float*** matrix, int dim)
{
    string line;
    int pos, pos2, n, n1, row, column;
    n1 = dim*dim;
    n = 0;
    row = 0;
    column = 0;
    do
    {
        getline(dataFile, line);
        pos = 0;
        do
        {
            while(line[pos]==' ') //leading spaces
                pos++;
            pos2 = line.find(" ", pos); //find space on other side
            if(pos2 ==-1) // if not found, try find end of line
                pos2 = line.find("\n", pos);
            matrix[0][row][column] = (float)stoi(line.substr(pos,pos2-pos));
            n++;
            column++;
            if(column == dim)
            {
                row++;
                column = 0;
            }
            if(pos2 >= (int)line.length()-1)
                break;
            pos=pos2+1;
        }while(pos > 0);
    }while(n<n1);
    return true;
}

void fillReturnRates()
{
    uint i, j;
    for(j=0; j < prData.dim; j++)
        for(i=0; i < prData.dim; i++)
            prData.returnRateMatrices[0][j][i]=0.0;
}

void allocateMatrices(int nCars, int dim)
{
    uint i, j;
    prData.edgeWeightMatrices = new float**[nCars];
    prData.returnRateMatrices = new float**[nCars];
    for(j=0; j<prData.nCars; j++)
    {
        prData.edgeWeightMatrices[j] = new float*[dim];
        prData.returnRateMatrices[j] = new float*[dim];
        for(i=0; i<prData.dim; i++)
        {
            prData.edgeWeightMatrices[j][i] = new float[dim];
            prData.returnRateMatrices[j][i] = new float[dim];
        }
    }
}