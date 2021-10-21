#ifndef DATAFLOW_H
#define DATAFLOW_H

#include <cstdlib>
#include <string>

#include <xready/ffc_args.h>
#include <xready/ddata_flow.h>
#include <xready/cgra.h>
#include <xready/cgra_arch.h>
#include <xready/scheduler.h>

class Dataflow {

private:
    std::string exec_type;
    std::string name;
    CgraArch *cgraArch;
    Cgra *cgraHw;
    Scheduler *scheduler;
    std::vector<DDataFlow *> data_flows;
    int run_on_cgra();

public:

    Dataflow(std::string json_name);

    ~Dataflow();

    void setInputData(int id, short * data, long size);

    void setOutputData(int id, short * data, long size);

    void setConstants(short * constants, int size);

    void run();

};


#endif //DATAFLOW_H
