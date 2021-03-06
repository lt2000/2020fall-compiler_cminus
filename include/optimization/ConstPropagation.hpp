#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"
#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>

// tips: 用来判断value是否为ConstantFP/ConstantInt
ConstantFP *cast_constantfp(Value *value);
ConstantInt *cast_constantint(Value *value);

// tips: ConstFloder类

class ConstFolder
{
public:
    ConstFolder(Module *m) : module_(m) {}
    ConstantInt *compute(
        Instruction::OpID op,
        ConstantInt *value1,
        ConstantInt *value2);
    ConstantFP *compute(
        Instruction::OpID op,
        ConstantFP *value1,
        ConstantFP *value2);
    ConstantInt *compute(
        FCmpInst::CmpOp op,
        ConstantFP *value1,
        ConstantFP *value2);
    ConstantInt *compute(
        CmpInst::CmpOp op,
        ConstantInt *value1,
        ConstantInt *value2);

private:
    Module *module_;
};

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m)
    {
        flod = new ConstFolder(m);         //常量折叠初始化
        INT32_T = Type::get_int32_type(m); //为了判断全局变量的类型
        FLOAT_T = Type::get_float_type(m);
    }
    void run();
    void CompressPath(Function *func);
    int global_def_func(); //全局变量在其他子函数被定值，就不能再传播

private:
    ConstFolder *flod;
    std::map<Value *, ConstantInt *> global_def_int;
    std::map<Value *, ConstantFP *> global_def_fp;
    std::map<Value *, std::pair<int, Function *>> global_def_call; //第一个int表示该全局变量在多少个函数中被定值，第二个func标志该全局变量当前在那个func中
    Type *INT32_T;
    Type *FLOAT_T;
};

#endif