#include "multadd_test.h"

int main(int argc, char *argv[]) {

    std::string fpgaBinaryFile;
    std::string kernel_name;
    int idx = 0;
    int test = 0;
    
    if(argc > 2){
        fpgaBinaryFile = argv[1];
        kernel_name =  argv[2];
    }else {
        std::cout << "invalid args!!!\n";
        std::cout << "usage: "<< argv[0] << " <xclbin> <kernel_name>\n";
        exit(255); 
    }

    if (argc > 3)
        test = atoi(argv[3]);

    if (argc > 4)
        idx = atoi(argv[4]);
    
    if (test & 1)
        multadd(idx);

    if (test & 2)
        multadd_openmp(idx);

    if (test & 4)
        multadd_cgra(fpgaBinaryFile,kernel_name,idx, 1);

    if (test & 8)
        multadd_dataflow_cpu();

    return 0;
}

int multadd(int idx) {

    unsigned short *data_in0, *data_out;

    data_in0 = new unsigned  short[DATA_SIZE];
    data_out = new unsigned short[DATA_SIZE];

    for (int k = 0; k < DATA_SIZE; ++k) {
        data_in0[k] = k+1;
        data_out[k] = 0;
    }

    high_resolution_clock::time_point s;
    duration<double> diff = {};

    for (int i = 0; i < SAMPLES; i++) {
        s = high_resolution_clock::now();
        for (int k = 0; k < DATA_SIZE; ++k) {
            int a = data_in0[k] * data_in0[k] + 3;
            for(int p = 0; p < 16; p++){
                a = a * a + 3;
            }
            data_out[k] = a;
        }
        diff += high_resolution_clock::now() - s;
    }

    double cpuExecTime = (diff.count() * 1000) / SAMPLES;

    printf("Time(ms) CPU 1 Thread: %5.2lf\n", cpuExecTime);
    
    for(int i = 0; i < 10; i++){
        printf("%d ",data_out[i]);
    }
    printf("\n");
    int v = data_out[idx];

    delete data_in0;
    delete data_out;

    return v;

}

int multadd_openmp(int idx) {

    unsigned short *data_in0, *data_out;

    data_in0 = new unsigned short[DATA_SIZE];
    data_out = new unsigned short[DATA_SIZE];

    for (int k = 0; k < DATA_SIZE; ++k) {
        data_in0[k] = k+1;
        data_out[k] = 0;
    }
    omp_set_num_threads(NUM_THREAD);
    high_resolution_clock::time_point s;
    duration<double> diff = {};
    for (int i = 0; i < SAMPLES; i++) {
        s = high_resolution_clock::now();
        #pragma omp parallel
        #pragma omp for
        for (int k = 0; k < DATA_SIZE; ++k) {
            int a = data_in0[k] * data_in0[k] + 3;
            for(int p = 0; p < 16; p++){
                a = a * a + 3;
            }
            data_out[k] = a;
        }
        diff += high_resolution_clock::now() - s;
    }
    double cpuExecTime = (diff.count() * 1000) / SAMPLES;

    printf("Time(ms) CPU %d Thread: %5.2lf\n", NUM_THREAD, cpuExecTime);

    for(int i = 0; i < 10; i++){
        printf("%d ",data_out[i]);
    }
    printf("\n");
    int v = data_out[idx];

    delete data_in0;
    delete data_out;

    return v;
}

