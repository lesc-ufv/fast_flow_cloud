#ifndef OPERATOR_H
#define OPERATOR_H

#include <vector>
#include <string>

#include <xready/cgra_arch_defs.h>

class Operator {

private:
    int id;
    int c_id;
    int in_id;
    int out_id;
    int level;
    int opCode;
    int type;
    short val;
    short constant;
    Operator *srcA;
    Operator *srcB;
    Operator *branchIn;
    std::vector<Operator *> dst;
    int dataFlowId;
    std::string label;
    bool isEnd;

public:
    Operator(int id, int op_code, int type, std::string label);

    Operator(int id, int op_code, int type, std::string label, short constant);

    virtual ~Operator();
    
    int getId() const;

    void setId(int id);

    int getOpCode() const;

    void setOpCode(int op_code);

    int getType() const;

    void setType(int type);

    short getVal() const;

    void setVal(int val);

    Operator *getSrcA() const;

    void setSrcA(Operator *srcA);

    Operator *getSrcB() const;

    void setSrcB(Operator *srcB);

    Operator *getBranchIn() const;

    void setBranchIn(Operator *branchIn);

    std::vector<Operator *> &getDst();

    short getConst() const;

    void setConst(short constant);

    void setLevel(int level);

    int getLevel() const;

    void setDataFlowId(int dataFlowId);

    int getDataFlowId() const;

    static int getPortA();

    static int getPortB();

    static int getPortBranch();

    const std::string &getLabel() const;

    virtual void compute() = 0;

    int getIsEnd() const;

    void setIsEnd(bool isEnd);

    void setCId(int cid);

    void setInId(int inid);

    void setOutId(int outid);

    int getCId() const;

    int getInId() const;

    int getOutId() const;

};

#endif //OPERATOR_H
