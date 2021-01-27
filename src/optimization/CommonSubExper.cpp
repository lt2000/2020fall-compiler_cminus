#include "CommonSubExper.hpp"
#define DEBUG
ConstantFP *constantfp(Value *value)
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
ConstantInt *constantint(Value *value)
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

void CommonSubExper::CommonSubExperDelete(BasicBlock *BB)
{
  std::vector<Instruction *> wait_delete;
  for (auto instr : BB->get_instructions())
  {
    if (instr->is_add() || instr->is_sub() || instr->is_mul() || instr->is_div() || instr->is_fadd() || instr->is_fsub() || instr->is_fmul() || instr->is_fdiv())
    {
      auto lval = instr->get_operand(0);
      auto rval = instr->get_operand(1);
      if (constantint(lval) != nullptr || constantint(rval) != nullptr)
      {
        auto lval_const = constantint(lval);
        auto rval_const = constantint(rval);
        if (lval_const)
        {
          if (BBop_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval}) != BBop_l_int.end())
          {
            auto iter = BBop_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval});
            instr->replace_all_use_with(iter->second);
            wait_delete.push_back(instr);
          }
          else
          {
            BBop_l_int.insert({{lval_const->get_value(), instr->get_instr_type(), rval}, instr});
          }
        }
        else if (rval_const)
        {
          if (BBop_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()}) != BBop_r_int.end())
          {
            auto iter = BBop_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()});
            instr->replace_all_use_with(iter->second);
            wait_delete.push_back(instr);
          }
          else
          {
            BBop_r_int.insert({{lval, instr->get_instr_type(), rval_const->get_value()}, instr});
          }
        }
      }
      else if (constantfp(lval) != nullptr || constantfp(rval) != nullptr)
      {
        //TODO float………………
        auto lval_const = constantfp(lval);
        auto rval_const = constantfp(rval);
        if (lval_const)
        {
          if (BBop_l_fp.find({lval_const->get_value(), instr->get_instr_type(), rval}) != BBop_l_fp.end())
          {
            auto iter = BBop_l_fp.find({lval_const->get_value(), instr->get_instr_type(), rval});
            instr->replace_all_use_with(iter->second);
            wait_delete.push_back(instr);
          }
          else
          {
            BBop_l_fp.insert({{lval_const->get_value(), instr->get_instr_type(), rval}, instr});
          }
        }
        else if (rval_const)
        {
          if (BBop_r_fp.find({lval, instr->get_instr_type(), rval_const->get_value()}) != BBop_r_fp.end())
          {
            auto iter = BBop_r_fp.find({lval, instr->get_instr_type(), rval_const->get_value()});
            instr->replace_all_use_with(iter->second);
            wait_delete.push_back(instr);
          }
          else
          {
            BBop_r_fp.insert({{lval, instr->get_instr_type(), rval_const->get_value()}, instr});
          }
        }
      }
      else
      {
        if (BBop.find({lval, instr->get_instr_type(), rval}) != BBop.end())
        {
          auto iter = BBop.find({lval, instr->get_instr_type(), rval});
          instr->replace_all_use_with(iter->second);
          wait_delete.push_back(instr);
        }
        else
        {
          BBop.insert({{lval, instr->get_instr_type(), rval}, instr});
        }
      }
    }
    else if (instr->is_gep())
    {
#ifdef DEBUG
      std::cerr << instr->get_name() << std::endl;
#endif
      auto base = instr->get_operand(0);
      auto offset = instr->get_operand(2);
      auto const_offset = constantint(offset);
      if (const_offset) //a = b[2]
      {
        if (BBgep_const.find({base, const_offset->get_value()}) != BBgep_const.end())
        {
#ifdef DEBUG
          std::cerr << "HAHAHAHAHHAHA" << std::endl;
          std::cerr << const_offset->get_value() << std::endl;
#endif
          auto iter = BBgep_const.find({base, const_offset->get_value()});
          instr->replace_all_use_with(iter->second);
          wait_delete.push_back(instr);
        }
        else
        {
          BBgep_const.insert({{base, const_offset->get_value()}, instr});
        }
      }
      else //a=b[i]
      {
        if (BBgep.find({base, offset}) != BBgep.end())
        {
          auto iter = BBgep.find({base, offset});
          instr->replace_all_use_with(iter->second);
          wait_delete.push_back(instr);
        }
        else
        {
          BBgep.insert({{base, offset}, instr});
        }
      }
    }
    else if (instr->is_load())
    {
      auto base = instr->get_operand(0);
      if (BBload.find(base) != BBload.end())
      {
        auto iter = BBload.find(base);
        instr->replace_all_use_with(iter->second);
        wait_delete.push_back(instr);
      }
      else
      {
        BBload.insert({base, instr});
      }
    }
    else if (instr->is_store()) //对数组元素或全局变量重新定值后，load中的值就失效了
    {
      std::vector<std::pair<Value *, Value *>> wait_delete_load;
      auto base = dynamic_cast<StoreInst *>(instr)->get_lval();
      if (!isGlobalArgArrayPtr(base))
      {
        for (auto iter : BBload)
        {
          if (isEqualFirstPtr(iter.first, base))
            wait_delete_load.push_back(iter);
        }
      }
      else if (isArgArrayPtr(base))
      {
        for (auto iter : BBload)
        {
          if (isGlobalArgArrayPtr(iter.first))
            wait_delete_load.push_back(iter);
        }
      }
      else
      {
        for (auto iter : BBload)
        {
          if (isArgArrayPtr(iter.first) || isEqualFirstPtr(iter.first, base))
            wait_delete_load.push_back(iter);
        }
      }

      for (auto iter : wait_delete_load)
      {
        BBload.erase(iter.first);
      }
    }
    else if (instr->is_call())
    {
      auto op_0 = instr->get_operand(0);
      auto callfunc = dynamic_cast<Function *>(op_0);
      if (isSideEffectFunc(callfunc))
        BBload.clear();
    }
  }
  for (auto instr : wait_delete)
  {
    BB->delete_instr(instr);
  }
  op[BB] = BBop;
  op_r_int[BB] = BBop_r_int;
  op_l_int[BB] = BBop_l_int;
  op_r_fp[BB] = BBop_r_fp;
  op_l_fp[BB] = BBop_l_fp;
  load[BB] = BBload;
  gep[BB] = BBgep;
  gep_const[BB] = BBgep_const;
}

