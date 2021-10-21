#ifndef PASSA_H
#define PASSA_H

#include <xready/operator.h>
#include <xready/data_flow_defs.h>
#include <xready/params.h>

class PassA : public Operator {
public:
    explicit PassA(int id) : Operator(id, OP_PASS_A, OP_BASIC, "pass_a") {}

    static Operator *create(Params params) {
        return new PassA(params.id);
    }

    void compute() override {
        if (Operator::getSrcA()) {
            auto v = Operator::getSrcA()->getVal();
            Operator::setVal(v);
        }
    }

};

#endif //PASSA_H
