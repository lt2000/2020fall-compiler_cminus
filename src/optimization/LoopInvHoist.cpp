#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

//#define DEBUG
void LoopInvHoist::run()
{
  // 先通过LoopSearch获取循环的相关信息
  loop_searcher = new LoopSearch(m_,false);
  loop_searcher->run();
  // save all base_loop's loop base
  std::unordered_set<BasicBlock *> base_loops;
  std::unordered_set<BasicBlock *> un_base_loops;
  for (auto it = loop_searcher->begin(); it != loop_searcher->end(); it++)
      base_loops.insert(loop_searcher->get_loop_base(*it));
  for (auto it : base_loops)
  {
    auto parent = loop_searcher->get_parent_loop(loop_searcher->get_inner_loop(it));
    while (parent)
    {
      un_base_loops.insert(loop_searcher->get_loop_base(parent));
      parent = loop_searcher->get_parent_loop(parent);
    }
  }
  for (auto it : un_base_loops)
  {
    if(base_loops.find(it) != base_loops.end() )
      base_loops.erase(it);
  }
  for (auto it: base_loops)
  {
    auto loop = loop_searcher->get_inner_loop(it);
    do
    {
#ifdef DEBUG
      std::cout << "inloop:\t";
      for (auto BB : *loop)
      {
        std::cout << BB->get_name() + "\t";
      }
      putchar('\n');
#endif
      invariant.clear();
      findInvariants(loop);
      if (invariant.size())
        moveInvariantsOut(loop);
      //由内至外，找到外层循环
      loop = loop_searcher->get_parent_loop(loop);
    } while (loop);
  }
}

void LoopInvHoist::findInvariants(BBset_t *loop)
{
  //set for those who have changed in loop
  std::unordered_set<Value *> needDefinedInLoop;
  for (auto BB : *loop)
  {
    for (auto ins : BB->get_instructions())
    {
      if(! ins->is_void())
      needDefinedInLoop.insert(ins);
    }
  }

  bool change = false;
  do
  {
    change = false;
    for (auto BB : *loop)
    {
      for (auto ins : BB->get_instructions())
      {
        bool allInvariant = true;
        if (ins->is_phi() || ins->is_void() || ins->is_call() ||
            ins->is_alloca() || ins->is_load() ||
            needDefinedInLoop.find(ins) == needDefinedInLoop.end())
          //已经是循环不变式
          continue;
        for (auto val : ins->get_operands())
        {
          //表达式参数在循环中有被赋值
          if (needDefinedInLoop.find(val) != needDefinedInLoop.end())
          {
            allInvariant = false;
            break;
          }
        }
        //当前ins是循环不变式
        if (allInvariant)
        {
          needDefinedInLoop.erase(ins);
          invariant.insert(std::make_pair(BB, ins));
          change = true;
        }
      }
    }
  } while (change);
#ifdef DEBUG
  std::cout << "find Loop invariant\n";
  for (auto pair : invariant)
  {
    std::cout << "in BB: " + pair.first->get_name() + "\t" +
                     " Val: " + (pair.second)->get_name() + "\n";
  }
#endif
}

void LoopInvHoist::moveInvariantsOut(BBset_t *loop)
{

  auto loopBase = loop_searcher->get_loop_base(loop);
  std::unordered_set<BasicBlock *> Pre_out_loop;
  
  for (auto preBB : loopBase->get_pre_basic_blocks())
  {
    if (loop->find(preBB) != loop->end())
      continue;
    Pre_out_loop.insert(preBB);
  }
  auto preBB = * Pre_out_loop.begin();
  auto br = preBB->get_terminator();
  if (br->is_br())
  {
    //delete br
    preBB->delete_instr(br);
    for (auto pair : invariant)
    {
      pair.first->delete_instr(pair.second);
      preBB->add_instruction(pair.second);
    }
    // add back br
    preBB->add_instruction(br);
  }
}