void CommonSubExper::InitSets(BasicBlock *BB)
{
  BBop.clear();
  BBop_r_int.clear();
  BBop_l_int.clear();
  BBop_r_fp.clear();
  BBop_l_fp.clear();
  BBload.clear();
  BBgep.clear();
  BBgep_const.clear();
  std::set<BasicBlock *> preBB_set;
  bool flag = true;
  for (auto preBB : BB->get_pre_basic_blocks())
  {
    if (preBB != BB && op.find(preBB) != op.end())
      preBB_set.insert(preBB);
    else
      flag = false;
  }
  if (preBB_set.size() < 1)
    return;
  auto preBBbegin = *(preBB_set.begin());
  //is not loopbase
  if (flag)
    BBload = load[preBBbegin];
  BBop = op[preBBbegin];
  BBop_r_int = op_r_int[preBBbegin];
  BBop_l_int = op_l_int[preBBbegin];
  BBop_r_fp = op_r_fp[preBBbegin];
  BBop_l_fp = op_l_fp[preBBbegin];
  BBgep = gep[preBBbegin];
  BBgep_const = gep_const[preBBbegin];
  auto preBB = preBB_set.begin();
  preBB++;
  while (preBB != preBB_set.end())
  {
    auto cur_pre_BB = *preBB;
    std::set<std::pair<std::tuple<Value *, Instruction::OpID, Value *>, Value *>> remove_op;
    for (auto it : BBop)
    {
      if (op[cur_pre_BB].find(it.first) == op[cur_pre_BB].end())
        remove_op.insert(it);
      else if (op[cur_pre_BB][it.first] != it.second)
        remove_op.insert(it);
    }
    for (auto it : remove_op)
      BBop.erase(it.first);

    std::set<std::pair<std::tuple<Value *, Instruction::OpID, int>, Value *>> remove_op_r_int;
    for (auto it : BBop_r_int)
    {
      if (op_r_int[cur_pre_BB].find(it.first) == op_r_int[cur_pre_BB].end())
        remove_op_r_int.insert(it);
      else if (op_r_int[cur_pre_BB][it.first] != it.second)
        remove_op_r_int.insert(it);
    }
    for (auto it : remove_op_r_int)
      BBop_r_int.erase(it.first);

    std::set<std::pair<std::tuple<int, Instruction::OpID, Value *>, Value *>> remove_op_l_int;
    for (auto it : BBop_l_int)
    {
      if (op_l_int[cur_pre_BB].find(it.first) == op_l_int[cur_pre_BB].end())
        remove_op_l_int.insert(it);
      else if (op_l_int[cur_pre_BB][it.first] != it.second)
        remove_op_l_int.insert(it);
    }
    for (auto it : remove_op_l_int)
      BBop_l_int.erase(it.first);

    std::set<std::pair<std::tuple<Value *, Instruction::OpID, float>, Value *>> remove_op_r_fp;
    for (auto it : BBop_r_fp)
    {
      if (op_r_fp[cur_pre_BB].find(it.first) == op_r_fp[cur_pre_BB].end())
        remove_op_r_fp.insert(it);
      else if (op_r_fp[cur_pre_BB][it.first] != it.second)
        remove_op_r_fp.insert(it);
    }
    for (auto it : remove_op_r_fp)
      BBop_r_fp.erase(it.first);

    std::set<std::pair<std::tuple<float, Instruction::OpID, Value *>, Value *>> remove_op_l_fp;
    for (auto it : BBop_l_fp)
    {
      if (op_l_fp[cur_pre_BB].find(it.first) == op_l_fp[cur_pre_BB].end())
        remove_op_l_fp.insert(it);
      else if (op_l_fp[cur_pre_BB][it.first] != it.second)
        remove_op_l_fp.insert(it);
    }
    for (auto it : remove_op_l_fp)
      BBop_l_fp.erase(it.first);

    std::set<std::pair<Value *, Value *>> remove_load;
    for (auto it : BBload)
    {
      if (load[cur_pre_BB].find(it.first) == load[cur_pre_BB].end())
        remove_load.insert(it);
      else if (load[cur_pre_BB][it.first] != it.second)
        remove_load.insert(it);
    }
    for (auto it : remove_load)
      BBload.erase(it.first);

    std::set<std::pair<std::tuple<Value *, Value *>, Value *>> remove_gep;
    for (auto it : BBgep)
    {
      if (gep[cur_pre_BB].find(it.first) == gep[cur_pre_BB].end())
        remove_gep.insert(it);
      else if (gep[cur_pre_BB][it.first] != it.second)
        remove_gep.insert(it);
    }
    for (auto it : remove_gep)
      BBgep.erase(it.first);

    std::set<std::pair<std::tuple<Value *, int>, Value *>> remove_gep_const;
    for (auto it : BBgep_const)
    {
      if (gep_const[cur_pre_BB].find(it.first) == gep_const[cur_pre_BB].end())
        remove_gep_const.insert(it);
      else if (gep_const[cur_pre_BB][it.first] != it.second)
        remove_gep_const.insert(it);
    }
    for (auto it : remove_gep_const)
      BBgep_const.erase(it.first);
  }
}

