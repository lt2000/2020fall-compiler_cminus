# Lab5 实验报告

李涛 PB18030972

汤颖超 PB18030954

刘莉 PB18111681

## 实验要求

本次实验要求在熟练掌握课本知识的基础上，开发基本优化Pass。对于常量传播，实现在优化之后，将能够计算出结果的变量直接替换成常量；对于循环不变式外提，实现在优化之后，将与循环无关的表达式提取到循环的外面，需要考虑全局变量；对于活跃变量分析，能够分析出基本块入口和出口的活跃变量。

## 实验难点

* **常量传播**
  * **需要考虑全局变量在基本块内的常量传播**。**全局变量在Mem2Reg优化中，没有实现SSA的形式。所以全局变量不能在整个函数内进行常量传播**，故store指令不能删除。但是如果store的时常量可以在块内进行传播。
  * **常量传播后需要对无用的分支进行删除**。
    * 删除无用分支需要将条件转移指令更换为无条件跳转，直接跳转到有用分支，**同时需要删除无用分支后继块phi节点中无用块的分支，并且将无用分支从后继块的前驱链表中删去**
    * 条件嵌套中删除无用分支后会产生无法到达的块，需要对这些块进行删除，**通过判断基本是否有前驱判断是否为不可到达的块（label_entry除外）**
* **活跃变量分析**
  - 在计算OUT集合时，如果后继基本块有PHI结点，需要特殊考虑

## 实验设计

