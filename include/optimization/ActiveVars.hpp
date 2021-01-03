#ifndef ACTIVEVARS_HPP
#define ACTIVEVARS_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <map>
#include <queue>
#include <fstream>

#include <unordered_set>

class ActiveVars : public Pass
{
public:
    ActiveVars(Module *m) : Pass(m) {}

    void initset(Function *func);

    void run();
    std::string print();
    //print use_set and def_set for cur func
    std::string use_def_print();
private:
    Function *func_;
    std::map<BasicBlock *, std::unordered_set<Value *>> live_in, live_out;

    std::map<BasicBlock *, std::unordered_set<Value *>> use_set, def_set, phi_op;
    //cur BB's phi func's parameter , it is a pair for BasicBlock and var
    std::map<BasicBlock *, std::set<std::pair<BasicBlock *, Value *>>> phi_op_pair;
};

#endif
