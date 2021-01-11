#include "DeadCodeEliminate.hpp"
#define DEBUG
void DeadCodeDelete::run()
{
  deleteDeadFunc(m_);
  InitSets(m_);
  for (auto func : m_->get_functions())
  {
    if (!is_main(func))
      deleteDeadRet(func);
  }
  for (auto func : m_->get_functions())
  {
    deleteDeadInst(func);
    //CompressPath(func);
    deleteDeadStore(func);
    deleteDeadInst(func);
  }
  for (auto func : m_->get_functions())
    DeleteFuncDeadArg(func);
  for (auto func : m_->get_functions())
    deleteDeadInst(func);
  deleteDeadFunc(m_);
}

void DeadCodeDelete::DeleteFuncDeadArg(Function *func)
{
  if (func->get_num_basic_blocks() == 0)
    return;
  int i = 1;
  auto func_use = func->get_use_list();
  for (auto arg : func->get_args())
  {
    if (arg->get_use_list().empty())
    {
      auto arg_type = arg->get_type();
      if (arg_type->is_integer_type())
      {
        for (auto use : func_use)
        {
          auto instr = dynamic_cast<Instruction *>(use.val_);
          instr->set_operand(i, ConstantInt::get(0, m_));
        }
      }
      else if (arg_type->is_float_type())
      {
        for (auto use : func_use)
        {
          auto instr = dynamic_cast<Instruction *>(use.val_);
          instr->set_operand(i, ConstantFP::get(0, m_));
        }
      }
    }
    i++;
  }
}

void DeadCodeDelete::CompressPath(Function *func)
{
  std::set<BasicBlock *> wait_merge;
  for (auto BB : func->get_basic_blocks())
  {
    if (BB->get_pre_basic_blocks().size() == 1)
    {
      auto preBB = *(BB->get_pre_basic_blocks().begin());
      if (preBB->get_succ_basic_blocks().size() == 1)
        wait_merge.insert(BB);
    }
  }
  for (auto BB : wait_merge)
  {
    auto preBB = *(BB->get_pre_basic_blocks().begin());
    auto br_ins = preBB->get_terminator();
    if (br_ins->is_br())
      preBB->delete_instr(br_ins);
    for (auto instr : BB->get_instructions())
      preBB->add_instruction(instr);
    preBB->remove_succ_basic_block(BB);
    BB->remove_pre_basic_block(preBB);
    auto succBB_list = BB->get_succ_basic_blocks();
    for (auto succBB : succBB_list)
    {
      // succBB->remove_pre_basic_block(BB);
      // BB->remove_succ_basic_block(succBB);
      succBB->add_pre_basic_block(preBB);
      preBB->add_succ_basic_block(succBB);
    }
    BB->replace_all_use_with(preBB);
    func->remove(BB);
  }
}

bool DeadCodeDelete::isLocalStore(StoreInst *store)
{
  auto l_val = store->get_lval();
  if (dynamic_cast<GlobalVariable *>(l_val))
    return false;
  else
  {
    auto instr = dynamic_cast<Instruction *>(l_val);
    if (!instr || !instr->is_gep())
      return false;
    else
    {
      auto first_addr = instr->get_operand(0);
      auto addr_ins = dynamic_cast<Instruction *>(first_addr);
      if (!addr_ins || !addr_ins->is_alloca())
        return false;
      else
        return true;
    }
  }
}

bool DeadCodeDelete::isArgArrayStore(StoreInst *store)
{
  auto l_val = store->get_lval();
  if (dynamic_cast<GlobalVariable *>(l_val))
    return false;
  else
  {
    auto instr = dynamic_cast<Instruction *>(l_val);
    if (!instr || !instr->is_gep())
      return false;
    else
    {
      auto first_addr = instr->get_operand(0);
      if (dynamic_cast<GlobalVariable *>(first_addr))
        return false;
      auto addr_ins = dynamic_cast<Instruction *>(first_addr);
      if (addr_ins && addr_ins->is_alloca())
        return false;
      else
        return true;
    }
  }
}

bool DeadCodeDelete::isGlobalArgArrayPtr(Value *val)
{
  if (dynamic_cast<GlobalVariable *>(val))
    return false;
  else
  {
    auto instr = dynamic_cast<Instruction *>(val);
    if (!instr || !instr->is_gep())
      return false;
    else
    {
      auto first_addr = instr->get_operand(0);
      if (dynamic_cast<GlobalVariable *>(first_addr))
        return true;
      auto addr_ins = dynamic_cast<Instruction *>(first_addr);
      if (addr_ins && addr_ins->is_alloca())
        return false;
      else
        return true;
    }
  }
}

