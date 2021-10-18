#include <xready/xready.h>
#include <cstdio>
#include <string>
#include <xready/data_flow.h>

using namespace std;

int ffc_dataflow_exec(
    int argc,
    char *argv[],
    void ** inputs,
    long *inputs_size,
    void ** outputs,
    long *outpus_size,
    void *constants,
    int number_of_const
);

void run_dataflow_cpu(
    DataFlow *df,
    void ** inputs,
    long *inputs_size,
    void ** outputs,
    long *outpus_size,
    void *constants,
    int number_of_const);
