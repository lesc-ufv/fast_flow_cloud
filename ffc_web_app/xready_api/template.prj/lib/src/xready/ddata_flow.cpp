#include <xready/ddata_flow.h>

DDataFlow::DDataFlow(int id, std::string name) :
        id(id),
        name(std::move(name)),
        num_op_in(0),
        num_op_out(0),
        num_op(0),
        num_level(0) {}

DDataFlow::DDataFlow(const DDataFlow &df){
    DDataFlow::id = -1;
    DDataFlow::name = df.name;
    DDataFlow::op_array = df.op_array;
    DDataFlow::graph = df.graph;
    DDataFlow::input_op_ids = df.input_op_ids;
    DDataFlow::output_op_ids = df.output_op_ids;
    DDataFlow::num_op_in = df.num_op_in;
    DDataFlow::num_op_out = df.num_op_out;
    DDataFlow::num_op = df.num_op;
    DDataFlow::num_level = df.num_level;
}

DDataFlow::~DDataFlow() { }

void DDataFlow::addOperator(Operator *op) {

    if (op->getDataFlowId() == -1) {
        op->setDataFlowId(DDataFlow::id);
        DDataFlow::op_array[op->getId()] = op;
        DDataFlow::num_op++;
        if (op->getType() == OP_IN) {
            DDataFlow::num_op_in++;
            DDataFlow::input_op_ids.push_back(op->getId());
        }
        if (op->getType() == OP_OUT) {
            DDataFlow::num_op_out++;
            DDataFlow::output_op_ids.push_back(op->getId());
        }
    }
}

Operator *DDataFlow::removeOperator(int op_id) {
    Operator *r = DDataFlow::op_array[op_id];
    DDataFlow::op_array.erase(op_id);
    r->setDataFlowId(-1);
    return r;
}

void DDataFlow::compute() {
    auto n = DDataFlow::getNumLevel();
    int allIsEnd = 0;
    int num_in = DDataFlow::getNumInputs();
    while (allIsEnd != num_in) {
        allIsEnd = 0;
        for (int i = 0; i <= n; ++i) {
            for (auto item:DDataFlow::getOpArray()) {
                auto op = item.second;
                if (op->getLevel() == i) {
                    op->compute();
                    if (op->getType() == OP_IN && op->getIsEnd()) {
                        allIsEnd++;
                    }
                }
            }
            if (allIsEnd == num_in) {
                break;
            }
        }
    }
}

const std::map<int, Operator *> &DDataFlow::getOpArray() const {
    return op_array;
}

Operator *DDataFlow::getOp(int id) {
    if (DDataFlow::op_array.find(id) != DDataFlow::op_array.end()) {
        return DDataFlow::op_array[id];
    }
    return nullptr;
}

void DDataFlow::toDOT(std::string fileNamePath) {
    std::ofstream myfile;
    myfile.open(fileNamePath);
    myfile << "digraph " << DDataFlow::name << "{" << std::endl;
    for (auto op:DDataFlow::op_array) {

        if (op.second->getType() == OP_IN) {
            myfile << " " << op.first << " [ label = input" << op.second->getId() << " ]" << std::endl;
        } else if (op.second->getType() == OP_OUT) {
            myfile << " " << op.first << " [ label = output" << op.second->getId() << " ]" << std::endl;
        } else if (op.second->getType() == OP_IMMEDIATE) {
            myfile << " " << op.first;
            myfile << " [ label = " << op_label[op.second->getOpCode()] << "i";
            myfile << ", VALUE = " << op.second->getConst();
            myfile << "]" << std::endl;
            myfile << " \"" << op.first << "." << op.second->getConst() << "\"[ label = " << op.second->getConst()
                   << " ]" << std::endl;

        } else {
            myfile << " " << op.first << " [ label = " << op_label[op.second->getOpCode()] << "]" << std::endl;
        }
    }
    for (auto op:DDataFlow::op_array) {
        if (op.second->getType() == OP_IMMEDIATE) {
            myfile << " \"" << op.first << "." << op.second->getConst() << "\" -> " << op.first << std::endl;
        }
        for (auto op_dst:op.second->getDst()) {
            myfile << " " << op.first << " -> " << op_dst->getId() << std::endl;
        }
    }
    myfile << "}" << std::endl;
    myfile.close();
}

void DDataFlow::toJSON(const std::string &fileNamePath) {
    std::ofstream myfile;
    myfile.open(fileNamePath);
    myfile << "[" << std::endl;

    char str_node[] = R"({"data":{"id":%d,"type":"%s","value":%d},"group":"nodes"})";
    char str_edge[] = R"({"data":{"id":%d,"source":%d,"target":%d, "port":%d},"group":"edges"})";

    char buf[256];
    int numEdge = DDataFlow::getNumEdges();
    int cnt = 0;
    int max_id = 0;
    int id_edges = 0;
    for (auto item:DDataFlow::op_array) {
        cnt++;
        auto op = item.second;
        sprintf(buf, str_node, op->getId(), op->getLabel().c_str(), op->getConst());
        if (op->getId() > max_id) {
            max_id = op->getId();
        }
        myfile << buf << "," << std::endl;
    }
    id_edges = max_id + 1;
    cnt = 0;
    int port;         //src,dst,port
    std::map<std::tuple<int,int,int>,bool> map_port;

    for (auto item:DDataFlow::op_array) {
        auto op = item.second;
        for (auto neighbor:op->getDst()) {
            cnt++;
            port = -1;
            if(neighbor->getSrcA()) {
                if (neighbor->getSrcA()->getId() == op->getId()) {
                    auto key = std::tuple<int,int,int>(op->getId(),neighbor->getId(),0);
                    if(map_port.find(key) == map_port.end()){
                        port = 0;
                        map_port[key] = true;
                    }
                }
            }
            if(neighbor->getSrcB() && port == -1) {
                if (neighbor->getSrcB()->getId() == op->getId()) {
                    auto key = std::tuple<int,int,int>(op->getId(),neighbor->getId(),1);
                    if(map_port.find(key) == map_port.end()){
                        port = 1;
                        map_port[key] = true;
                    }
                }
            }
            if(neighbor->getBranchIn()) {
                if (neighbor->getBranchIn()->getId() == op->getId()) {
                    port = 2;
                }
            }
            sprintf(buf, str_edge, id_edges++, op->getId(), neighbor->getId(), port);
            if (cnt < numEdge)
                myfile << buf << "," << std::endl;
            else
                myfile << buf << std::endl;
        }
    }
    myfile << "]";
    myfile.close();
}

