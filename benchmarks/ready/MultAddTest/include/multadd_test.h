#ifndef MULTADD_H
#define MULTADD_H

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <omp.h>

#include <xready/xready.h>

using namespace std;
using namespace std::chrono;

#define DATA_SIZE (1 << 10)
#define NUM_THREAD (8)
#define SAMPLES (1)

int multadd(int idx);

int multadd_openmp(int idx);

int multadd_cgra(std::string fpgaBinaryFile,std::string kernel_name, int idx, int copies);

void multadd_dataflow_cpu();

DataFlow *createDataFlow(int id, int copies);

int main(int argc, char *argv[]);

#endif //MULTADD_H_H
