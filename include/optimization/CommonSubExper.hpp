#ifndef COMMOCSUBEXPER_HPP
#define COMMOCSUBEXPER_HPP
#include "logging.hpp"
#include "PassManager.hpp"
#include <set>
#include <tuple>

ConstantFP *constantfp(Value *value);
ConstantInt *constantint(Value *value);

class CommonSubExper : public Pass
{
public:
  CommonSubExper(Module *m) : Pass(m) {}
  void run();
  void InitSets(BasicBlock *BB);
  void CommonSubExperDelete(BasicBlock *BB);
  bool is_main(Function *func)
  {
    return func->get_name() == "main";
  }
  void InitSideEffectFunc(Module *m);
  bool isSideEffectFunc(Function *func)
  {
    return SideEffectFunc.find(func) != SideEffectFunc.end();
  }
  bool isLocalStore(StoreInst *store_ins);
  bool isArgArrayPtr(Value *ptr);
  bool isGlobalArgArrayPtr(Value *val);
  bool isEqualFirstPtr(Value *ptr1, Value *ptr2);
private:
  std::set<Function *> SideEffectFunc;
  //map for cur BB
  std::map<std::tuple<Value *, Instruction::OpID, Value *>, Value *> BBop;    //操作数都不是常数的运算表达式
  std::map<std::tuple<Value *, Instruction::OpID, int>, Value *>BBop_r_int; //右操作数是常数的运算表达式
  std::map<std::tuple<int, Instruction::OpID, Value *>, Value *> BBop_l_int; //左操作数是常数的运算表达式
  std::map<std::tuple<Value *, Instruction::OpID, float>, Value *> BBop_r_fp; //右操作数是常数的运算表达式
  std::map<std::tuple<float, Instruction::OpID, Value *>, Value *> BBop_l_fp; //左操作数是常数的运算表达式
  std::map<Value *, Value *> BBload;
  std::map<std::tuple<Value *, Value *>, Value *> BBgep;
  std::map<std::tuple<Value *, int>, Value *> BBgep_const;
  //map for func
  std::map<BasicBlock *,std::map<std::tuple<Value *, Instruction::OpID, Value *>, Value *>> op;    //操作数都不是常数的运算表达式
  std::map<BasicBlock *,std::map<std::tuple<Value *, Instruction::OpID, int>, Value *>>op_r_int; //右操作数是常数的运算表达式
  std::map<BasicBlock *,std::map<std::tuple<int, Instruction::OpID, Value *>, Value *>> op_l_int; //左操作数是常数的运算表达式
  std::map<BasicBlock *,std::map<std::tuple<Value *, Instruction::OpID, float>, Value *>> op_r_fp; //右操作数是常数的运算表达式
  std::map<BasicBlock *,std::map<std::tuple<float, Instruction::OpID, Value *>, Value *>> op_l_fp; //左操作数是常数的运算表达式
  std::map<BasicBlock *,std::map<Value *, Value *>> load;
  std::map<BasicBlock *,std::map<std::tuple<Value *, Value *>, Value *>> gep;
  std::map<BasicBlock *,std::map<std::tuple<Value *, int>, Value *>> gep_const;
};
#endif