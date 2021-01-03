#pragma once

#include <unordered_map>
#include <unordered_set>
#include "PassManager.hpp"
#include "Module.h"
#include "Function.h"
#include "LoopSearch.hpp"
#include "BasicBlock.h"

class LoopInvHoist : public Pass {
    std::set<
        std::pair<BasicBlock *, Instruction *> >
            invariant;
    void findInvariants(BBset_t *loop);
    void moveInvariantsOut(BBset_t *loop);
    LoopSearch  *loop_searcher;

public:
    LoopInvHoist(Module *m) : Pass(m) {}
    void run() override;
};