int multadd_cgra(std::string fpgaBinaryFile,std::string kernel_name, int idx, int copies) {

    auto cgraArch = new CgraArch("cgra_arch.json");
    auto cgraHw = new Cgra(fpgaBinaryFile,kernel_name);
    Scheduler scheduler(cgraArch);
    
    std::vector<DataFlow *> dfs;
    unsigned short *data_in0, *data_out;
    data_in0 = new unsigned short[DATA_SIZE];
    data_out = new unsigned short[DATA_SIZE];
    
    int r = 0, v = 0, tries = 0;

    for (int k = 0; k < DATA_SIZE; ++k) {
        data_in0[k] = k+1;
        data_out[k] = 0;
    }

    for (int i = 0; i < NUM_THREAD; ++i) {
        dfs.push_back(createDataFlow(i, copies));
        scheduler.addDataFlow(dfs[i], i, 0);
    }
    do {
        r = scheduler.scheduling();
        tries++;
    } while (r != SCHEDULE_SUCCESS && tries < 100);

    if (r == SCHEDULE_SUCCESS) {

         cgraHw->loadCgraProgram(cgraArch->getCgraProgram());
 
         auto data_size = (size_t) (DATA_SIZE / ((NUM_THREAD) * copies));
         auto data_size_bytes = sizeof(unsigned short) * data_size;
 
         int k = 0;
         for (int i = 0; i < NUM_THREAD; ++i) {
             for (int j = 0, c = 0; j < copies; ++j) {
                 cgraHw->setCgraProgramInputStreamByID(i, c, &data_in0[k * data_size], data_size_bytes);
                 cgraHw->setCgraProgramOutputStreamByID(i, c + 1, &data_out[k * data_size], data_size_bytes);
                 c = c + 2;
                 k++;
             }
         }
         double cgraExecTime = 0;
         for (int i = 0; i < SAMPLES; i++){
             cgraHw->syncExecute();
             cgraExecTime += cgraHw->getTimeExec();
         }
         cgraExecTime /= SAMPLES;
         printf("Time(ms) CGRA: %5.2lf\n", cgraExecTime);
         v = data_out[idx];
 
         for(int i = 0; i < 10; i++){
             printf("%d ",data_out[i]);
         }
         printf("\n");

    } else {
        printf("Scheduler Error: %d\n", r);
    }

    delete cgraArch;
    delete cgraHw;
    delete data_in0;
    delete data_out;
    for (auto df:dfs) {
        delete df;
    }
    return v;
}

DataFlow *createDataFlow(int id, int copies) {

    auto df = new DataFlow(id, "multadd");
    int idx = 0;
    Operator *inA[copies];
    Operator *out[copies];
    Operator *last=nullptr;
    for (int i = 0; i < copies; ++i) {
        inA[i] = new InputStream(idx++,nullptr,0);
        out[i] = new OutputStream(idx++,nullptr,0);
    }
    
    for (int i = 0; i < copies; ++i) {
        for(int j = 0; j < 14; j++){
           auto reg = new MultAdd(idx++,3);
           if(j == 0){
               df->connect(inA[i], reg, reg->getPortA());
               df->connect(inA[i], reg, reg->getPortB());
           }
           else{
               df->connect(last, reg, reg->getPortA());
               df->connect(last, reg, reg->getPortB());
	       if(j == 13){
                  df->connect(reg, out[i], out[i]->getPortA());
               }
           }
           last = reg;
        }
    }
    
    df->toDOT("multadd.dot");

    return df;
}

void multadd_dataflow_cpu() {

    short *data_in0, *data_out;

    data_in0 = new short[DATA_SIZE];
    data_out = new short[DATA_SIZE];

    for (int k = 0; k < DATA_SIZE; ++k) {
        data_in0[k] = k+1;
        data_out[k] = 0;
    }

    auto dataFlow = createDataFlow(0, 1);

    auto in0 = reinterpret_cast<InputStream *>(dataFlow->getOp(0));
    auto out = reinterpret_cast<OutputStream *>(dataFlow->getOp(1));
    
    in0->setData(data_in0,DATA_SIZE);
    out->setData(data_out,DATA_SIZE);

    dataFlow->compute();
    dataFlow->toJSON("multadd.json");
    dataFlow->toDOT("multadd.dot");

    for(int i = 0; i < 10; i++){
        printf("%d ",data_out[i]);
    }
    printf("\n");
    
    delete dataFlow;
}
