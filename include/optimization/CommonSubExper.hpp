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
  ~CommonSubExper(){};
  void run();
  void CommonSubExperElimination();
  Value *getBinaryLop(Instruction *instr);
  Value *getBinaryRop(Instruction *instr);
  Value *getLoadPtr(Instruction *instr);
  Value *getLoadOffset(Instruction *instr);
  int getLoadOffsetShl(Instruction *instr);

  Value *getGepPtr(Instruction *instr);
  Value *getGepOffset(Instruction *instr);

private:
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