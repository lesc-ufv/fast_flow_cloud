#ifndef MIN_H
#define MIN_H

#include <xready/data_flow_defs.h>
#include <xready/operator.h>
#include <xready/params.h>

class Min : public Operator {
public:
    explicit Min(int id) : Operator(id, OP_MIN, OP_BASIC, "min") {}

    static Operator *create(Params params) {
        return new Min(params.id);
    }

    void compute() override {
        if (Operator::getSrcA() && Operator::getSrcB()) {
            if (Operator::getSrcA()->getVal() < Operator::getSrcB()->getVal()) {
                auto v = Operator::getSrcA()->getVal();
                Operator::setVal(v);
            } else {
                auto v = Operator::getSrcB()->getVal();
                Operator::setVal(v);
            }
        }
    }
};

class Mini : public Operator {
public:
    Mini(int id, int constant) : Operator(id, OP_MIN, OP_IMMEDIATE, "mini", constant) {}

    static Operator *create(Params params) {
        return new Mini(params.id, params.constant);
    }

    void compute() override {
        if (Operator::getSrcA()) {
            if (Operator::getSrcA()->getVal() < Operator::getConst()) {
                auto v = Operator::getSrcA()->getVal();
                Operator::setVal(v);
            } else {
                auto v = Operator::getConst();
                Operator::setVal(v);
            }
        }
    }
};


#endif //MIN_H
