#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include <fstream>
#include "ACO.h"

void beginSupervisor(const char* filename);
void endSupervisor();
void writeHeaders();

void writeBestData();
void writeAntsData();
void writeArcsData(uint iter);
void writeCarsData(uint iter);
void writePassData(uint iter);
void writeResult(const char* filename);

#endif