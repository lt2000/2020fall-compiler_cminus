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
  auto module = new Module("Cminus code");  
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);
  Type *floatType = Type::get_float_type(module);             //定义浮点型
  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);

  auto retAlloca = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0

  auto a = builder->create_alloca(floatType);                 //给a分配空间
  builder->create_store(CONST_FP(5.555), a);                  //给a赋值
  auto aload = builder->create_load(a);
  auto fcmp = builder->create_fcmp_lt(aload, CONST_INT(1));    //条件判断
  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
  auto retBB = BasicBlock::create(module, "retBB",mainFun);  
  builder->create_cond_br(fcmp, trueBB, falseBB);

  builder->set_insert_point(trueBB);                            //a>1
  builder->create_store(CONST_INT(233), retAlloca);
  builder->create_br(retBB);                                    //转到返回

  builder->set_insert_point(falseBB);                           //a<=1
  builder->create_store(CONST_INT(0), retAlloca);
  builder->create_br(retBB);                                     //转到返回

  builder->set_insert_point(retBB);                             // ret分支
  auto retLoad = builder->create_load(retAlloca);
  builder->create_ret(retLoad);
  
  std::cout << module->print();
  delete module;
  return 0;
}