* 常量传播
    实现思路：

    * 局部变量的常量传播

      * 局部变量在Mem2Reg实现了SSA，可以在整个函数内传播，对每个函数中每个基本块中的add，sub，mul，div，fp2si，si2fp，zext，cmp，fcmp进行常量传播，先判断指令的操作数是否为常量，若为都为常量先进行常量折叠，然后向使用该变量的其他指令（`instr->get_use_list()`）传播。

        ```c++
         for (auto func : m_->get_functions())
            {
                for (auto bb : func->get_basic_blocks())
                {
                    global_def_int.clear(); //在每个bb内实现全局变量的常量传播
                    global_def_fp.clear();
                    std::vector<Instruction *> wait_delete;
                    for (auto instr : bb->get_instructions())
                    {
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
                                    for (auto use : instr->get_use_list()) //进行整型常量传播
                                    {
                                        dynamic_cast<User *>(use.val_)->set_operand(use.arg_no_, const_flod);
                                    }
                                    wait_delete.push_back(instr);
                                }
                            }
                            else if (cast_constantfp(instr->get_operand(0)) != nullptr && cast_constantfp(instr->get_operand(1)) != nullptr) //浮点型
                            {
                               //…………
                            }
                        }
                        else if (instr->is_fp2si()) //类型转换
                        {
                            //…………
                        }
                        else if (instr->is_si2fp()) //类型转换
                        {
                            //…………
                        }
                        else if (instr->is_zext()) //零扩展
                        {
                            //…………
                        }
                        else if (instr->is_cmp()) //比较指令
                        {
                            //…………
                        }
                        else if (instr->is_fcmp()) //比较指令
                        {
                            //…………
                        }
                    for (auto instr : wait_delete)
                    {
                        bb->delete_instr(instr);
                    }
                }
            }
         
        ```
      
    * 全局变量的常量传播

      * **全局变量在Mem2Reg优化中，没有实现SSA的形式。所以全局变量不能在整个函数内进行常量传播**，故store指令不能删除。但是如果store的时常量可以在块内进行传播

        * 如果全局变量`@a`store的是常量`c`，那么将`@a`和`c`存入 map`global_def_int`，否则应该将`global_def_int（fp）`中`a`的条目删去，因为现在store的已经不是常值，不能进行常量传播.
        * 在遇到load指令时，如果可以找到全局变量`@a`的 `global_def_int（fp）`中有常量条目则进行常量传播。

        ```c++
        for (auto func : m_->get_functions())
            {
                for (auto bb : func->get_basic_blocks())
                {
                    global_def_int.clear(); //在每个bb内实现全局变量的常量传播
                    global_def_fp.clear();
                    for (auto instr : bb->get_instructions())
                    {
                        //…………
                       else if (instr->is_load())
                        {
                            auto globalval = dynamic_cast<GlobalVariable *>(instr->get_operand(0));
                            if (globalval != nullptr)
                            {
                                if (globalval->get_operand(0)->get_type() == INT32_T)
                                {
                                    ConstantInt *val; //全局变量并不符合SSA规则,用一个map记录其定值
                                    if (global_def_int.find(globalval) != global_def_int.end())
                                    {
                                        val = global_def_int.find(globalval)->second;
                                    }
                                    else
                                        val = nullptr;
                                    if (val)
                                    {
                                        for (auto use : instr->get_use_list())
                                        {
                                            dynamic_cast<User *>(use.val_)->set_operand(use.arg_no_, val);
                                        }
                                        wait_delete.push_back(instr);
                                    }
                                }
                                else if (globalval->get_operand(0)->get_type() == FLOAT_T)
                                {
                                   //全局变量为浮点型…………
                                }
                            }
                        }
                        else if (instr->is_store())
                        {
                            if (instr->get_operand(0)->get_type() == INT32_T) // 整型
                            {
                                //全局变量为整型…………
                            }
                            else if (instr->get_operand(0)->get_type() == FLOAT_T) //浮点型
                            {
                                auto val1 = dynamic_cast<StoreInst *>(instr)->get_lval(); //store的地址
                                auto val2 = cast_constantfp(dynamic_cast<StoreInst *>(instr)->get_rval());
                                if (val2)
                                {
                                    auto globalval = dynamic_cast<GlobalVariable *>(val1);
                                    if (globalval != nullptr)
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
        
        ```

    * 无用分支的删除

      * 删除无用分支需要将条件转移指令更换为无条件跳转，直接跳转到有用分支，**同时需要删除无用分支后继块phi节点中无用块的分支，并且将无用分支从后继块的前驱链表中删去。**

        ```c++
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
                                        succ_bb->remove_pre_basic_block(bb); //从后继块的前驱链表中删去无用分支
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
                                     BranchInst::create_br(dynamic_cast<BasicBlock *>(falsebb), bb);//直接跳转到false
                                    bb->get_succ_basic_blocks().clear();
                                    bb->add_succ_basic_block(dynamic_cast<BasicBlock *>(falsebb));
                                }
                                else
                                {
                                   //flasebb为无用分支…………
                                }
                            }
                        }
                    }
                }
            }
        ```

    * 不可到达基本块的删除

      * 条件嵌套中删除无用分支后会产生无法到达的块，需要对这些块进行删除，**通过判断基本是否有前驱判断是否为不可到达的块（label_entry除外），同时删去这些块之后也要更新后继块的PHI节点**

        ```c++
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
                                                    instr->remove_operands(i - 1, i);
                                                }
                                            }
                                        }
                                    }
                                }
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
        ```

    * phi指令的删除

      * 删除无用分支后，其后继块的phi指令可能只有一个条目，将其删除进行传播可能会给常量传播带来新的机会，进行递归操作，当没有phi指令被删除时，返回。

        ```c++
         int flag = 0;
            for (auto func : m_->get_functions())
            {
                std::vector<Instruction *> wait_delete;
                for (auto bb : func->get_basic_blocks())
                {
                    for (auto instr : bb->get_instructions())
                    {
                        if (instr->is_phi())
                        {
                            if (instr->get_num_operand() == 2)
                            {
                                auto val = instr->get_operand(0);
                                instr->replace_all_use_with(val);
                                wait_delete.push_back(instr);
                                flag = 1;
                            }
                        }
                    }
                    for (auto instr : wait_delete) //删除无法到达的块
                    {
                        bb->delete_instr(instr);
                    }
                    wait_delete.clear();
                }
            }
            if(flag)//删除phi指令后，可能会给常量传播带来新的的机会，递归处理
            {
                run();
            }
            else return;
            return;
        ```
    
    *  在子函数中对数组和全局变量重新赋值的问题
    
      *  如果在子函数中对数组元素和全局变量重新赋值，就无法再进行其的常量传播，因为其值在子函数中已经改变，采用的是记录全局变量被定值函数的个数，大于1则不进行传播。
    
        ```c++
        int ConstPropagation::global_def_func() //得到全局变量被定值的函数个数
        {
            for (auto func : m_->get_functions())
            {
                for (auto bb : func->get_basic_blocks())
            {
                    for (auto instr : bb->get_instructions())
                {
                        if (instr->is_store())
                    {
                            auto val1 = dynamic_cast<StoreInst *>(instr)->get_lval();
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
        ```
    
        
    
    优化前后：
    
    * cminus代码
    
      ```c
      int a;
      void main(void){
          int b;
          int c;
          int i;
          float d;
          float e;
          b = 1;
          i = 1;
          c = b + 2.5;
          d = c / 2;
          e = d * 2;
          while(i<2)
          {
              if(1)
           {
              a = 2;
              a = a +1;
              a = i;
              a = a +1;
           }
           else
           {
               if(1)
               {
                   a = 3;
               }
               else
               {
                   a = 4;
               }
           }
           i = i + 1;
          }
          output(a);
      output(b);
          output(c);
      outputFloat(d);
          outputFloat(e);
      return ;
      }
      ```
    
      
    
    * 优化前
    
      ```c
      @a = global i32 zeroinitializer
      declare i32 @input()
      
      declare void @output(i32)
      
      declare void @outputFloat(float)
      
      declare void @neg_idx_except()
      
      define void @main() {
      label_entry:
        %op6 = sitofp i32 1 to float
        %op7 = fadd float %op6, 0x4004000000000000
        %op8 = fptosi float %op7 to i32
        %op10 = sdiv i32 %op8, 2
        %op11 = sitofp i32 %op10 to float
        %op13 = sitofp i32 2 to float
        %op14 = fmul float %op11, %op13
        br label %label15
      label15:                                                ; preds = %label_entry, %label36
        %op42 = phi i32 [ 1, %label_entry ], [ %op38, %label36 ]
        %op17 = icmp slt i32 %op42, 2
        %op18 = zext i1 %op17 to i32
        %op19 = icmp ne i32 %op18, 0
        br i1 %op19, label %label20, label %label22
      label20:                                                ; preds = %label15
        %op21 = icmp ne i32 1, 0
        br i1 %op21, label %label28, label %label34
      label22:                                                ; preds = %label15
        %op23 = load i32, i32* @a
        call void @output(i32 %op23)
        call void @output(i32 1)
        call void @output(i32 %op8)
        call void @outputFloat(float %op11)
        call void @outputFloat(float %op14)
        ret void
      label28:                                                ; preds = %label20
        store i32 2, i32* @a
        %op29 = load i32, i32* @a
        %op30 = add i32 %op29, 1
        store i32 %op30, i32* @a
        store i32 %op42, i32* @a
        %op32 = load i32, i32* @a
        %op33 = add i32 %op32, 1
        store i32 %op33, i32* @a
        br label %label36
      label34:                                                ; preds = %label20
        %op35 = icmp ne i32 1, 0
        br i1 %op35, label %label39, label %label40
      label36:                                                ; preds = %label28, %label41
        %op38 = add i32 %op42, 1
        br label %label15
      label39:                                                ; preds = %label34
        store i32 3, i32* @a
        br label %label41
      label40:                                                ; preds = %label34
        store i32 4, i32* @a
        br label %label41
      label41:                                                ; preds = %label39, %label40
        br label %label36
      }
      ```
    
      
    
    * 优化后（**分析见注释**）
    
      ```c
      @a = global i32 zeroinitializer
      declare i32 @input()
      
      declare void @output(i32)
      
      declare void @outputFloat(float)
      
      declare void @neg_idx_except()
      
      define void @main() {
      label_entry:
        br label %label15  //通过常量传播将运算和类型转换指令优化删去了
      label15:                                                ; preds = %label_entry, %label36
        %op42 = phi i32 [ 1, %label_entry ], [ %op38, %label36 ]
        %op17 = icmp slt i32 %op42, 2
        %op18 = zext i1 %op17 to i32
        %op19 = icmp ne i32 %op18, 0
        br i1 %op19, label %label20, label %label22
      label20:                                                ; preds = %label15
        br label %label28 //删去比较指令和无用分支，变为无条件跳转
      label22:                                                ; preds = %label15
        %op23 = load i32, i32* @a
        call void @output(i32 %op23)
        call void @output(i32 1)  //output的结果都是传播后的
        call void @output(i32 3)  
        call void @outputFloat(float 0x3ff0000000000000)
        call void @outputFloat(float 0x4000000000000000)
        ret void
      label28:                                                ; preds = %label20
        store i32 2, i32* @a
        store i32 3, i32* @a   //全局变量第一个store的是常值，进行了常量传播删去了load和add
        store i32 %op42, i32* @a
        %op32 = load i32, i32* @a //全局变量第二个store的不是常值，所以无法进行常量传播
        %op33 = add i32 %op32, 1
        store i32 %op33, i32* @a
        br label %label36
      label36:                                                ; preds = %label28
        %op38 = add i32 %op42, 1
        br label %label15
      //label34，label39，label40,label41是不可到达的基本块，被删除了
      }
      ```


