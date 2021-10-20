#include <xready/data_flow.h>

DataFlow::DataFlow(std::string json_name){

    int argc = FFC::getInstance()->getArgc();
    char **argv = FFC::getInstance()->getArgv();

    DataFlow::exec_type = "cpu";
    DataFlow::name = name;
    std::string json_path = json_name + ".json";
    std::string kernel_name;
    std::string fpgaBinaryFile;
    std::string cgra_arch;

    if(argc > 1){
        DataFlow::exec_type = argv[1];
    }
    if(argc > 2){
        kernel_name = argv[2];
    }
    if(argc > 3){
        fpgaBinaryFile = argv[3];
    }
    if(argc > 4){
        cgra_arch = argv[4];
    }

    DataFlow::cgraArch = new CgraArch(cgra_arch);
    DataFlow::cgraHw = new Cgra(fpgaBinaryFile,kernel_name);
    DataFlow::scheduler = new Scheduler(cgraArch);

    int num_thr = cgraArch->getNumThreads();
    for(int t = 0; t < num_thr;++t){
        auto df = new DDataFlow(t,json_name);
        df->fromJSON(json_path);
        DataFlow::data_flows.push_back(df);
    }
}

DataFlow::~DataFlow() {

}

void DataFlow::setInputData(int id, short * data, long size){
    int num_thr = DataFlow::cgraArch->getNumThreads();
    size_t sz_per_thread = size/num_thr;
    size_t sz_rest = size%num_thr;
    size_t s;
    for(int t = 0; t < num_thr;++t){
        auto op = DataFlow::data_flows[t]->getInOp(id);
        if(t == num_thr-1){
            s = sz_per_thread + sz_rest;
        }else{
            s  = sz_per_thread;
        }
        op->setData(&data[t*sz_per_thread],s);
    }
}

void DataFlow::setOutputData(int id, short * data, long size){
    int num_thr = DataFlow::cgraArch->getNumThreads();
    size_t sz_per_thread = size/num_thr;
    size_t sz_rest = size%num_thr;
    size_t s;
    for(int t = 0; t < num_thr;++t){
        auto op = DataFlow::data_flows[t]->getOutOp(id);
        if(t == num_thr-1){
            s = sz_per_thread + sz_rest;
        }else{
            s  = sz_per_thread;
        }
        op->setData(&data[t*sz_per_thread],s);
    }
}

void DataFlow::setConstants(short * constants, int size){
    int num_thr = DataFlow::cgraArch->getNumThreads();
    for(int t = 0; t < num_thr;++t){
        for(int i=0; i < size;i++)
           DataFlow::data_flows[t]->getCOp(i)->setConst(constants[i]);
    }
}

void DataFlow::run(){

    if(DataFlow::exec_type == "cpu"){
        for(auto d : DataFlow::data_flows)
            d->compute();

    }else if(DataFlow::exec_type == "sim" || DataFlow::exec_type == "cgra"){
        DataFlow::run_on_cgra();
    }
    printf("--split--\n");
}

int DataFlow::run_on_cgra() {
    int num_thr = cgraArch->getNumThreads();
    int tries = 0;
    int r;
    for (int i = 0; i < num_thr; ++i) {
        DataFlow::scheduler->addDataFlow(DataFlow::data_flows[i], i, 0);
        DataFlow::cgraArch->getNetBranch(i)->createRouteTable();
        DataFlow::cgraArch->getNet(i)->createRouteTable();
    }
    do {
        r = DataFlow::scheduler->scheduling();
        tries++;
    } while (r != SCHEDULE_SUCCESS && tries < 10000);


    if (r == SCHEDULE_SUCCESS) {
        auto cgra_program = DataFlow::cgraArch->getCgraProgram();
        DataFlow::cgraHw->loadCgraProgram(cgra_program);

        auto num_in = DataFlow::data_flows[0]->getNumInputs();
        auto num_out = DataFlow::data_flows[0]->getNumOutputs();

        auto in_op = DataFlow::data_flows[0]->getInOp(0);
        auto data_size = (size_t) in_op->getSize();
        auto data_per_thread = data_size / num_thr;
        int data_per_thread_rest = data_size % num_thr;
        for (int m = 0; m < num_thr; ++m) {
            auto df = DataFlow::data_flows[m];
            for(int i = 0;i < num_in;++i){
                auto op = df->getInOp(i);
                DataFlow::cgraHw->setCgraProgramInputStreamByOp(op);
            }
            for(int i = 0;i < num_out;++i){
                auto op = df->getOutOp(i);
                DataFlow::cgraHw->setCgraProgramOutputStreamByOp(op);
            }
        }
        DataFlow::cgraHw->syncExecute();
    } else {
        printf("Scheduler Error: %s\n",DataFlow::scheduler->getMessageError(r).c_str());
    }

    return r;
}
