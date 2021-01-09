#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include "ACO.h"

void beginSupervisor(const char* filename);
void endSupervisor();
void writeHeaders();

void writeProblemData();
void writeInitData();

#endif