* 循环不变式外提
    实现思路：在写代码之前要考虑以下两个问题：
    
    a. 思考如何判断语句与循环无关，且外提没有副作用
    
    ​	首先我们知道在不考虑数组与全局变量，又是在在mem2reg优化后的IR上进行不变式外提，所以load和store还有phi指令以及call指令都不能外提，其余只要看当前指令的参数是不是在循环中有被赋值过即可，若在循环中都没有赋值过，那就是循环不变式。
    
    b. 循环的条件块（就是在LoopSearch中找到的Base块）最多只有两个前驱，思考下，不变式应该外提到哪一个前驱。
    
    ​	loopbase有两个前驱块，一个在loop_set中，一个不在loop_set中，仅需要将不变式外提到不在loop_set中的loopbase的前驱块即可。
    
    ```c++
    for (auto preBB : loopBase->get_pre_basic_blocks())
      {
        if (loop->find(preBB) != loop->end())
          continue;
        Pre_out_loop.insert(preBB);
      }
      auto preBB = * Pre_out_loop.begin();
    ```
    
* 相应代码：
  
    * 各函数以及全局变量的说明
    
      ` std::set<std::pair<BasicBlock *, Instruction *> > invariant;`
    
      存储循环不变式
    
        `void findInvariants(BBset_t *loop);`找到循环不变式，并存在` invariant`中
    
        `void moveInvariantsOut(BBset_t *loop);` 将` invariant`中的不变式外提到上述问题b中提到的正确模块
    
      `void run() override;`找到所有的最内层循环，对这些最内层循环，依次由内至外，调用 `findInvariants`和`moveInvariantsOut`
    
    * 求内层循环
    
      ```c++
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
      ```
    
      由于loop_base可以唯一标志一个循环，所以un_base_loops和base_loops中存的都是loop_base，初始化base_loops是所有循环，只要将所有的loop的祖先存入un_base_loops中，在将un_base_loops中所有元素从base_loops中删去，base_loops中就仅存最内层循环了。
    
    * `void findInvariants(BBset_t *loop);`
    
      ```c++
      //set for those who have changed in loop
        std::unordered_set<Value *> needDefinedInLoop;
      ```
    
      `needDefinedInLoop`初始化为loop中所有不是void的instruction，这写都是循环中有被赋值的
    
      ```c++
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
      ```
    
      思路就是问题a中回答的思路，但是由于找到一个循环不变式时要将其从`needDefinedInLoop`中删去，所以`needDefinedInLoop`再找的过程中改变了，所以要再次遍历循环中的指令，保证没有因为`needDefinedInLoop`的改变产生新的循环不变式，所以`change`为false，既标志这次遍历没有找到新的不变式，也标志`needDefinedInLoop`没有改变
    
    *  `void moveInvariantsOut(BBset_t *loop);`
    
      由问题b，我们已经找到提循环不变式的目的模块`preBB`，`preBB`的最后一条指令必定为branch指令，并且是无条件跳转指令，先删去branch指令，依次按序添上不变式，在不变式的原block删去不变式，最后加回branch指令即可。
    
    * 优化前后的IR对比（举一个例子）并辅以简单说明：
    
      ```c
      int g;
      void print(int x)
      {
        output(x);
      }
      void main(void)
      {
        int i;
        int j;
        int A[20];
        int ret;
        int gret;
        int a;
        i = 1;
        j = 2;
        a = 5;
        g = 110;
        while(i < 100)
        {
          while(j < 200)
          {
            print(A[i]);
            j = j + 10;
            ret = a * a;
            gret = g - 10;
          }
          i = i + 1;
        }
        output(ret);
      }
      ```
    
      * 优化前
    
        ```c#
        ; ModuleID = 'cminus'
        source_filename = "/home/cp/lab5/2020fall-compiler_cminus/tests.cminus"
        
        @g = global i32 zeroinitializer
        declare i32 @input()
        
        declare void @output(i32)
        
        declare void @outputFloat(float)
        
        declare void @neg_idx_except()
        
        define void @print(i32 %arg0) {
        label_entry:
          call void @output(i32 %arg0)
          ret void
        }
        define void @main() {
        label_entry:
          %op2 = alloca [20 x i32]
          store i32 110, i32* @g
          br label %label6
        label6:                                                ; preds = %label_entry, %label23
          %op37 = phi i32 [ %op41, %label23 ], [ undef, %label_entry ]
          %op38 = phi i32 [ %op42, %label23 ], [ undef, %label_entry ]
          %op39 = phi i32 [ 2, %label_entry ], [ %op43, %label23 ]
          %op40 = phi i32 [ 1, %label_entry ], [ %op25, %label23 ]
          %op8 = icmp slt i32 %op40, 100
          %op9 = zext i1 %op8 to i32
          %op10 = icmp ne i32 %op9, 0
          br i1 %op10, label %label11, label %label12
        label11:                                                ; preds = %label6
          br label %label14
        label12:                                                ; preds = %label6
          call void @output(i32 %op38)
          ret void
        label14:                                                ; preds = %label11, %label27
          %op41 = phi i32 [ %op37, %label11 ], [ %op36, %label27 ]
          %op42 = phi i32 [ %op38, %label11 ], [ %op34, %label27 ]
          %op43 = phi i32 [ %op39, %label11 ], [ %op31, %label27 ]
          %op16 = icmp slt i32 %op43, 200
          %op17 = zext i1 %op16 to i32
          %op18 = icmp ne i32 %op17, 0
          br i1 %op18, label %label19, label %label23
        label19:                                                ; preds = %label14
          %op22 = icmp slt i32 %op40, 0
          br i1 %op22, label %label26, label %label27
        label23:                                                ; preds = %label14
          %op25 = add i32 %op40, 1
          br label %label6
        label26:                                                ; preds = %label19
          call void @neg_idx_except()
          ret void
        label27:                                                ; preds = %label19
          %op28 = getelementptr [20 x i32], [20 x i32]* %op2, i32 0, i32 %op40
          store i32 %op40, i32* %op28
          call void @print(i32 %op40)
          %op31 = add i32 %op43, 10
          %op34 = mul i32 5, 5
          %op35 = load i32, i32* @g
          %op36 = sub i32 %op35, 10
          br label %label14
        }
        
        ```
    
        
    
      * 优化后
    
        ```c
        ; ModuleID = 'cminus'
        source_filename = "/home/cp/lab5/2020fall-compiler_cminus/tests.cminus"
        
        @g = global i32 zeroinitializer
        declare i32 @input()
        
        declare void @output(i32)
        
        declare void @outputFloat(float)
        
        declare void @neg_idx_except()
        
        define void @print(i32 %arg0) {
        label_entry:
          call void @output(i32 %arg0)
          ret void
        }
        define void @main() {
        label_entry:
          %op2 = alloca [20 x i32]
          store i32 110, i32* @g
          %op34 = mul i32 5, 5	//与两层循环都无关的ret = a * a;提到了两层循环之外
          br label %label6
        //while(i < 100)的loop_base
        label6:                                               ; preds = %label_entry, %label23
          %op37 = phi i32 [ %op41, %label23 ], [ undef, %label_entry ]
          %op38 = phi i32 [ %op42, %label23 ], [ undef, %label_entry ]
          %op39 = phi i32 [ 2, %label_entry ], [ %op43, %label23 ]
          %op40 = phi i32 [ 1, %label_entry ], [ %op25, %label23 ]
          %op8 = icmp slt i32 %op40, 100
          %op9 = zext i1 %op8 to i32
          %op10 = icmp ne i32 %op9, 0 
          br i1 %op10, label %label11, label %label12
        label11:                                                ; preds = %label6
          %op22 = icmp slt i32 %op40, 0	//i>0?结果用于判断A[i]下边是否合法，从while(j < 200)循环中提出来了，但是%op22的br语句是不可能外提的
          %op28 = getelementptr [20 x i32], [20 x i32]* %op2, i32 0, i32 %op40	//求A[i]地址的gep指令，从内层循环提出来了
          br label %label14
        label12:                                                ; preds = %label6
          call void @output(i32 %op38)
          ret void
        label14:                                                ; preds = %label11, %label27
          %op41 = phi i32 [ %op37, %label11 ], [ %op36, %label27 ]
          %op42 = phi i32 [ %op38, %label11 ], [ %op34, %label27 ]
          %op43 = phi i32 [ %op39, %label11 ], [ %op31, %label27 ]
          %op16 = icmp slt i32 %op43, 200
          %op17 = zext i1 %op16 to i32
          %op18 = icmp ne i32 %op17, 0
          br i1 %op18, label %label19, label %label23
        label19:                                                ; preds = %label14
          br i1 %op22, label %label26, label %label27
        label23:                                                ; preds = %label14
          %op25 = add i32 %op40, 1
          br label %label6
        label26:                                                ; preds = %label19
          call void @neg_idx_except()
          ret void
        label27:                                                ; preds = %label19
          store i32 %op40, i32* %op28 //数组相关的store，不予考虑
          call void @print(i32 %op40) //尽管%op40是循环不变式，但是这句仍然不能外提，所以没有将print(i)提到内层循环之外
          %op31 = add i32 %op43, 10
          %op35 = load i32, i32* @g	//全局变量，不予考虑
          %op36 = sub i32 %op35, 10
          br label %label14
        }
        ```
        
    
