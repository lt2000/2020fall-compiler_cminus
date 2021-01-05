#ifndef COMMOCSUBEXPER_HPP
#define COMMOCSUBEXPER_HPP
#include "BasicBlock.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.hpp"
#include <tuple>

ConstantFP *constantfp(Value *value);
ConstantInt *constantint(Value *value);

class CommonSubExper : public Pass
{
public:
  CommonSubExper(Module *m) : Pass(m) {}
  void run();
  bool is_main(Function *func)
  {
    return func->get_name() == "main";
  }
  void InitStoreFunc(Module *m);
  bool isStoreFunc(Function *func)
  {
    return StoreFunc.find(func) != StoreFunc.end();
  }
  bool isLocalStore(StoreInst *store_ins);
private:
  std::set<Function *> StoreFunc;
  std::map<std::tuple<Value *, Instruction::OpID, Value *>, Value *> op;    //操作数都不是常数的运算表达式
  std::map<std::tuple<Value *, Instruction::OpID, int>, Value *> op_r_int; //右操作数是常数的运算表达式
  std::map<std::tuple<int, Instruction::OpID, Value *>, Value *> op_l_int; //左操作数是常数的运算表达式
  std::map<std::tuple<Value *, Instruction::OpID, float>, Value *> op_r_fp; //右操作数是常数的运算表达式
  std::map<std::tuple<float, Instruction::OpID, Value *>, Value *> op_l_fp; //左操作数是常数的运算表达式
  std::map<Value *, Value *> load;
  std::map<std::tuple<Value *, Value *>, Value *> gep;
  std::map<std::tuple<Value *, int>, Value *> gep_const;
};
#endif