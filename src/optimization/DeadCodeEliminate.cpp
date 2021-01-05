#include "DeadCodeEliminate.hpp"
//#define DEBUG
void DeadCodeEliminate::run()
{
  deleteDeadFunc(m_);
  InitSideEffectFunc(m_);
  for (auto func : m_->get_functions())
  {
    if (!is_main(func))
      deleteDeadRet(func);

    for (auto func : m_->get_functions())
    {
      deleteDeadInst(func);
      deleteDeadStore(func);
      deleteDeadInst(func);
    }
  }
}

bool DeadCodeEliminate::isLocalStore(StoreInst *store)
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

bool DeadCodeEliminate::isArgArrayStore(StoreInst *store)
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

bool DeadCodeEliminate::isGlobalArgArrayPtr(Value *val)
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

void DeadCodeEliminate::InitSideEffectFunc(Module *m)
{
  std::set<std::pair<CallInst *, Function *>> call_list;
  for (auto func : m->get_functions())
  {
    if (func->get_num_basic_blocks() == 0 || is_main(func))
      continue;
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

void DeadCodeEliminate::deleteDeadFunc(Module *m)
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

void DeadCodeEliminate::markUse(Instruction *inst,
                                std::unordered_set<Instruction *> &worklist)
{
  if (worklist.find(inst) != worklist.end())
    return;
  worklist.insert(inst);
  for (auto op : inst->get_operands())
  {
    auto op_inst = dynamic_cast<Instruction *>(op);
    if (op_inst)
      markUse(op_inst, worklist);
  }
}

void DeadCodeEliminate::deleteDeadInst(Function *func)
{
  std::unordered_set<Instruction *> worklist;
  for (auto bb : func->get_basic_blocks())
    for (auto inst : bb->get_instructions())
      if (isSideEffect(inst))
        markUse(inst, worklist);

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

void DeadCodeEliminate::deleteDeadRet(Function *func)
{
  bool flag = true;
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
    for (auto bb : func->get_basic_blocks())
    {
      for (auto instr : bb->get_instructions())
      {
        if (instr->is_ret() && instr->get_num_operand() == 1)
        {
          instr->remove_use_of_ops();
          instr->set_operand(0, ConstantInt::get(0, m_));
        }
      }
    }
  }
}

bool DeadCodeEliminate::isEqualFirstPtr(Value *ptr1, Value *ptr2)
{
  if (ptr1 == ptr2)
    return true;
  else
  {
    auto instr1 = dynamic_cast<Instruction *>(ptr1);
    auto instr2 = dynamic_cast<Instruction *>(ptr2);
    if (!instr1->is_gep() || !instr2->is_gep())
      return false;
    else
      return instr1->get_operand(0) == instr2->get_operand(0);
  }
}

bool DeadCodeEliminate::isEqualIntr(Instruction *instr1, Instruction *instr2)
{
  if (instr1->get_num_operand() != instr2->get_num_operand())
    return false;
  else
  {
    for (int i = 0; i < instr1->get_num_operand(); i++)
    {
      if (instr1->get_operand(i) != instr2->get_operand(i))
        return false;
    }
  }
}

void DeadCodeEliminate::dealStore(StoreInst *store, std::unordered_set<StoreInst *> &pre_stores,
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

void DeadCodeEliminate::deleteDeadStore(Function *func)
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
        auto flag = false;
        for (auto pre : pre_geps)
        {
          if (isEqualIntr(pre, instr))
          {
            wait_remove.push_back(instr);
            instr->replace_all_use_with(pre);
            flag = true;
            break;
          }
        }
        if (!flag)
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