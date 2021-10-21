#ifndef NOT_H
#define NOT_H

#include <xready/operator.h>
#include <xready/data_flow_defs.h>
#include <xready/params.h>

class Not : public Operator {
public:
    explicit Not(int id) : Operator(id, OP_NOT, OP_BASIC, "not") {}

    static Operator *create(Params params) {
        return new Not(params.id);
    }

    void compute() override {
        if (Operator::getSrcA() && Operator::getSrcB()) {
            auto v = ~Operator::getSrcA()->getVal();
            Operator::setVal(v);
        }
    }

};


#endif //NOT_H
