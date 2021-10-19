#include <xready/scheduler.h>

Scheduler::Scheduler(CgraArch *cgra) : cgraArch(cgra) {
    int num_thread = Scheduler::cgraArch->getNumThreads();
    for (int i = 0; i < num_thread; i++) {
        Scheduler::cgraArch->reset(i);
    }
}

Scheduler::~Scheduler() { }

bool Scheduler::addDataFlow(DDataFlow *df, int threadID, int groupID) {
    if (Scheduler::cgraArch) {
        if (threadID >= 0 && threadID <= (Scheduler::cgraArch->getNumThreads() - 1)) {
            Scheduler::dataflows[threadID] = df;
            Scheduler::dataflow_group[df->getId()] = groupID;
            return true;
        }
    }
    return false;
}

void Scheduler::setCgra(CgraArch *cgra) {
    Scheduler::cgraArch = cgra;
}

CgraArch *Scheduler::getCgra() {
    return Scheduler::cgraArch;
}

int Scheduler::scheduling() {

    for (const auto &df : Scheduler::dataflows) {
        int r = mapAndRoute(df.first);
        if (r != SCHEDULE_SUCCESS) {
            return r;
        }
    }

    return SCHEDULE_SUCCESS;
}

int Scheduler::mapAndRoute(int threadID) {

    if (dataflows[threadID]->getNumNodes() > cgraArch->getNumPe()) {
        return SCHEDULER_INSUFFICIENT_PE;
    }
    if (dataflows[threadID]->getNumInputs() > cgraArch->getNumPeIn()) {
        return SCHEDULER_INSUFFICIENT_PE_IN;
    }
    if (dataflows[threadID]->getNumOutputs() > cgraArch->getNumPeOut()) {
        return SCHEDULER_INSUFFICIENT_PE_OUT;
    }
    auto operators = dataflows[threadID]->getOpArray();
    auto pes = cgraArch->getPeArray();
    std::vector<int> solution;
    std::deque<int> pe_free;
    std::deque<int> pe_in_free;
    std::deque<int> pe_out_free;
    int pe_swap;
    int swapness[pes.size()];
    int group = Scheduler::dataflow_group[Scheduler::dataflows[threadID]->getId()];
    std::vector<int> pe_list_aux;
    random_selector<> selector{};

    if (Scheduler::data_flow_mapping.find(group) != Scheduler::data_flow_mapping.end()) {
        solution = Scheduler::data_flow_mapping[group];
    } else {
        solution.reserve(pes.size());
        for (int i = 0,sz = pes.size(); i < sz;i++) {
            solution.push_back(-1);
            swapness[i] = -1;
        }
        int idx = 0;
        for (auto op:operators) {
            if (op.second->getType() == OP_IN) {
                pe_list_aux.clear();
                for (auto p:pes) {
                    if (solution[p.first] == -1 && p.second->getType() == PE_IN) {
                        pe_list_aux.push_back(p.first);
                    }
                }
            } else if (op.second->getType() == OP_OUT) {
                pe_list_aux.clear();
                for (auto p:pes) {
                    if (solution[p.first] == -1 && p.second->getType() == PE_OUT) {
                        pe_list_aux.push_back(p.first);
                    }
                }
            } else {
                pe_list_aux.clear();
                for (auto p:pes) {
                    if (solution[p.first] == -1 && p.second->getType() == PE_BASIC) {
                        pe_list_aux.push_back(p.first);
                    }
                }
                if (pe_list_aux.empty()) {
                    for (auto p:pes) {
                        if (solution[p.first] == -1) {
                            pe_list_aux.push_back(p.first);
                        }
                    }
                }
            }
            idx = selector(pe_list_aux);
            solution[idx] = op.second->getId();
        }
        for (int k = 0, sz = solution.size(); k < sz; k++) {
            if (solution[k] == -1) {
                if (pes[k]->getType() == PE_IN) {
                    pe_in_free.push_back(k);
                } else if (pes[k]->getType() == PE_OUT) {
                    pe_out_free.push_back(k);
                } else {
                    pe_free.push_back(k);
                }
            }
        }
    }
    do {
        pe_swap = Scheduler::placeAndRoute(solution, threadID);
        if (pe_swap == -1) {
            Scheduler::data_flow_mapping[group] = solution;
            return SCHEDULE_SUCCESS;
        }
        int swap = -1;
        if (pe_swap < cgraArch->getNumPeIn()) {
            if (!pe_in_free.empty()) {
                swap = pe_in_free.back();
                pe_in_free.pop_back();
                pe_in_free.push_front(pe_swap);
            }
        } else if (pe_swap < (cgraArch->getNumPeIn() + cgraArch->getNumPeOut())) {
            if (!pe_out_free.empty()) {
                swap = pe_out_free.back();
                pe_out_free.pop_back();
                pe_out_free.push_front(pe_swap);
            }
        } else {
            if (!pe_free.empty()) {
                swap = pe_free.back();
                pe_free.pop_back();
                pe_free.push_front(pe_swap);
            }
        }
        if (swap == -1) {
            return SCHEDULE_PE_FREE_IS_GONE;
        };
        if (swapness[pe_swap] == swap) {
            return SCHEDULE_HAS_LOOPED;
        }
        std::swap(solution[pe_swap], solution[swap]);
        swapness[pe_swap] = swap;
    } while (true);

}

