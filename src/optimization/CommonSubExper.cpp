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

void CommonSubExper::run()
{
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
      std::vector<Instruction *> wait_delete;
      for (auto instr : bb->get_instructions())
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
              if (op_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval}) != op_l_int.end())
              {
                auto iter = op_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval});
                instr->replace_all_use_with(iter->second);
                wait_delete.push_back(instr);
              }
              else
              {
                op_l_int.insert({{lval_const->get_value(), instr->get_instr_type(), rval}, instr});
              }
            }
            else if (rval_const)
            {
              if (op_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()}) != op_r_int.end())
              {
                auto iter = op_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()});
                instr->replace_all_use_with(iter->second);
                wait_delete.push_back(instr);
              }
              else
              {
                op_r_int.insert({{lval, instr->get_instr_type(), rval_const->get_value()}, instr});
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
              if (op_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval}) != op_l_int.end())
              {
                auto iter = op_l_int.find({lval_const->get_value(), instr->get_instr_type(), rval});
                instr->replace_all_use_with(iter->second);
                wait_delete.push_back(instr);
              }
              else
              {
                op_l_int.insert({{lval_const->get_value(), instr->get_instr_type(), rval}, instr});
              }
            }
            else if (rval_const)
            {
              if (op_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()}) != op_r_int.end())
              {
                auto iter = op_r_int.find({lval, instr->get_instr_type(), rval_const->get_value()});
                instr->replace_all_use_with(iter->second);
                wait_delete.push_back(instr);
              }
              else
              {
                op_r_int.insert({{lval, instr->get_instr_type(), rval_const->get_value()}, instr});
              }
            }
          }
          else
          {
            if (op.find({lval, instr->get_instr_type(), rval}) != op.end())
            {
              auto iter = op.find({lval, instr->get_instr_type(), rval});
              instr->replace_all_use_with(iter->second);
              wait_delete.push_back(instr);
            }
            else
            {
              op.insert({{lval, instr->get_instr_type(), rval}, instr});
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
            if (gep_const.find({base, const_offset->get_value()}) != gep_const.end())
            {
#ifdef DEBUG
              std::cerr << "HAHAHAHAHHAHA" << std::endl;
              std::cerr << const_offset->get_value() << std::endl;
#endif
              auto iter = gep_const.find({base, const_offset->get_value()});
              instr->replace_all_use_with(iter->second);
              wait_delete.push_back(instr);
            }
            else
            {
              gep_const.insert({{base, const_offset->get_value()}, instr});
            }
          } 
          else//a=b[i]
          {
            if (gep.find({base, offset}) != gep.end())
            {
              auto iter = gep.find({base, offset});
              instr->replace_all_use_with(iter->second);
              wait_delete.push_back(instr);
            }
            else
            {
              gep.insert({{base, offset}, instr});
            }
          }
        }
        else if (instr->is_load())
        {
          auto base = instr->get_operand(0);
          if (load.find(base) != load.end())
          {
            auto iter = load.find(base);
            instr->replace_all_use_with(iter->second);
            wait_delete.push_back(instr);
          }
          else
          {
            load.insert({base, instr});
          }
        }
        else if (instr->is_store()) //对数组元素或全局变量重新定值后，load中的值就失效了
        {
          std::vector<std::pair< Value *, Value *>> wait_delete_load;
          auto base = dynamic_cast<StoreInst *>(instr)->get_lval();
          for (auto iter : load)
          {
            if (iter.first == base)
            {
              wait_delete_load.push_back(iter);
            }
          }
          for (auto iter : wait_delete_load)
          {
            load.erase(load.find(iter.first));
          }
        }
      }
      for (auto instr : wait_delete)
      {
        bb->delete_instr(instr);
      }
    }
  }
}