void DeadCodeDelete::InitSets(Module *m)
{
  std::set<std::pair<CallInst *, Function *>> call_list;
  for (auto func : m->get_functions())
  {
    if (func->get_num_basic_blocks() == 0 || is_main(func))
    {
      SideEffectFunc.insert(func);
      continue;
    }
    bool side_effect = false;
    for (auto bb : func->get_basic_blocks())
    {
      if (side_effect)
        break;
      for (auto inst : bb->get_instructions())
      {
        if (inst->is_store())
        {
          auto store = dynamic_cast<StoreInst *>(inst);
          if (!isLocalStore(store))
          {
            side_effect = true;
            SideEffectFunc.insert(func);
            break;
          }
        }
        else if (inst->is_call())
        {
          auto call = static_cast<CallInst *>(inst);
          call_list.insert(std::make_pair(call, func));
        }
      }
    }
  }
  bool changed = true;
  while (changed)
  {
    changed = false;
    std::set<std::pair<CallInst *, Function *>> remove_calls;
    for (auto cur_call : call_list)
    {
      if (SideEffectFunc.find(cur_call.second) != SideEffectFunc.end())
        remove_calls.insert(cur_call);
      else
      {
        auto op_0 = cur_call.first->get_operand(0);
        auto op_func = dynamic_cast<Function *>(op_0);
        if (SideEffectFunc.find(op_func) != SideEffectFunc.end())
        {
          remove_calls.insert(cur_call);
          SideEffectFunc.insert(cur_call.second);
          changed = true;
        }
      }
    }
    for (auto remove_call : remove_calls)
      call_list.erase(remove_call);
  }
}

void DeadCodeDelete::deleteDeadFunc(Module *m)
{
  auto func_list = m->get_functions();
  auto changed = true;
  while (changed)
  {
    changed = false;
    std::set<Function *> remove_funcs;
    for (auto itfunc : func_list)
    {
      if (!itfunc->get_basic_blocks().empty() &&
          itfunc->get_use_list().empty() && !is_main(itfunc))
      {
        remove_funcs.insert(itfunc);
        changed = true;
      }
    }
    for (auto itfunc : remove_funcs)
    {
      std::set<CallInst *> calls;
      for (auto BB : itfunc->get_basic_blocks())
      {
        for (auto instr : BB->get_instructions())
          if (instr->is_call())
            calls.insert(dynamic_cast<CallInst *>(instr));
      }
      for (auto call : calls)
      {
        auto callfunc = call->get_operand(0);
#ifdef DEBUG
        std::cerr << "remove callfunc " << callfunc->get_name() << std::endl;
#endif
        callfunc->remove_use(call);
      }
      func_list.remove(itfunc);
      m->remove_function(itfunc);
    }
  }
}

void DeadCodeDelete::CompleteWorklist(Instruction *inst,
                                      std::unordered_set<Instruction *> &worklist)
{
  if (worklist.find(inst) != worklist.end())
    return;
  worklist.insert(inst);
  for (auto op : inst->get_operands())
  {
    auto op_inst = dynamic_cast<Instruction *>(op);
    if (op_inst)
      CompleteWorklist(op_inst, worklist);
  }
}

void DeadCodeDelete::deleteDeadInst(Function *func)
{
  std::unordered_set<Instruction *> worklist;
  for (auto bb : func->get_basic_blocks())
    for (auto inst : bb->get_instructions())
      if (isSideEffect(inst))
        CompleteWorklist(inst, worklist);

  for (auto bb : func->get_basic_blocks())
  {
    std::unordered_set<Instruction *> remove_list;
    for (auto inst : bb->get_instructions())
      if (worklist.find(inst) == worklist.end())
        remove_list.insert(inst);
    for (auto inst : remove_list)
      bb->delete_instr(inst);
  }
}

void DeadCodeDelete::deleteDeadRet(Function *func)
{
  bool flag = true;
  if (func->get_function_type()->get_return_type()->is_void_type())
    return;
  for (auto use : func->get_use_list())
  {
    if (!use.val_->get_use_list().empty())
    {
      flag = false;
      break;
    }
  }
  if (flag)
  {
    auto ret_type = func->get_function_type()->get_return_type();
    for (auto bb : func->get_basic_blocks())
    {
      auto instr = bb->get_terminator();
      if (instr->is_ret() && instr->get_num_operand() == 1)
      {
        instr->remove_use_of_ops();
        if (ret_type->is_integer_type())
          instr->set_operand(0, ConstantInt::get(0, m_));
        else if (ret_type->is_float_type())
          instr->set_operand(0, ConstantFP::get(0, m_));
      }
    }
  }
}

