#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
  auto module = new Module("Cminus code");  // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);

  auto retAlloca = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0
  auto a = builder->create_alloca(Int32Type);                 //给a,i分配内存
  auto i = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(10), a);                    //给a,i赋值
  builder->create_store(CONST_INT(0), i); 
  auto whilecond = BasicBlock::create(module, "whilecond", mainFun); 
  auto whilebody = BasicBlock::create(module, "whilebody", mainFun); 
  auto whileend = BasicBlock::create(module, "whileend", mainFun);   
  builder->create_br(whilecond);                               //转到循环条件分支

  builder->set_insert_point(whilecond);                       //循环条件分支
  auto iteration = builder->create_load(a);
  auto icmp = builder->create_icmp_lt(a,CONST_INT(10));       //循环条件判断
  builder->create_cond_br(icmp,whilebody,whileend);           

  builder->set_insert_point(whilebody);                       //循环体分支
  iteration = builder->create_load(i);
  auto add = builder->create_iadd(iteration,CONST_INT(1));
  builder->create_store(add, i);                              //i++
  iteration = builder->create_load(i);
  auto a_iteration = builder->create_load(a);  
  add = builder->create_iadd(a_iteration,iteration);
  builder->create_store(add, a);                              //a=a+i
  builder->create_br(whilecond);

  builder->set_insert_point(whileend);                       //循环结束分支
  a_iteration = builder->create_load(a);
  builder->create_ret(a_iteration);
  std::cout << module->print();
  delete module;
  return 0;
}