void DDataFlow::fromJSON(const std::string &fileNamePath) {
    Json::Value df;
    Json::Value map_op;

    const auto str_map_op_length = static_cast<int>(str_map_op.length());

    std::ifstream ifs;
    ifs.open(fileNamePath);

    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, ifs, &df, &errs)) {
        std::cout << errs << std::endl;
        return;
    }
    ifs.close();
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(str_map_op.c_str(), str_map_op.c_str() + str_map_op_length, &map_op, &errs)) {
        std::cout << errs << std::endl;
        return;
    }

    for (auto e:df["nodes"]) {
        int id = e["id"].asInt();
        std::string opcode = e["opcode"].asString();
        auto params = Params(id, 0, nullptr, 0);
        auto op = OperatorFactory::Get()->CreateOperator(opcode, params);
        if(opcode == "input"){
            op->setInId(e["in_id"].asInt());
        }else if(opcode == "output"){
            op->setOutId(e["out_id"].asInt());
        }else{
            op->setCId(e["const_id"].asInt());
        }
        DDataFlow::addOperator(op);
    }

    for (auto e:df["edges"]) {
        auto src = e["source"].asInt();
        auto dst = e["target"].asInt();
        auto port = e["port"].asInt();
        auto op_src = DDataFlow::op_array[src];
        auto op_dst = DDataFlow::op_array[dst];
        DDataFlow::connect(op_src, op_dst, port);
    }
}

void DDataFlow::connect(Operator *src, Operator *dst, int dstPort) {

    DDataFlow::addOperator(src);
    DDataFlow::addOperator(dst);
    DDataFlow::graph[src->getId()].push_back(dst->getId());

    src->getDst().push_back(dst);

    if (dstPort == PORT_A) {
        dst->setSrcA(src);
    } else if (dstPort == PORT_B) {
        dst->setSrcB(src);
    } else if (dstPort == PORT_BRANCH) {
        dst->setBranchIn(src);
    }
    DDataFlow::updateOpLevel();
}

void DDataFlow::updateOpLevel() {
    std::queue<int> q;
    int parent;
    for (auto op:DDataFlow::op_array) {
        if (op.second->getType() == OP_IN) {
            q.push(op.first);
            while (!q.empty()) {
                parent = q.front();
                q.pop();
                for (auto child:DDataFlow::graph[parent]) {
                    int lp = DDataFlow::op_array[parent]->getLevel();
                    int lc = DDataFlow::op_array[child]->getLevel();
                    if (lp >= lc) {
                        DDataFlow::op_array[child]->setLevel(lp + 1);
                    }
                    q.push(child);
                }
            }
        }
    }
    for (auto op:DDataFlow::op_array) {
        if (op.second->getType() == OP_IN) {
            int level = 0;
            for (auto child:DDataFlow::graph[op.first]) {
                if (DDataFlow::op_array[child]->getLevel() > level) {
                    level = DDataFlow::op_array[child]->getLevel();
                }
            }
            if (level > 0)
                level = level - 1;
            op.second->setLevel(level);
        }
    }
    for (auto op:DDataFlow::op_array) {
        if (op.second->getLevel() > DDataFlow::num_level) {
            DDataFlow::num_level = op.second->getLevel();
        }
    }
}


int DDataFlow::getId() const {
    return id;
}

const std::string &DDataFlow::getName() const {
    return name;
}

const std::map<int, std::vector<int>> &DDataFlow::getGraph() const {
    return graph;
}

int DDataFlow::getNumInputs() const {
    return num_op_in;
}

int DDataFlow::getNumOutputs() const {
    return num_op_out;
}

int DDataFlow::getNumNodes() const {
    return num_op;
}

void DDataFlow::setId(int id) {
    DDataFlow::id = id;
}

int DDataFlow::getNumEdges() const {
    int num_edges = 0;
    for (const auto &v:DDataFlow::graph) {
        num_edges += v.second.size();
    }
    return num_edges;
}

int DDataFlow::getNumLevel() const {
    return DDataFlow::num_level;
}
std::vector<int> &DDataFlow::getInputIds(){
    return DDataFlow::input_op_ids;
}
    
std::vector<int> &DDataFlow::getOutputIds(){
    return DDataFlow::output_op_ids;
}

Operator *DDataFlow::getCOp(int c_id){
    for (auto op:DDataFlow::op_array) {
        if (op.second->getCId() == c_id) {
            return op.second;
        }
    }
    return nullptr;
}

InputStream *DDataFlow::getInOp(int in_id){
    for (auto op:DDataFlow::op_array) {
            if (op.second->getInId() == in_id) {
            return reinterpret_cast<InputStream*>(op.second);
        }
    }
    return nullptr;
}

OutputStream *DDataFlow::getOutOp(int out_id){
    for (auto op:DDataFlow::op_array) {
        if (op.second->getOutId() == out_id) {
            return reinterpret_cast<OutputStream*>(op.second);
        }
    }
    return nullptr;
}
