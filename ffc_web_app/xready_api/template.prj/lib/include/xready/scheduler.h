#ifndef CGRASCHEDULER_SCHELUDER_H
#define CGRASCHEDULER_SCHELUDER_H

#include <map>
#include <queue>

#include <xready/cgra_arch.h>
#include <xready/global.h>
#include <xready/scheduler_defs.h>
#include <xready/ddata_flow.h>

class Scheduler {

private:
    std::map<int, DDataFlow *> dataflows;

    std::map<int, int> dataflow_group;

    std::map<int, std::vector<int>> data_flow_mapping;

    CgraArch *cgraArch;

    int mapAndRoute(int ThreadId);

    int placeAndRoute(std::vector<int> &mapping, int threadID);

public:
    explicit Scheduler(CgraArch *cgra);

    Scheduler();

    ~Scheduler();

    bool addDataFlow(DDataFlow *df, int threadID, int groupID);

    void setCgra(CgraArch *cgra);

    CgraArch *getCgra();

    int scheduling();

    void reset();

    std::vector<int> getMapping(int dataFlowId, int threadID);
    
    static std::string getMessageError(int code);
    
};


#endif //CGRASCHEDULER_SCHELUDER_H