* 活跃变量分析
  
    实现思路：
    
    - `void ActiveVars::initset(Function *func)`以函数为单位进行初始化。在初始化的过程中，`NeedDef`为函数变量集合，但不包括全局变量；`used`和`defed`暂存每个基本块中已引用和已定义的变量集合；`use_set[BB]`和`def_set[BB]`分别为每个基本块中引用之前无定值、定值之前无引用的变量集合。
    
      - 将函数变量放入`NeedDef`集合中，不包括全局变量
    
        ```c++
        for (auto it : func->get_args())
                NeedDef.insert(it);
            for (auto BB : func->get_basic_blocks())
            {
                for (auto it : BB->get_instructions())
                {
                    if (!it->is_void())
                    {
                        NeedDef.insert(it);
                    }
                }
            }
        ```
    
      - 对于每个基本块，求相应的`used`和`defed`集合。
    
        获取基本块中每个指令的操作数，放在`vec`集合中。
    
        如果该指令用于分配寄存器，则已定值，放入`defed`。
    
        否则，对于该指令的每个操作数，如果指令不是`load`类型且操作数不在`NeedDef`集合中，那么该操作数被引用，放入`used`；对于全局数组，gep指令的vec[0]虽然不在`NeedDef`中，但属于已引用的变量，放入`used`；如果store指令的左值不是数组类型，必定是全局变量，被引用，放入`used`中；对于其他不在`NeedDef`集合中的`instr`，被定值，放入`defed`。
    
        最后，如果`defed`中的变量没有出现在`used`中，则定值之前无引用，放入`def_set[BB]`中；如果`used`中的变量没有出现在`defed`中，则引用之前无定值，放入`use_set[BB]`中。
    
        对于PHI结点，将其偶数位参数放入`phi_op[BB]`，对应BB为PHI结点所在的基本块；将其`vec[i]`与`vec[i+1]`作为一个`pair`，放入 `phi_op_pair[BB]`中，关联参数与参数所在的基本块。
    
      相应代码：
    
      ```c++
      void ActiveVars::initset(Function *func)
      {
          std::set<Value *> used, defed;
          //func的变量集合，但是不包括使用到的全局变量的名称
          std::set<Value *> NeedDef;
      
          for (auto it : func->get_args())
              NeedDef.insert(it);
          for (auto BB : func->get_basic_blocks())
          {
              for (auto it : BB->get_instructions())
              {
                  if (!it->is_void())
                  {
                      NeedDef.insert(it);
                  }
              }
          }
          for (auto BB : func->get_basic_blocks())
          {
              used.clear();
              defed.clear();
              for (auto instr : BB->get_instructions())
              {
                  auto vec = instr->get_operands();
                  if (instr->is_alloca())
                  {
                      defed.insert(instr);
                      def_set[BB].insert(instr);
                  }
                  else
                  {
                      for (unsigned i = 0; i < instr->get_num_operand(); i++)
                      {
                          if (NeedDef.find(vec[i]) == NeedDef.end() && !instr->is_load())
                              continue;
                          used.insert(vec[i]);
                          if (defed.find(vec[i]) == defed.end())
                          {
                              use_set[BB].insert(vec[i]);
                              live_in[BB].insert(vec[i]);
                          }
                      }
                      //如果是全局数组，那gep指令的vec[0]在NeedDef中找不到，但它确是gep的参数
                      if (instr->is_gep())
                      {
                          used.insert(vec[0]);
                          if (defed.find(vec[0]) == defed.end())
                          {
                              use_set[BB].insert(vec[0]);
                              live_in[BB].insert(vec[0]);
                          }
                      }
                      if (instr->is_store())
                      {
                          auto l_val = static_cast<StoreInst *>(instr)->get_lval();
                          //不是数组，那必定是全局变量了,但由于定义不在函数中，所以在NeedDef中找不到，要单独处理
                          if (!(static_cast<Instruction *>(l_val)->is_gep()))
                          {
                              used.insert(l_val);
                              use_set[BB].insert(l_val);
                              live_in[BB].insert(l_val);
                          }
                      }
                      if (NeedDef.find(instr) == NeedDef.end())
                          continue;
                      defed.insert(instr);
                      if (used.find(instr) == used.end())
                          def_set[BB].insert(instr);
                  }
                  if (instr->is_phi())
                  {
                      for (unsigned i = 0; i < instr->get_num_operand(); i += 2)
                      {
                          if (NeedDef.find(vec[i]) != NeedDef.end())
                          {
                              phi_op[BB].insert(vec[i]);
                              phi_op_pair[BB].insert(std::make_pair(dynamic_cast<BasicBlock *>(vec[i + 1]), vec[i]));
                          }
                      }
                  }
              }
          }
      }
      ```
    
    - `void ActiveVars::run()`
    
      用布尔变量rescan控制迭代次数。
    
      对于每一个基本块，按照算法计算`live_in[BB]`和`live_out[BB]`。求某基本块的OUT集合时，如果其后继块中有PHI结点，只选取出现在该基本块的变量。
    
      迭代结束之后，分别删去live_in[BB]为空或live_out[BB]为空的基本块。
    
      相应代码：
    
      ```c++
      void ActiveVars::run()
      {
          std::ofstream output_active_vars;
          output_active_vars.open("active_vars.json", std::ios::out);
          output_active_vars << "[";
          for (auto &func : this->m_->get_functions())
          {
              if (func->get_basic_blocks().empty())
              {
                  continue;
              }
              else
              {
                  func_ = func;
                  bool rescan = true;
                  initset(func_);
                  // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
                  std::set<Value *> curIn;
                  while (rescan)
                  {
                      rescan = false;
                      for (auto BB : func_->get_basic_blocks())
                      {
                          curIn.clear();
                          for (auto it : live_in[BB])
                              curIn.insert(it);
                          live_out[BB].clear();
                          live_in[BB].clear();
                          for (auto succBB : BB->get_succ_basic_blocks())
                          {
                              if (live_in.find(succBB) != live_in.end())
                              {
                                  for (auto inItem : live_in[succBB])
                                  {
                                      if (live_out[BB].find(inItem) == live_out[BB].end())
                                      {
                                          if (phi_op[succBB].find(inItem) != phi_op[succBB].end() &&
                                              phi_op_pair[succBB].find(std::make_pair(BB, inItem)) == phi_op_pair[succBB].end())
                                              continue;
                                          else
                                          {
                                              live_out[BB].insert(inItem);
                                              live_in[BB].insert(inItem);
                                          }
                                      }
                                  }
                              }
                          }
                          for (auto it : use_set[BB])
                          {
                              if (live_in[BB].find(it) == live_in[BB].end())
                                  live_in[BB].insert(it);
                          }
                          for (auto it : def_set[BB])
                          {
                              if (live_in[BB].find(it) != live_in[BB].end())
                                  live_in[BB].erase(it);
                          }
                          if (live_in[BB].size() != curIn.size())
                          {
                              rescan = true;
                              continue;
                          }
                          for (auto inItem : curIn)
                          {
                              if (live_in[BB].find(inItem) == live_in[BB].end())
                              {
                                  rescan = true;
                                  continue;
                              }
                          }
                      }
                  }
      
                  for (auto BB : func_->get_basic_blocks())
                  {
                      if (live_in[BB].size() == 0)
                          live_in.erase(BB);
                      if (live_out[BB].size() == 0)
                          live_out.erase(BB);
                  }
      
                  output_active_vars << print();
                  output_active_vars << ",";
      
                  live_in.clear();
                  live_out.clear();
                  use_set.clear();
                  def_set.clear();
              }
          }
          output_active_vars << "]";
          output_active_vars.close();
          return;
      }
      ```

### 实验总结

通过本次实验，我们加深了对课本知识的理解，用代码实现了相关算法，并体会到代码所做的优化；更加深入了解C++语法，对map，set，pair等数据结构更加熟练；我们学会了与同组队员进行及时交流，相互学习。总之，通过本次实验，小组中每个成员的能力均有提升。

### 实验反馈 （可选 不会评分）

实验文档详细，可读性强。

### 组间交流 （可选）

无。
