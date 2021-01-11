#include "TailRecursion.hpp"
#define DEBUG

void TailRecursion::run()
{
    for (auto func : m_->get_functions())
    {
        if (is_main(func) || func->get_num_basic_blocks() == 0)
            continue;
        if (func->get_return_type()->is_void_type())
            VoidFindTailRecursion(func);
        else
            BackFindTailRecursion(func);
        if (TailCalls.size() > 0)
            dealTailRecursion(func);
    }
}

bool TailRecursion::is_main(Function *func)
{
    return func->get_name() == "main";
}

bool TailRecursion::DirectTailRecursion(Function *func)
{
    auto flag = true;
    const_retBB.clear();
    std::set<int> const_int;
    std::set<float> const_fp;
    for (auto BB : func->get_basic_blocks())
    {
        auto ret = BB->get_terminator();
        if (ret->is_br())
            continue;
        auto ret_val = ret->get_operand(0);
        if (!dynamic_cast<Constant *>(ret_val))
        {
            auto ret_call = dynamic_cast<CallInst *>(ret_val);
            if (!ret_call)
            {
                flag = false;
                continue;
            }
            auto callfunc = dynamic_cast<Function *>(ret_call->get_operand(0));
            if (!callfunc || callfunc != func)
            {
                flag = false;
                continue;
            }
            TailCalls.insert({ret_call, BB});
        }
        else if (!flag)
            continue;
        else if (dynamic_cast<ConstantInt *>(ret_val))
        {
            const_retBB.insert(BB);
            auto ret_int = dynamic_cast<ConstantInt *>(ret_val)->get_value();
            bool inflag = false;
            for (auto it : const_int)
            {
                if (ret_int == it)
                    inflag = true;
                else
                    flag = false;
            }
            if (!inflag)
                const_int.insert(ret_int);
        }
        else
        {
            const_retBB.insert(BB);
            auto ret_fp = dynamic_cast<ConstantFP *>(ret_val)->get_value();
            bool inflag = false;
            for (auto it : const_fp)
            {
                if (ret_fp == it)
                    inflag = true;
                else
                    flag = false;
            }
            if (!inflag)
                const_fp.insert(ret_fp);
        }
    }
    return flag;
}

void TailRecursion::BackFindTailRecursion(Function *func)
{
    TailCalls.clear();
    auto flag = DirectTailRecursion(func);
    if (!flag)
        return;
    for (auto BB : const_retBB)
    {
        auto ret = BB->get_terminator();
        Instruction *call = nullptr;
        for (auto instr : BB->get_instructions())
        {
            if (instr != ret)
            {
                call = instr;
            }
            else
                break;
        }
        auto tail_call = dynamic_cast<CallInst *>(call);
        if (!tail_call)
            continue;
        auto callfunc = dynamic_cast<Function *>(tail_call->get_operand(0));
        if (!callfunc || callfunc != func)
            continue;
        TailCalls.insert({tail_call, BB});
    }
}

void TailRecursion::VoidFindTailRecursion(Function *func)
{
    TailCalls.clear();
    for (auto BB : func->get_basic_blocks())
    {
        auto ret = BB->get_terminator();
        if (ret->is_br())
            continue;
        Instruction *call = nullptr;
        for (auto instr : BB->get_instructions())
        {
            if (instr != ret)
            {
                call = instr;
            }
            else
                break;
        }
        auto tail_call = dynamic_cast<CallInst *>(call);
        if (!tail_call)
            continue;
        auto callfunc = dynamic_cast<Function *>(tail_call->get_operand(0));
        if (!callfunc || callfunc != func)
            continue;
        TailCalls.insert({tail_call, BB});
    }
}

void TailRecursion::AddBBandReplace(Function *func)
{
    auto entry = func->get_entry_block();
    new_BB = BasicBlock::create(m_, "", func);
    std::set<BasicBlock *> remove_succ;
    for (auto ins : entry->get_instructions())
        new_BB->add_instruction(ins);
    entry->get_instructions().clear();
    for (auto succ : entry->get_succ_basic_blocks())
    {
        remove_succ.insert(succ);
        new_BB->add_succ_basic_block(succ);
        succ->add_pre_basic_block(new_BB);
        succ->remove_pre_basic_block(entry);
    }
    for (auto it : remove_succ)
        entry->remove_succ_basic_block(it);
    entry->replace_all_use_with(new_BB);
    BranchInst::create_br(new_BB, entry);
}

void TailRecursion::AddPhis(Function *func)
{
    auto entry = func->get_entry_block();
    int size = 0;
    Phis.clear();
    for (auto arg : func->get_args())
    {
        size++;
        auto phi = PhiInst::create_phi(arg->get_type(), new_BB);
        phi->add_phi_pair_operand(arg, entry);
        new_BB->add_instr_begin(phi);
#ifdef DEBUG
        std::cerr << phi->print() << std::endl;
#endif
        Phis.insert({size, phi});
    }
    for (auto pair : TailCalls)
    {
        auto call_instr = pair.first;
        for (int i = 1; i <= size; i++)
        {
            Phis[i]->add_phi_pair_operand(call_instr->get_operand(i), pair.second);
        }
        pair.second->delete_instr(pair.first);
        pair.second->delete_instr(pair.second->get_terminator());
        BranchInst::create_br(new_BB, pair.second);
    }
}

void TailRecursion::dealTailRecursion(Function *func)
{
    AddBBandReplace(func);
    AddPhis(func);
    int i = 1;
    for (auto arg : func->get_args())
    {
#ifdef DEBUG
        std::cerr << "i = " << i << std::endl;
#endif
        auto cur_phi = Phis[i];
        auto flag = true;
        for (int j = 0; j < cur_phi->get_num_operand(); j += 2)
        {
            if (cur_phi->get_operand(j) != arg)
            {
                flag = false;
                break;
            }
        }
        if (flag)
            new_BB->delete_instr(cur_phi);
        else
        {
            auto use_list = arg->get_use_list();
            for (auto use : use_list)
            {
                auto instr = dynamic_cast<Instruction *>(use.val_);
                if (!instr || instr == cur_phi)
                    continue;
                else
                    instr->set_operand(use.arg_no_, cur_phi);
            }
        }
        i++;
    }
}