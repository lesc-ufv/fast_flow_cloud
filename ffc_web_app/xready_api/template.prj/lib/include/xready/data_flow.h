#ifndef DATAFLOW_H
#define DATAFLOW_H

#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <string>

#include <json/json.h>

#include <xready/data_flow_defs.h>
#include <xready/cgra_arch_defs.h>
#include <xready/operator.h>
#include <xready/operator_factory.h>
#include <xready/params.h>
#include <xready/input_stream.h>
#include <xready/output_stream.h>
#include <xready/ffc_args.h>

class DataFlow {

private:
    int id;
    std::string name;
    std::map<int, Operator *> op_array;
    std::map<int, std::vector<int>> graph;
    std::vector<int> input_op_ids;
    std::vector<int> output_op_ids;    
    int num_op_in;
    int num_op_out;
    int num_op;
    int num_level;

    void addOperator(Operator *op);

public:

    DataFlow(int id, std::string name);

    DataFlow(std::string json_name);

    ~DataFlow();

    Operator *removeOperator(int op_id);

    void compute();

    const std::map<int, Operator *> &getOpArray() const;

    Operator *getOp(int id);

    void toDOT(std::string fileNamePath);

    void toJSON(const std::string &fileNamePath);

    void fromJSON(const std::string &fileNamePath);

    void connect(Operator *src, Operator *dst, int dstPort);

    void updateOpLevel();

    int getId() const;

    void setId(int id);

    const std::string &getName() const;

    const std::map<int, std::vector<int>> &getGraph() const;

    int getNumOpIn() const;

    int getNumOpOut() const;

    int getNumOp() const;

    int getNumEdges() const;

    int getNumLevel() const;
    
    std::vector<int> &getInputIds();
    
    std::vector<int> &getOutputIds();

    Operator *getCOp(int c_id);

    InputStream *getInOp(int in_id);

    OutputStream *getOutOp(int out_id);

    void setInputData(int id, short * data, long size);

    void setOutputData(int id, short * data, long size);

    void setConstants(short * constants, int size);

    void run();

};


#endif //DATAFLOW_H
