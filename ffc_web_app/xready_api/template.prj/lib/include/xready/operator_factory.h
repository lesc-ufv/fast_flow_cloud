#ifndef CGRASCHEDULER_OPERATOR_H
#define CGRASCHEDULER_OPERATOR_H

#include <string>
#include <unordered_map>

#include <xready/operator.h>
#include <xready/params.h>
#include <xready/abs.h>
#include <xready/add.h>
#include <xready/and.h>
#include <xready/beq.h>
#include <xready/bne.h>
#include <xready/input_stream.h>
#include <xready/max.h>
#include <xready/min.h>
#include <xready/mult.h>
#include <xready/mux.h>
#include <xready/not.h>
#include <xready/or.h>
#include <xready/output_stream.h>
#include <xready/pass_a.h>
#include <xready/pass_b.h>
#include <xready/sgt.h>
#include <xready/shl.h>
#include <xready/shr.h>
#include <xready/slt.h>
#include <xready/sub.h>
#include <xready/xor.h>

typedef Operator *(*pfnCreate_t)(Params); // function pointer type

class OperatorFactory {
private:
    OperatorFactory();

    OperatorFactory(const OperatorFactory &) {}

    OperatorFactory &operator=(const OperatorFactory &) { return *this; }

    typedef std::unordered_map<std::string, pfnCreate_t> FactoryMap;
    FactoryMap m_FactoryMap;

public:
    ~OperatorFactory() { m_FactoryMap.clear(); }

    static OperatorFactory *Get() {
        static OperatorFactory instance;
        return &instance;
    }

    void Register(const std::string &operatorName, pfnCreate_t pfnCreate);

    Operator *CreateOperator(const std::string &operatorName, Params &params);
};

#endif //CGRASCHEDULER_OPERATOR_H
