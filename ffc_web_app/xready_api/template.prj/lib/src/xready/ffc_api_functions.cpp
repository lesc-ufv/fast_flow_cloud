#include<xready/ffc_api_functions.h>

int ffc_dataflow_exec(
    int argc,
    char *argv[],
    std::string dataflow,
    void ** inputs,
    long *inputs_size,
    void ** outputs,
    long *outpus_size,
    void *constants,
    int number_of_const
){

    std::string json_path = dataflow+".json";
    auto df = new DataFlow(0,dataflow);
    df->fromJSON(json_path);
    run_dataflow_cpu(df,inputs,inputs_size,outputs,outpus_size,constants,number_of_const);
    return 0;
}

void run_dataflow_cpu(
    DataFlow *df,
    void ** inputs,
    long *inputs_size,
    void ** outputs,
    long *outpus_size,
    void *constants,
    int number_of_const){

    auto in = reinterpret_cast<short **>(inputs);
    auto out = reinterpret_cast<short **>(outputs);
    auto cons = reinterpret_cast<short *>(constants);

    int num_inputs = df->getNumOpIn();
    for(int i = 0; i < num_inputs;i++){
        auto op_in = reinterpret_cast<InputStream *>(df->getInOp(i));
        op_in->setData(in[i],inputs_size[i]);
    }
    int num_outputs = df->getNumOpOut();
    for(int i = 0; i < num_outputs;i++){
        auto op_out = reinterpret_cast<OutputStream *>(df->getOutOp(i));
        op_out->setData(out[i],outpus_size[i]);
    }
    for(int i = 0; i < number_of_const;i++){
        auto op_const = df->getCOp(i);
        op_const->setConst(cons[i]);
    }

    df->compute();

}
