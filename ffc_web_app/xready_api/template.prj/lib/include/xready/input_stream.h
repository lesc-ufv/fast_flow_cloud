#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <iostream>

#include <xready/operator.h>
#include <xready/data_flow_defs.h>
#include <xready/cgra_arch_defs.h>
#include <xready/params.h>

class InputStream : public Operator {
private:
    int index;
    short *data;
    long size;

public:
    explicit InputStream(int id, short *data, long size) : Operator(id, OP_PASS_A, OP_IN, "input"),
                                                        index(0), data(data),
                                                        size(size) {}

    static Operator *create(Params params) {
        return new InputStream(params.id, params.data, params.size);
    }

    void setData(short *data, long size) {
        InputStream::data = data;
        InputStream::size = size;
    }

    ~InputStream(){
        delete []InputStream::data;
    }

    short * getData(){
        return InputStream::data;
    }
    
    int getSize(){
        return InputStream::size;
    }

    void reset() {
        InputStream::index = 0;
        Operator::setIsEnd(false);
    }

    void compute() override {

        if (data) {
            if (InputStream::index < InputStream::size) {
                auto v = InputStream::data[InputStream::index++];
                Operator::setVal(v);
            } else {
                Operator::setIsEnd(true);
            }
        } else {
            Operator::setIsEnd(true);
        }
    }
};

#endif //INPUTSTREAM_H
