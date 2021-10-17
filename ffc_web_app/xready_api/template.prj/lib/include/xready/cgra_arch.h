#ifndef CGRA_ARCH_H
#define CGRA_ARCH_H

#include <map>
#include <fstream>

#include <xready/data_flow.h>
#include <xready/operator.h>
#include <xready/data_flow_defs.h>
#include <xready/cgra_defs.h>
#include <xready/pe_arch.h>
#include <xready/omega.h>

#include <json/json.h>

class CgraArch {

private:
    int id;

    int num_pe;

    int num_pe_in;

    int num_pe_out;

    int num_extra_stage;

    int net_radix;

    int word_size;
public:
    std::vector<int> pe_list_in;
    
    std::vector<int> pe_list_out;
    
    std::vector<int> pe_list_basic;

    std::map<int, int> pe_in_map;
    
    std::map<int, int> pe_out_map;

private:

    std::map<int, PEArch *> pe_array;

    std::vector<Omega *> netThreads;

    std::vector<Omega *> net_branchThreads;

    std::map<int, DataFlow *> dataFlows;

    cgra_program_t cgra_program;

    void makeProgram();

    std::map<int, int> makeListPe(int num_pe, int num_pe_in, int num_pe_out);

    void initialize();

public:
    CgraArch(int id, int num_pe, int num_pe_in, int num_pe_out, int net_radix, int num_extra_stage, int word_size);

    explicit CgraArch(arch_t arch);
    
    explicit CgraArch(std::string json_file);

    ~CgraArch();

    int getId() const;

    void setId(int id);

    int getNumPe() const;

    void setNumPe(int num_pe);

    int getNumPeIn() const;

    void setNumPeIn(int num_pe_in);

    int getNumPeOut() const;

    void setNumPeOut(int num_pe_out);

    int getNetRadix() const;

    void setNetRadix(int net_radix);

    Omega *getNet(int threadID) const;

    Omega *getNetBranch(int threadID) const;

    void setDataFlow(DataFlow *df, int threadID);

    DataFlow *getDataFlow(int threadID);

    PEArch *getPE(int id);

    void reset();

    void reset(int threadID);

    void writeCgraProgram(const std::string &filePath);

    int getNumThreads();

    const std::map<int, PEArch *> &getPeArray() const;

    cgra_program_t getCgraProgram();

    int getWordSize() const;

};

#endif //CGRA_ARCH_H