bool DeadCodeDelete::isEqualFirstPtr(Value *ptr1, Value *ptr2)
{
  if (ptr1 == ptr2)
    return true;
  else
  {
    auto instr1 = dynamic_cast<Instruction *>(ptr1);
    auto instr2 = dynamic_cast<Instruction *>(ptr2);
    if (!instr1 || !instr2)
      return false;
    if(!instr1->is_gep() || !instr2->is_gep() )
      return false;
    else
      return instr1->get_operand(0) == instr2->get_operand(0);
  }
}

bool DeadCodeDelete::isEqualGep(Instruction *instr1, Instruction *instr2)
{
  if (instr1->get_num_operand() != instr2->get_num_operand())
    return false;
  else
  {
    for (int i = 0; i < instr1->get_num_operand(); i++)
    {
      auto op_1 = instr1->get_operand(i);
      auto op_2 = instr2->get_operand(i);
      if (op_1 == op_2)
        continue;
      else
      {
        auto op1_const = dynamic_cast<ConstantInt *>(op_1);
        auto op2_const = dynamic_cast<ConstantInt *>(op_2);

        if (!op1_const  || !op2_const )
          return false;
        else if (op1_const->get_value() != op2_const->get_value())
          return false;
      }
    }
  }
  return true;
}

void DeadCodeDelete::dealStore(StoreInst *store, std::unordered_set<StoreInst *> &pre_stores,
                               std::unordered_set<LoadInst *> &pre_loads,
                               std::vector<Instruction *> &wait_remove)
{
  auto l_val = store->get_lval();
  StoreInst *pre_store = nullptr;
  std::set<StoreInst *> remove_stores;
  std::set<LoadInst *> remove_loads;
  if (!isArgArrayStore(store))
  {
    for (auto pre : pre_stores)
    {
      if (isEqualStoreFirstPtr(pre, store))
      {
        if (isEqualStorePtr(pre, store))
          wait_remove.push_back(pre);
        pre_store = pre;
        break;
      }
    }
    if (pre_store)
      pre_stores.erase(pre_store);
    else
    {
      for (auto pre : pre_loads)
      {
        if (isEqualStoreLoadFirstPtr(store, pre))
          remove_loads.insert(pre);
      }
    }
  }
  else
  //isArgArrayStore
  {
    for (auto pre : pre_stores)
    {
      if (isGlobalArgArrayPtr(pre->get_lval()))
      {
        if (isEqualStorePtr(pre, store))
        {
          wait_remove.push_back(pre);
        }
        remove_stores.insert(pre);
      }
    }
    for (auto pre : pre_loads)
    {
      if (isGlobalArgArrayPtr(pre->get_lval()))
        remove_loads.insert(pre);
    }
  }
  pre_stores.insert(store);
  for (auto pre : remove_stores)
    pre_stores.erase(pre);
  for (auto pre : remove_loads)
    pre_loads.erase(pre);
}

void DeadCodeDelete::deleteDeadStore(Function *func)
{
  for (auto bb : func->get_basic_blocks())
  {
    std::unordered_set<StoreInst *> pre_stores;
    std::unordered_set<LoadInst *> pre_loads;
    std::unordered_set<Instruction *> pre_geps;
    std::vector<Instruction *> wait_remove;
    for (auto instr : bb->get_instructions())
    {
      if (instr->is_gep())
      {
        auto gep_flag = false;
        for (auto pre : pre_geps)
        {
          if (isEqualGep(pre, instr))
          {
            wait_remove.push_back(instr);
            instr->replace_all_use_with(pre);
            gep_flag = true;
            break;
          }
        }
        if (!gep_flag)
          pre_geps.insert(instr);
      }
      else if (instr->is_store())
      {
        auto new_store = static_cast<StoreInst *>(instr);
        dealStore(new_store, pre_stores, pre_loads, wait_remove);
      }
      else if (instr->is_load())
      {
        auto load = static_cast<LoadInst *>(instr);
        auto flag = false;
        for (auto pre : pre_stores)
        {
          if (isEqualStoreLoadFirstPtr(pre, load))
          {
            flag = true;
            if (isEqualStoreLoadPtr(pre, load))
            {
              load->replace_all_use_with(pre->get_rval());
              wait_remove.push_back(instr);
            }
            else
              pre_loads.insert(load);
            break;
          }
        }
        if (!flag)
        {
          for (auto pre : pre_loads)
          {
            if (isEqualLoadPtr(load, pre))
            {
              load->replace_all_use_with(pre);
              wait_remove.push_back(instr);
              flag = true;
              break;
            }
          }
          if (!flag)
            pre_loads.insert(load);
        }
      }
      else if (instr->is_call())
      {
        auto call = static_cast<CallInst *>(instr);
        if (isSideEffectFunc(dynamic_cast<Function *>(call->get_operand(0))))
        {
          pre_stores.clear();
          pre_loads.clear();
        }
      }
    }
    for (auto instr : wait_remove)
    {
      bb->delete_instr(instr);
    }
  }
}