void CommonSubExper::run()
{
  InitSideEffectFunc(m_);
  for (auto func : m_->get_functions())
  {
    op.clear();
    op_r_int.clear();
    op_l_int.clear();
    op_l_fp.clear();
    op_r_fp.clear();
    load.clear();
    gep.clear();
    gep_const.clear();
    for (auto bb : func->get_basic_blocks())
    {
      InitSets(bb);
      CommonSubExperDelete(bb);
    }
  }
}

void CommonSubExper::InitSideEffectFunc(Module *m)
{
  std::set<std::pair<CallInst *, Function *>> call_list;
  for (auto func : m->get_functions())
  {
    if (func->get_num_basic_blocks() == 0 || is_main(func))
    {
      if (func->get_num_of_args() != 0)
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

bool CommonSubExper::isLocalStore(StoreInst *store_ins)
{
  auto l_val = store_ins->get_lval();
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

bool CommonSubExper::isArgArrayPtr(Value *ptr)
{
  if (dynamic_cast<GlobalVariable *>(ptr))
    return false;
  else
  {
    auto instr = dynamic_cast<Instruction *>(ptr);
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

bool CommonSubExper::isGlobalArgArrayPtr(Value *val)
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

bool CommonSubExper::isEqualFirstPtr(Value *ptr1, Value *ptr2)
{
  if (ptr1 == ptr2)
    return true;
  else
  {
    auto instr1 = dynamic_cast<Instruction *>(ptr1);
    auto instr2 = dynamic_cast<Instruction *>(ptr2);
    if (!instr1 || !instr2)
      return false;
    if (!instr1->is_gep() || !instr2->is_gep())
      return false;
    else
      return instr1->get_operand(0) == instr2->get_operand(0);
  }
}