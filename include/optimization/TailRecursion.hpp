#ifndef TAILRECURSION_HPP
#define TAILRECURSION_HPP
#include "logging.hpp"
#include "PassManager.hpp"
#include <map>
#include <set>

class TailRecursion : public Pass
{
public:
  TailRecursion(Module *m) : Pass(m) {}
  void run();
  void BackFindTailRecursion(Function *func);
  void VoidFindTailRecursion(Function *func);
  //find all Direct Tail Recursions, and return bool for whether it can have Indirect Tail Recursion
  bool DirectTailRecursion(Function *func);
  void AddBBandReplace(Function *func);
  void AddPhis(Function *func);
  void dealTailRecursion(Function *func);
  bool is_main(Function *func);
private:
  std::map<CallInst *, BasicBlock *> TailCalls;
  std::set<BasicBlock *>const_retBB;
  BasicBlock * new_BB;
  std::map<int, PhiInst *> Phis;
};
#endif