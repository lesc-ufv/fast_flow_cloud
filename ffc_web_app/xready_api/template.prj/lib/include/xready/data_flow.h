#ifndef DATAFLOW_H
#define DATAFLOW_H

#include <cstdlib>
#include <string>

#include <xready/ffc_args.h>
#include <xready/ddata_flow.h>
#include <xready/cgra.h>
#include <xready/cgra_arch.h>
#include <xready/scheduler.h>

class DataFlow {

private:
    std::string exec_type;
    std::string name;

    DDataFlow *df;
    int run_on_cgra();

public:

    DataFlow(std::string json_name);

    ~DataFlow();

    void setInputData(int id, short * data, long size);

    void setOutputData(int id, short * data, long size);

    void setConstants(short * constants, int size);

    void run();

};


#endif //DATAFLOW_H