int Scheduler::placeAndRoute(std::vector<int> &mapping, int threadID) {

    std::map<int, int> mapping_op;
    Operator *op_src = nullptr;
    PEArch *pe_dst = nullptr;
    PEArch *pe_src = nullptr;

    int pe_src_port_a;
    int pe_src_port_b;
    int pe_dst_port_in;

    Scheduler::cgraArch->reset(threadID);
    Omega *net = Scheduler::cgraArch->getNet(threadID);
    Omega *net_branch = Scheduler::cgraArch->getNetBranch(threadID);
    Scheduler::cgraArch->setDataFlow(Scheduler::dataflows[threadID], threadID);

    for (int j = 0, sz = mapping.size(); j < sz; ++j) {
        mapping_op[mapping[j]] = j;
    }
    for (int i = 0, sz = mapping.size(); i < sz; ++i) {
        if (mapping[i] != -1) {
            op_src = Scheduler::dataflows[threadID]->getOp(mapping[i]);
            pe_src = Scheduler::cgraArch->getPE(i);
            if (pe_src && op_src) {
                if (op_src->getType() == OP_IN && pe_src->getType() != PE_IN) {
                    return i;
                } else if (op_src->getType() == OP_OUT && pe_src->getType() != PE_OUT) {
                    return i;
                } else {
                    if (op_src->getDst().empty() && op_src->getType() != OP_OUT) {
                        return i;
                    } else {
                        for (auto op_dst:op_src->getDst()) {
                            pe_dst = Scheduler::cgraArch->getPE(mapping_op[op_dst->getId()]);
                            int dst_branch_in_id = -1;
                            int dst_src_a_id = -1;
                            int dst_src_b_id = -1;
                            if (op_dst->getBranchIn()) {
                                dst_branch_in_id = op_dst->getBranchIn()->getId();
                            }
                            if (op_dst->getSrcA()) {
                                dst_src_a_id = op_dst->getSrcA()->getId();
                            }
                            if (op_dst->getSrcB()) {
                                dst_src_b_id = op_dst->getSrcB()->getId();
                            }
                            if (pe_dst) {
                                if (dst_branch_in_id == op_src->getId()) {
                                    if (!net_branch->addRoute(pe_src->getId(), pe_dst->getId())) {
                                        return i;
                                    }
                                    pe_src->setOperator(op_src, threadID);
                                    pe_dst->setOperator(op_dst, threadID);

                                } else {
                                    pe_src_port_a = pe_src->getId() * 2;
                                    pe_src_port_b = (pe_src->getId() * 2) + 1;
                                    if (dst_src_a_id == dst_src_b_id && pe_src->getOut(threadID) != -1) {
                                        pe_dst_port_in = (pe_dst->getId() * 2) + 1;
                                    } else if (dst_src_a_id == op_src->getId())
                                        pe_dst_port_in = pe_dst->getId() * 2;
                                    else if (dst_src_b_id == op_src->getId())
                                        pe_dst_port_in = (pe_dst->getId() * 2) + 1;
                                    else {
                                        return i;
                                    }
                                    if (pe_src->getOut(threadID) == PORT_A) {
                                        if (!net->addRoute(pe_src_port_a, pe_dst_port_in)) {
                                            return i;
                                        }
                                    } else if (pe_src->getOut(threadID) == PORT_B) {
                                        if (!net->addRoute(pe_src_port_b, pe_dst_port_in)) {
                                            return i;
                                        }
                                    } else {
                                        if (!net->addRoute(pe_src_port_a, pe_dst_port_in)) {
                                            if (!net->addRoute(pe_src_port_b, pe_dst_port_in)) {
                                                return i;
                                            } else {
                                                pe_src->setOut(PORT_B, threadID);
                                            }
                                        } else {
                                            pe_src->setOut(PORT_A, threadID);
                                        }
                                    }
                                    pe_src->setOperator(op_src, threadID);
                                    pe_dst->setOperator(op_dst, threadID);
                                }
                            } else {
                                return i;
                            }
                        }
                    }
                }
            } else {
                return i;
            }
        }
    }
    return -1;
}

void Scheduler::reset() {
    Scheduler::dataflows.clear();
    Scheduler::dataflow_group.clear();
    Scheduler::data_flow_mapping.clear();
}

std::vector<int> Scheduler::getMapping(int dataFlowId, int threadID) {
    int group = Scheduler::dataflow_group[Scheduler::dataflows[threadID]->getId()];
    return Scheduler::data_flow_mapping[group];
}
std::string Scheduler::getMessageError(int code){
    code *= -1;
    code -= 1;
    if(code >= 0 && code <= 5){
        return scheduler_error_message[code];
    }

    return "Error code does not exist.";
}
