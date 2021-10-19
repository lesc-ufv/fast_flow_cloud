#include <xready/data_flow.h>

DataFlow::DataFlow(std::string json_name): name(std::move(json_name)){

    int argc = FFC::getInstance()->getArgc();
    char **argv = FFC::getInstance()->getArgv();

    DataFlow::exec_type = "cpu";
    std::string json_path = name + ".json";

    if(argc > 1){
        DataFlow::exec_type = argv[1];
    }

    DataFlow::df = new DDataFlow(0,DataFlow::name);
    DataFlow::df->fromJSON(json_path);
}

DataFlow::~DataFlow() {
    delete DataFlow::df;
}

void DataFlow::setInputData(int id, short * data, long size){
    auto op = DataFlow::df->getInOp(id);
    op->setData(data,size);
}

void DataFlow::setOutputData(int id, short * data, long size){
    auto op = DataFlow::df->getOutOp(id);
    op->setData(data,size);
}

void DataFlow::setConstants(short * constants, int size){
    for(int i=0; i < size;i++)
        DataFlow::df->getCOp(i)->setConst(constants[i]);
}

void DataFlow::run(){

    if(DataFlow::exec_type == "cpu"){
        DataFlow::df->compute();
    }else if(DataFlow::exec_type == "sim"){
        DataFlow::run_on_cgra();
    }else if(DataFlow::exec_type == "cgra"){
        DataFlow::run_on_cgra();
    }
}

int DataFlow::run_on_cgra() {
    int argc = FFC::getInstance()->getArgc();
    char **argv = FFC::getInstance()->getArgv();

    std::string kernel_name;
    std::string fpgaBinaryFile;
    std::string cgra_arch;

    if(argc > 2){
        kernel_name = argv[2];
    }
    if(argc > 3){
        fpgaBinaryFile = argv[3];
    }
    if(argc > 4){
        cgra_arch = argv[4];
    }

    auto cgraArch = new CgraArch(cgra_arch);
    auto cgraHw = new Cgra(fpgaBinaryFile,kernel_name);
    auto scheduler = new Scheduler(cgraArch);

    int num_thr = cgraArch->getNumThreads();
    int tries = 0;
    int r;
    for (int i = 0; i < num_thr; ++i) {
        auto clone = DataFlow::df;
        clone->setId(i);
        scheduler->addDataFlow(clone, i, 0);
        cgraArch->getNetBranch(i)->createRouteTable();
        cgraArch->getNet(i)->createRouteTable();
    }

    do {
        r = scheduler->scheduling();
        tries++;
    } while (r != SCHEDULE_SUCCESS && tries < 10000);


    if (r == SCHEDULE_SUCCESS) {
        auto cgra_program = cgraArch->getCgraProgram();
        cgraHw->loadCgraProgram(cgra_program);

        auto num_in = DataFlow::df->getNumInputs();
        auto num_out = DataFlow::df->getNumOutputs();

        auto in_op = DataFlow::df->getInOp(0);
        auto data_size = (size_t) in_op->getSize();
        auto data_per_thread = data_size / num_thr;
        int data_per_thread_rest = data_size % num_thr;
        for (int m = 0; m < num_thr; ++m) {
            for(int i = 0;i < num_in;++i){
                auto op = DataFlow::df->getInOp(i);
                auto data_ptr = op->getData();
                auto size = data_per_thread;
                if(m==num_thr-1)
                    size += data_per_thread_rest;
                cgraHw->setCgraProgramInputStreamByID(DataFlow::df->getId(),op->getId(), &data_ptr[m*data_per_thread], size);
            }
            for(int i = 0;i < num_out;++i){
                auto op = DataFlow::df->getOutOp(i);
                auto data_ptr = op->getData();
                auto size = data_per_thread;
                if(m==num_thr-1)
                    size += data_per_thread_rest;
                cgraHw->setCgraProgramOutputStreamByID(DataFlow::df->getId(),op->getId(), &data_ptr[m*data_per_thread], size);
            }
        }
        cgraHw->syncExecute();
    } else {
        printf("Scheduler Error: %s\n",scheduler->getMessageError(r).c_str());
    }

    delete cgraArch;
    delete cgraHw;
    delete scheduler;

    return r;
}
