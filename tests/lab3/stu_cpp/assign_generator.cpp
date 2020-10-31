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
  auto *arrayType = ArrayType::get(Int32Type, 10);            //定义数组类型
  auto  a = builder->create_alloca(arrayType);                //为数组分配空间
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0

  auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});  // GEP: 指向a[0]
  builder->create_store(CONST_INT(10), a0GEP);                        //a[0]=10
  a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});       // GEP: 指向a[0]
  auto a0load = builder->create_load(a0GEP);                          //a[0] load出来
  auto mul = builder->create_imul(a0load, CONST_INT(2));              //a[0]*2

  auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});  // GEP: 指向a[1]
  builder->create_store(mul, a1GEP);                                  //将结果store进a[1]
  a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});
  auto a1load = builder->create_load(a1GEP);                          //将结果从a[1]中load出来
  builder->create_ret(a1load);
  std::cout << module->print();
  delete module;
  return 0;
}