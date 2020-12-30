# Lab5 实验报告

李涛 PB18030972

## 实验要求

请按照自己的理解，写明本次实验需要干什么

## 实验难点

* **常量传播**
  * **需要考虑全局变量在基本块内的常量传播**。**全局变量在Mem2Reg优化中，没有实现SSA的形式。所以全局变量不能在整个函数内进行常量传播**，故store指令不能删除。但是如果store的时常量可以在块内进行传播。
  * **常量传播后需要对无用的分支进行删除**。
    * 删除无用分支需要将条件转移指令更换为无条件跳转，直接跳转到有用分支，**同时需要删除无用分支后继块phi节点中无用块的分支，并且将无用分支从后继块的前驱链表中删去**
    * 条件嵌套中删除无用分支后会产生无法到达的块，需要对这些块进行删除，**通过判断基本是否有前驱判断是否为不可到达的块（label_entry除外）**

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
    实现思路：
    相应代码：
    优化前后的IR对比（举一个例子）并辅以简单说明：
    
* 活跃变量分析
    实现思路：
    相应的代码：

### 实验总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
