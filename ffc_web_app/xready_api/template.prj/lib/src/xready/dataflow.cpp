#include <xready/dataflow.h>

Dataflow::Dataflow(std::string json_name){

    int argc = FFC::getInstance()->getArgc();
    char **argv = FFC::getInstance()->getArgv();

    Dataflow::exec_type = "cpu";
    Dataflow::name = name;
    std::string json_path = json_name + ".json";
    std::string kernel_name;
    std::string fpgaBinaryFile;
    std::string cgra_arch;

    if(argc > 1){
        Dataflow::exec_type = argv[1];
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

    Dataflow::cgraArch = new CgraArch(cgra_arch);
    Dataflow::scheduler = new Scheduler(cgraArch);
    Dataflow::cgraHw = new Cgra(fpgaBinaryFile,kernel_name);
    int num_thr = cgraArch->getNumThreads();
    for(int t = 0; t < num_thr;++t){
        auto df = new DDataFlow(t,json_name);
        df->fromJSON(json_path);
        Dataflow::data_flows.push_back(df);
    }
}

Dataflow::~Dataflow() {
    delete cgraArch;
    delete scheduler;
    delete cgraHw;
}

void Dataflow::setInputData(int id, short * data, long size){
    int num_thr = Dataflow::cgraArch->getNumThreads();
    size_t sz_per_thread = size/num_thr;
    size_t sz_rest = size%num_thr;
    size_t s;
    for(int t = 0; t < num_thr;++t){
        auto op = Dataflow::data_flows[t]->getInOp(id);
        if(t == num_thr-1){
            s = sz_per_thread + sz_rest;
        }else{
            s  = sz_per_thread;
        }
        op->setData(&data[t*sz_per_thread],s);
    }
}

void Dataflow::setOutputData(int id, short * data, long size){
    int num_thr = Dataflow::cgraArch->getNumThreads();
    size_t sz_per_thread = size/num_thr;
    size_t sz_rest = size%num_thr;
    size_t s;
    for(int t = 0; t < num_thr;++t){
        auto op = Dataflow::data_flows[t]->getOutOp(id);
        if(t == num_thr-1){
            s = sz_per_thread + sz_rest;
        }else{
            s  = sz_per_thread;
        }
        op->setData(&data[t*sz_per_thread],s);
    }
}

void Dataflow::setConstants(short * constants, int size){
    int num_thr = Dataflow::cgraArch->getNumThreads();
    for(int t = 0; t < num_thr;++t){
        for(int i=0; i < size;i++)
           Dataflow::data_flows[t]->getCOp(i)->setConst(constants[i]);
    }
}

void Dataflow::run(){

    if(Dataflow::exec_type == "cpu"){
        int len = Dataflow::data_flows.size();
        #pragma omp parallel for
        for(int i = 0;i < len;++i){
                Dataflow::data_flows[i]->compute();
        }
    }else if(Dataflow::exec_type == "sim" || Dataflow::exec_type == "cgra"){
        Dataflow::run_on_cgra();
    }
    printf("--split--\n");
}

int Dataflow::run_on_cgra() {
    int num_thr = cgraArch->getNumThreads();
    int tries = 0;
    int r;
    for (int i = 0; i < num_thr; ++i) {
        Dataflow::scheduler->addDataFlow(Dataflow::data_flows[i], i, 0);
        Dataflow::cgraArch->getNetBranch(i)->createRouteTable();
        Dataflow::cgraArch->getNet(i)->createRouteTable();
    }
    do {
        r = Dataflow::scheduler->scheduling();
        tries++;
    } while (r != SCHEDULE_SUCCESS && tries < 10000);


    if (r == SCHEDULE_SUCCESS) {
        auto cgra_program = Dataflow::cgraArch->getCgraProgram();
        Dataflow::cgraHw->loadCgraProgram(cgra_program);
        auto num_in = Dataflow::data_flows[0]->getNumInputs();
        auto num_out = Dataflow::data_flows[0]->getNumOutputs();
        for (int m = 0; m < num_thr; ++m) {
            auto df = Dataflow::data_flows[m];
            for(int i = 0;i < num_in;++i){
                auto op = df->getInOp(i);
                Dataflow::cgraHw->setCgraProgramInputStreamByOp(op);
            }
            for(int i = 0;i < num_out;++i){
                auto op = df->getOutOp(i);
                Dataflow::cgraHw->setCgraProgramOutputStreamByOp(op);
            }
        }
        Dataflow::cgraHw->syncExecute();
    } else {
        printf("Scheduler Error: %s\n",Dataflow::scheduler->getMessageError(r).c_str());
    }

    return r;
}
