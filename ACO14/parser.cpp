#include "ACO.h"

using namespace std;

problemData prData;

int parseHeaderToken(const char* name)
{
    uchar length, tokenLength, i;
    const char headerTokens[][48] =
    {
        "NAME","TYPE","COMMENT","DIMENSION","CARS_NUMBER","PASSENGERS_NUMBER","EDGE_WEIGHT",
        "RETURN_RATE","EDGE_WEIGHT_TYPE","EDGE_WEIGHT_FORMAT","EDGE_WEIGHT_SECTION",
        "RETURN_RATE_SECTION","PASSENGERS_LIMIT","ORIGINS_DESTINATIONS_AND_FINANCIAL_LIMITS",
        "EOF"
    };
    tokenLength = sizeof(headerTokens[0]);
    length = sizeof(headerTokens)/tokenLength;
    for(i=0; i<length; i++)
    {
        if(strncmp((char*)name, headerTokens[i], tokenLength)==0)
            return i;
    }
    return -1;
}

bool parseMatrix(ifstream& dataFile, float*** matrix, int nMatrix, int dim)
{
    int i,j,k, res, pos, pos2;
    float *pt1, **pt2, ***pt3;
    string line;
    for(k=0, pt3=matrix; k<nMatrix; k++, pt3++)
    {
        getline(dataFile, line);
        res = stoi(line);
        if(res < 0 || res >= nMatrix)
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
    while(getline(dataFile, line))
    {
        token = parseHeaderToken(line.substr(0, pos=line.find(" : ")).c_str());
        if(pos>0 && (int)line.size() > pos+3)
        {
            element = line.substr(pos+3);
            switch(token)
            {
            case 0: strncpy(prData.name, element.c_str(), element.size());
                    break;
            case 1: strncpy(prData.type, element.c_str(), element.size());
                    break;
            case 2: strncpy(prData.comment, element.c_str(), element.size());
                    break;
            case 3: prData.dim = stoi(element);
                    break;
            case 4: prData.nCars = stoi(element);
                    if(prData.nCars > 0 && prData.dim > 0)
                    {
                        prData.edgeWeightMatrices = new float**[prData.nCars];
                        prData.returnRateMatrices = new float**[prData.nCars];
                        for(j=0; j<prData.nCars; j++)
                        {
                            prData.edgeWeightMatrices[j] = new float*[prData.dim];
                            prData.returnRateMatrices[j] = new float*[prData.dim];
                            for(i=0; i<prData.dim; i++)
                            {
                                prData.edgeWeightMatrices[j][i] = new float[prData.dim];
                                prData.returnRateMatrices[j][i] = new float[prData.dim];
                            }
                        }
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
            case 10:
                parseMatrix(dataFile, prData.edgeWeightMatrices, prData.nCars, prData.dim);
                break;
            case 11:
                parseMatrix(dataFile, prData.returnRateMatrices, prData.nCars, prData.dim);
                break;
            case 12: getline(dataFile, line);
                pos = 0;
                for(i=0, uchPt=prData.carPassLimit; i<prData.nCars; i++, uchPt++)
                {
                    pos2 = line.find(" ", pos);
                    *uchPt = (uchar)stoi(line.substr(pos,pos2-pos));
                    pos=pos2+1;
                }
                break;
            case 13:
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
            case 14:
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