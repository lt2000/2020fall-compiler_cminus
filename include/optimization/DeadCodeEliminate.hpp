#ifndef SYSYC_DEADCODEELIMATE_H
#define SYSYC_DEADCODEELIMATE_H
#include "logging.hpp"
#include "PassManager.hpp"
#include <unordered_map>
#include <unordered_set>

class DeadCodeEliminate : public Pass
{
public:
  DeadCodeEliminate(Module *m) : Pass(m) {}
  void run() override;

  bool isEqualIntr(Instruction *instr1, Instruction *instr2);
  bool isEqualFirstPtr(Value *ptr1, Value *ptr2);
  bool isDeadInstruction(Instruction *inst);
  bool isLocalStore(StoreInst *store);
  bool isArgArrayStore(StoreInst *store);
  bool isGlobalArgArrayPtr(Value *val);
  void dealStore(StoreInst *store, std::unordered_set<StoreInst *> &pre_stores,
                 std::unordered_set<LoadInst *> &pre_loads,
                 std::vector<Instruction *> &wait_remove);
  bool isSideEffect(Instruction *instr)
  {
    return (instr->is_call() || instr->is_void());
  }
  bool isSideEffectFunc(Function *func)
  {
    return SideEffectFunc.find(func) != SideEffectFunc.end();
  }
  bool isEqualPtr(Value *ptr1, Value *ptr2)
  {
    return ptr1 == ptr2;
  }
  bool isEqualStorePtr(StoreInst *store_a, StoreInst *store_b)
  {
    return isEqualPtr(store_a->get_lval(), store_b->get_lval());
  }

  bool isEqualLoadPtr(LoadInst *load_a, LoadInst *load_b)
  {
    return isEqualPtr(load_a->get_lval(), load_b->get_lval());
  }

  bool isEqualStoreFirstPtr(StoreInst *store_a, StoreInst *store_b)
  {
    return isEqualFirstPtr(store_a->get_lval(), store_b->get_lval());
  }

  bool isEqualStoreLoadPtr(StoreInst *store, LoadInst *load)
  {
    return isEqualPtr(store->get_lval(), load->get_lval());
  }

  bool isEqualStoreLoadFirstPtr(StoreInst *store, LoadInst *load)
  {
    return isEqualFirstPtr(store->get_lval(), load->get_lval());
  }
  bool is_main(Function *func)
  {
    return func->get_name() == "main";
  }

  void InitSideEffectFunc(Module *m);

  void deleteDeadFunc(Module *m);
  void deleteDeadInst(Function *func);
  void deleteDeadRet(Function *func);
  void deleteDeadStore(Function *func);

  void markUse(Instruction *inst, std::unordered_set<Instruction *> &worklist);

private:
  std::unordered_set<Function *> SideEffectFunc;
};
#endif // SYSYC_DEADCODEELIMATE_H