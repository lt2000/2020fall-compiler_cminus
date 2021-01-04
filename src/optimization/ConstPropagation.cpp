#include "ConstPropagation.hpp"
#include "logging.hpp"
#define DEBUG
// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute( //整型值的折叠
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)

{
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);
        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantFP *ConstFolder::compute( //浮点型的折叠
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2)
{

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);
        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get((c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::compute( //比较指令整型折叠
    CmpInst::CmpOp op,
    ConstantInt *value1,
    ConstantInt *value2)
{
    // EQ, // ==
    // NE, // !=
    // GT, // >
    // GE, // >=
    // LT, // <
    // LE  // <=
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case CmpInst::EQ:
        return ConstantInt::get(c_value1 == c_value2, module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get(c_value1 != c_value2, module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case CmpInst::GE:
        return ConstantInt::get((c_value1 >= c_value2), module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get((c_value1 < c_value2), module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get((c_value1 <= c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::compute( //比较指令浮点型折叠
    FCmpInst::CmpOp op,
    ConstantFP *value1,
    ConstantFP *value2)
{
    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case FCmpInst::EQ:
        return ConstantInt::get(c_value1 == c_value2, module_);
        break;
    case FCmpInst::NE:
        return ConstantInt::get(c_value1 != c_value2, module_);
        break;
    case FCmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case FCmpInst::GE:
        return ConstantInt::get((c_value1 >= c_value2), module_);
        break;
    case FCmpInst::LT:
        return ConstantInt::get((c_value1 < c_value2), module_);
        break;
    case FCmpInst::LE:
        return ConstantInt::get((c_value1 <= c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}

int ConstPropagation::global_def_func()//得到全局变量被定值的函数个数
{
    for (auto func : m_->get_functions())
    {
        for (auto bb : func->get_basic_blocks())
        {
            for (auto instr : bb->get_instructions())
            {
                if (instr->is_store())
                {
                    auto val1 = dynamic_cast<StoreInst *>(instr)->get_lval(); //store的地址
                    auto globalval = dynamic_cast<GlobalVariable *>(val1);
                    if (globalval)
                    {
                        if (global_def_call.find(globalval) == global_def_call.end())
                        {
                            global_def_call.insert({globalval, {1, func}});
                        }
                        else if (global_def_call.find(globalval)->second.second != func)
                        {
                            global_def_call.find(globalval)->second.first++;
                            global_def_call.find(globalval)->second.second = func;
                        }
                    }
                }
            }
        }
    }
}

void ConstPropagation::run()
{
    // 从这里开始吧！
    global_def_func();
    for (auto func : m_->get_functions())
    {
        for (auto bb : func->get_basic_blocks())
        {
            global_def_int.clear(); //在每个bb内实现全局变量的常量传播
            global_def_fp.clear();
#ifdef DEBUG
            std::cerr << "ConstPropagation: " << bb->get_name() << std::endl;
#endif
            ///////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////删除常数运算指令和比较指令、类型转换指令、零扩展指令、全局变量的load指令//////////////
            ///////////////////////////////////////////////////////////////////////////////////////////////////
            std::vector<Instruction *> wait_delete;
            for (auto instr : bb->get_instructions())
            {
#ifdef DEBUG
                std::cerr << "instr: " << instr->get_name() << std::endl;
#endif
                //运算指令
                if (instr->is_add() || instr->is_sub() || instr->is_mul() || instr->is_div() || instr->is_fadd() || instr->is_fsub() || instr->is_fmul() || instr->is_fdiv())
                {
                    if (cast_constantint(instr->get_operand(0)) != nullptr && cast_constantint(instr->get_operand(1)) != nullptr)
                    {

                        auto val1 = cast_constantint(instr->get_operand(0));
                        auto val2 = cast_constantint(instr->get_operand(1));
                        if (val1 && val2)
                        {
                            auto const_flod = flod->compute(instr->get_instr_type(), val1, val2);
                            instr->replace_all_use_with(const_flod);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (cast_constantfp(instr->get_operand(0)) != nullptr && cast_constantfp(instr->get_operand(1)) != nullptr)
                    {
                        auto val1 = cast_constantfp(instr->get_operand(0));
                        auto val2 = cast_constantfp(instr->get_operand(1));
                        if (val1 && val2)
                        {
                            auto const_flod = flod->compute(instr->get_instr_type(), val1, val2);
                            instr->replace_all_use_with(const_flod); //进行浮点型常量传播
                            wait_delete.push_back(instr);
                        }
                    }
                }
                else if (instr->is_fp2si()) //类型转换
                {
                    auto val = cast_constantfp(instr->get_operand(0));
                    if (val)
                    {
                        int intval = val->get_value();
                        auto const_flod = ConstantInt::get(intval, m_);
                        instr->replace_all_use_with(const_flod);
                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->is_si2fp()) //类型转换
                {
                    auto val = cast_constantint(instr->get_operand(0));
                    if (val)
                    {
                        float fpval = val->get_value();
                        auto const_flod = ConstantFP::get(fpval, m_);
                        instr->replace_all_use_with(const_flod);
                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->is_zext()) //零扩展
                {
                    auto val = cast_constantint(instr->get_operand(0));
                    if (val)
                    {
                        int intval = val->get_value();
                        auto const_flod = ConstantInt::get(intval, m_);
                        instr->replace_all_use_with(const_flod);
                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->is_cmp()) //比较指令
                {
                    auto val1 = cast_constantint(instr->get_operand(0));
                    auto val2 = cast_constantint(instr->get_operand(1));
                    if (val1 && val2)
                    {
                        auto const_flod = flod->compute(dynamic_cast<CmpInst *>(instr)->get_cmp_op(), val1, val2);

                        instr->replace_all_use_with(const_flod);
                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->is_fcmp()) //比较指令
                {
                    auto val1 = cast_constantfp(instr->get_operand(0));
                    auto val2 = cast_constantfp(instr->get_operand(1));
                    if (val1 && val2)
                    {
                        auto const_flod = flod->compute(dynamic_cast<FCmpInst *>(instr)->get_cmp_op(), val1, val2);
                        instr->replace_all_use_with(const_flod);

                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->is_load())
                {

                    auto globalval = dynamic_cast<GlobalVariable *>(instr->get_operand(0));
                    if (globalval != nullptr)
                    {
                        if (globalval->get_operand(0)->get_type() == INT32_T)
                        {
#ifdef DEBUG
                            std::cerr << "GLOBAL " << instr->get_operand(0)->get_name() << std::endl;
#endif
                            ConstantInt *val; //全局变量并不符合SSA规则,用一个向量记录其定值
                            if (global_def_int.find(globalval) != global_def_int.end())
                            {
                                val = global_def_int.find(globalval)->second;
                            }
                            else
                                val = nullptr;
                            if (val)
                            {
                                instr->replace_all_use_with(val);
                                wait_delete.push_back(instr);
                            }
                        }
                        else if (globalval->get_operand(0)->get_type() == FLOAT_T)
                        {
                            ConstantFP *val; //全局变量并不符合SSA规则,用一个向量记录其定值
                            if (global_def_fp.find(globalval) != global_def_fp.end())
                            {
                                val = global_def_fp.find(globalval)->second;
                            }
                            else
                                val = nullptr;
                            if (val)
                            {
                                instr->replace_all_use_with(val);
                                wait_delete.push_back(instr);
                            }
                        }
                    }
                }
                else if (instr->is_store())
                {
                    auto val1 = dynamic_cast<StoreInst *>(instr)->get_lval(); //store的地址
                    auto globalval = dynamic_cast<GlobalVariable *>(val1);
                    if (globalval)
                    {
                        if (global_def_call.find(val1)->second.first == 1)//只有全局变量在一个函数内被定值才进行传播
                        {
                            if (instr->get_operand(0)->get_type() == INT32_T) // 整型
                            {
#ifdef DEBUG
                                std::cerr << "XIXIXIXIIXIXIX " << instr->get_operand(0)->get_name() << std::endl;
#endif
                                auto val2 = cast_constantint(dynamic_cast<StoreInst *>(instr)->get_rval());
                                if (val2)
                                {

                                    if (global_def_int.find(globalval) != global_def_int.end())
                                    {
                                        auto key = global_def_int.find(globalval);
                                        key->second = val2;
                                    }
                                    else
                                    {
                                        global_def_int.insert({globalval, val2});
                                    }
                                }
                                else //不是对全局变量进行常数定值，需要删除val1对应的条目，因为已经不是最新的了
                                {
#ifdef DEBUG
                                    std::cerr << "HAHAHHAHAHAH " << instr->get_operand(0)->get_name() << std::endl;
#endif
                                    if (global_def_int.find(val1) != global_def_int.end())
                                        global_def_int.erase(global_def_int.find(val1));
                                }
                            }
                            else if (instr->get_operand(0)->get_type() == FLOAT_T) //浮点型
                            {
                                auto val2 = cast_constantfp(dynamic_cast<StoreInst *>(instr)->get_rval());
                                if (val2)
                                {
                                    if (global_def_fp.find(globalval) != global_def_fp.end())
                                    {
                                        auto key = global_def_fp.find(globalval);
                                        key->second = val2;
                                    }
                                    else
                                    {
                                        global_def_fp.insert({globalval, val2});
                                    }
                                }
                                else //不是对全局变量进行常数定值，需要删除val1对应的条目，因为已经不是最新的了
                                {
                                    if (global_def_fp.find(val1) != global_def_fp.end())
                                        global_def_fp.erase(global_def_fp.find(val1));
                                }
                            }
                        }
                    }
                }
            }
            for (auto instr : wait_delete)
            {
                bb->delete_instr(instr);
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////删除无用分支///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    for (auto func : m_->get_functions())
    {
        for (auto bb : func->get_basic_blocks())
        {
            if (bb->get_terminator()->is_br())
            {
                auto br = bb->get_terminator();
                if (dynamic_cast<BranchInst *>(br)->is_cond_br()) //条件br
                {
                    auto cond = dynamic_cast<ConstantInt *>(br->get_operand(0));
                    auto truebb = br->get_operand(1);
                    auto falsebb = br->get_operand(2);
                    if (cond)
                    {
                        if (cond->get_value() == 0) //truebb是无用分支
                        {
                            bb->delete_instr(br); //删除分支跳转指令
                            for (auto succ_bb : bb->get_succ_basic_blocks())
                            {
                                succ_bb->remove_pre_basic_block(bb);
                                if (succ_bb != falsebb)
                                {
                                    for (auto instr : succ_bb->get_instructions())
                                    {
                                        if (instr->is_phi())
                                        { //从phi指令中删除turebb的条目
                                            for (int i = 1; i < instr->get_num_operand(); i = i + 2)
                                            {

                                                if (instr->get_operand(i) == bb)
                                                {
                                                    instr->remove_operands(i - 1, i);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            BranchInst::create_br(dynamic_cast<BasicBlock *>(falsebb), bb); //直接跳转到false
                            bb->get_succ_basic_blocks().clear();
                            bb->add_succ_basic_block(dynamic_cast<BasicBlock *>(falsebb));
                        }
                        else
                        {
                            bb->delete_instr(br); //删除分支跳转指令
                            for (auto succ_bb : bb->get_succ_basic_blocks())
                            {
                                succ_bb->remove_pre_basic_block(bb);
                                if (succ_bb != truebb)
                                {
                                    for (auto instr : succ_bb->get_instructions())
                                    {
                                        if (instr->is_phi())
                                        { //从phi指令中删除turebb的条目
                                            for (int i = 1; i < instr->get_num_operand(); i = i + 2)
                                            {

                                                if (instr->get_operand(i) == bb)
                                                {
                                                    instr->remove_operands(i - 1, i);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            BranchInst::create_br(dynamic_cast<BasicBlock *>(truebb), bb); //直接跳转到true
                            bb->get_succ_basic_blocks().clear();
                            bb->add_succ_basic_block(dynamic_cast<BasicBlock *>(truebb));
                        }
                    }
                }
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////删除删除无用分支后产生的不可到达的块/////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    for (auto func : m_->get_functions())
    {
        std::vector<BasicBlock *> wait_delete;
        for (auto bb : func->get_basic_blocks())
        {

            if (bb->get_name() != "label_entry") //除了第一个块，没有前驱的块都是不可达的
            {
                if (bb->get_terminator()->is_br())
                {
                    if (bb->get_pre_basic_blocks().empty())
                    {
                        for (auto succ_bb : bb->get_succ_basic_blocks())
                        {
                            succ_bb->remove_pre_basic_block(bb);
                            for (auto instr : succ_bb->get_instructions())
                            {
                                if (instr->is_phi())
                                {
                                    for (int i = 1; i < instr->get_num_operand(); i = i + 2)
                                    {

                                        if (instr->get_operand(i) == bb)
                                        {
#ifdef DEBUG
                                            std::cerr << "deletephi: " << instr->get_operand(i)->get_name() << std::endl;
#endif
                                            instr->remove_operands(i - 1, i);
                                        }
                                    }
                                }
                            }
                        }
#ifdef DEBUG
                        std::cerr << "deletebb!:" << bb->get_name() << std::endl;
#endif
                        wait_delete.push_back(bb);
                    }
                }
            }
        }
        for (auto bb : wait_delete) //删除无法到达的块
        {
            func->remove(bb);
        }
        wait_delete.clear();
    }
}