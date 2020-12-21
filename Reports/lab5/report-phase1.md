# Lab5 实验报告-阶段一

汤颖超 PB18030954

刘莉  PB18111681 

李涛 PB18030972

## 实验要求

阅读`Mem2Reg`与`LoopSearch`两个优化Pass的代码，能够描述优化的基本流程，并回答思考题

## 思考题
### LoopSearch
1. 循环的入口如何确定？循环的入口的数量可能超过1嘛？

   * 对强连通分量的每个节点循环查找每个节点的前驱，如果该节点有一个前驱不在该连通分量中，则该节点就是循环入口。

     ```c++
     for (auto n : *set)
         {
             for (auto prev : n->prevs)
             {
                 if (set->find(prev) == set->end())
                 {
                     base = n;
                 }
            
             }
         }
     ```

   * 循环入口的数量不会超过1，因为在没有goto语句的前提下，循环入口就是循环的条件块。

2. 简述一下算法怎么解决循环嵌套的情况。

   * find_loop_base()每次查找的都是最外层的循环入口,因为内部循环的前驱一定在强连通分量中。然后切断循环入口与其他基本块的连接，重新搜索强连通分量，在查找的循环就是内部的循环。

   * **特殊情况**：当内部循环的入口节点是外部循环入口节点的后继，外部循环的入口连接删除时，会导致内部循环找不到入口。因此用reserved保存找到的base，当找不到入口时，通过reserved的后继找到当前循环的base。

     ```c++
       for (auto res : reserved)
         {
             for (auto succ : res->succs)
             {
                 if (set->find(succ) != set->end())
                 {
                     base = succ;
                 }
             }
         }
     ```

     

### Mem2reg
1. 请简述支配边界的概念。

   * 节点n的支配边界DF(n)是指在离开n的每条CFG路径上，从节点n可达但不可支配的第一个节点。

2. 请简述`phi`节点的概念，与其存在的意义。

   * phi函数是伪赋值函数，phi节点可以辅助实现替换栈变量的功能，它可以合并来自不同路径的值。在顺序执行结构中，phi节点是非必要的，因为最近更新是唯一的，但是对于有多个前驱块的程序块，每一个名字的最近更新不是唯一的，这时就需要phi函数帮助我们根据程序执行路径选择合适前驱块上的更新为被引用变量值，当引用变量时，我们就可以直接通过phi函数得到最近更新的变量值。。
   
3. Mem2Reg将alloca 指令分配的栈变量转化成 SSA 寄存器，并且在合适的地方插入 PHI 结点。执行后，遇到PHI结点时，将函数结果放在var_val_stack[l_val]栈中；遇到store指令时，将右值放在var_val_stack[l_val]栈中；遇到load指令时，从var_val_stack[l_val]栈中取出变量的值，并用该值替换后续所有该load指令的结果，从而删除了alloca、store、load指令。

   例如：

   * 遇到`store i32 %arg0, i32* %op2`指令时，将`%arg0`存储到var_val_stack[%op2]栈中；遇到`%op13 = load i32, i32* %op2`指令时，从var_val_stack[%op2]栈中取出`%arg0`，并用`%arg0`替换后续所有`%op13`。

   * 在label14中，指令`%op15 = load i32, i32* %op0`中的`%op0`既可能由label_entry中的`store i32 %op3, i32* %op0`定义，又可能由label10中的`store i32 %op12, i32* %op0`定义，所以在原`%op15`处插入PHI结点`%op21 = phi i32 [ %op3, %label_entry ], [ %op4, %label10 ]`，将`%op21`放入var_val_stack[%op0]栈中。重新遍历遇到该PHI结点时，从var_val_stack[%op0]栈中取出值。

before `Mem2Reg`：

```c
; ModuleID = 'cminus'
source_filename = "complex3.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @gcd(i32 %arg0, i32 %arg1) {
label_entry:
  %op2 = alloca i32
  store i32 %arg0, i32* %op2
  %op3 = alloca i32
  store i32 %arg1, i32* %op3
  %op4 = load i32, i32* %op3
  %op5 = icmp eq i32 %op4, 0
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label10
label8:                                                ; preds = %label_entry
  %op9 = load i32, i32* %op2
  ret i32 %op9
label10:                                                ; preds = %label_entry
  %op11 = load i32, i32* %op3
  %op12 = load i32, i32* %op2
  %op13 = load i32, i32* %op2
  %op14 = load i32, i32* %op3
  %op15 = sdiv i32 %op13, %op14
  %op16 = load i32, i32* %op3
  %op17 = mul i32 %op15, %op16
  %op18 = sub i32 %op12, %op17
  %op19 = call i32 @gcd(i32 %op11, i32 %op18)
  ret i32 %op19
}
define void @main() {
label_entry:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = alloca i32
  %op3 = call i32 @input()
  store i32 %op3, i32* %op0
  %op4 = call i32 @input()
  store i32 %op4, i32* %op1
  %op5 = load i32, i32* %op0
  %op6 = load i32, i32* %op1
  %op7 = icmp slt i32 %op5, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label14
label10:                                                ; preds = %label_entry
  %op11 = load i32, i32* %op0
  store i32 %op11, i32* %op2
  %op12 = load i32, i32* %op1
  store i32 %op12, i32* %op0
  %op13 = load i32, i32* %op2
  store i32 %op13, i32* %op1
  br label %label14
label14:                                                ; preds = %label_entry, %label10
  %op15 = load i32, i32* %op0
  %op16 = load i32, i32* %op1
  %op17 = call i32 @gcd(i32 %op15, i32 %op16)
  store i32 %op17, i32* %op2
  %op18 = load i32, i32* %op2
  call void @output(i32 %op18)
  ret void
}
```

After `Mem2Reg`：

```c
; ModuleID = 'cminus'
source_filename = "complex3.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @gcd(i32 %arg0, i32 %arg1) {
label_entry:
  %op5 = icmp eq i32 %arg1, 0
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label10
label8:                                                ; preds = %label_entry
  ret i32 %arg0
label10:                                                ; preds = %label_entry
  %op15 = sdiv i32 %arg0, %arg1
  %op17 = mul i32 %op15, %arg1
  %op18 = sub i32 %arg0, %op17
  %op19 = call i32 @gcd(i32 %arg1, i32 %op18)
  ret i32 %op19
}
define void @main() {
label_entry:
  %op3 = call i32 @input()
  %op4 = call i32 @input()
  %op7 = icmp slt i32 %op3, %op4
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label14
label10:                                                ; preds = %label_entry
  br label %label14
label14:                                                ; preds = %label_entry, %label10
  %op19 = phi i32 [ %op3, %label10 ], [ undef, %label_entry ]
  %op20 = phi i32 [ %op4, %label_entry ], [ %op3, %label10 ]
  %op21 = phi i32 [ %op3, %label_entry ], [ %op4, %label10 ]
  %op17 = call i32 @gcd(i32 %op21, i32 %op20)
  call void @output(i32 %op17)
  ret void
}
```

4. 在放置phi节点的时候，算法是如何利用支配树的信息的？

   * 对于`src/optimization/Mem2Reg.cpp`，我们可以知道用的偏向于最小静态单赋值形式，在任何汇合点处要加入一个phi函数，对应同一原始名字的两个不同定义的会合。因为对于当前模块的store语句，它会影响自己支配边界的变量值，所以支配边界中需要加入phi函数，并且store右值会成为phi函数的参数（在`re_name`中体现）

   *  支配边界的计算需要依靠支配树，因为只有汇合点（前驱节点≥2）才可能为支配边界，所以依`src/optimization/Dominators.cpp`，对每个汇合点n考究其`每一个前驱`，若是前驱非IDOM（n），则n为该前驱的支配边界，反之沿支配树上溯（` runner = get_idom(runner);`），找到以n为支配边界的节点，并将n加入该节点的支配边界中。

5. 算法是如何选择`value`(变量最新的值)来替换`load`指令的？（描述数据结构与维护方法）

   * 数据结构：算法对一个变量使用一个栈依次保存变量每次的定值，以保证栈顶是变量的最新值。
   * 维护方法：
     * 首先将基本块开头的phi指令对变量的新命名压入栈中作为最新值。
     * 然后遍历剩余指令，如果是store指令，将右值压入栈中作为最新值；如果是load指令，从栈顶取出最新值并用该值替换后续所有该load指令得到的结果。


### 代码阅读总结

#### LoopSearch

* 建立CFG

* 从外向内寻找强连通分量
  
  * 因为tarjan算法是根据生成的IR深度优先搜索强连通分量，所以是从外向内寻找强连通分量
  
* 对于每个强连通分量找到每个循环的入口节点

  * find_loop_base()每次查找的都是最外层的循环入口,因为内部循环的前驱一定在强连通分量中。然后切断循环入口与其他基本块的连接，重新搜索强连通分量，在查找的循环就是内部的循环。

    ```c++
    for (auto n : *set)
        {
            for (auto prev : n->prevs)
            {
                if (set->find(prev) == set->end())
                {
                    base = n;
                }
            }
        }
    ```

    

  * **特殊情况**：当内部循环的入口节点是外部循环入口节点的后继，外部循环的入口连接删除时，会导致内部循环找不到入口。因此用reserved保存找到的base，当找不到入口时，通过reserved的后继找到当前循环的base。

    ```c++
      for (auto res : reserved)
        {
            for (auto succ : res->succs)
            {
                if (set->find(succ) != set->end())
                {
                    base = succ;
                }
            }
        }
    ```

    

* 储存结果

  * 将连通分量插入到循环的集合中
  * 记录函数与循环的对应关系
  * 记录base与循环的对应关系

* 将强连通分量每个节点映射到循环入口。

* 为搜索内部循环，删除循环入口节点与其他节点的连接，然后转到第二步重新搜索强连通分量。

####  Dominators

* `src/optimization/Dominators.cpp`文件主要实现了求支配树和支配边界（对支配边界的求值见上问题4）。
*  支配树依靠IDOM，因为m=IDOM(n)等价于支配树上有一条边从m指向n，所以IDOM的求值是关键。由IDOM的定义知，IDOM(n)应该是n所有前驱的共同祖先，为了减少求值路径所以令`new_idom = intersect(p, new_idom);`，new_idom的初值是n一个前驱的IDOM，为此要自上而下求IDOM。
*  对IDOM的求值顺序是从后序遍历的反序，是自上而下的，这样就可以充分利用前驱的idom，但是一旦有一个节点的IDOM改变了就要进行迭代` while (changed)`，以保证后继节点（依赖此节点的IDOM）的正确性。2中的`new_idom = intersect(p, new_idom);` 有一个判断条件`if (get_idom(p))`，这个主要在第一次迭代时起作用，因为`(get_idom(p))`为`nullptr`说明，p的求值顺序在n之后，即p可以由n的一个后继到达。事实上由于cminus生成的IR的逻辑很简单，几乎所有IR仅有第一次的迭代会改变IDOM，所以基本只需要迭代2次。
*  该文件中还有`create_doms`函数，该函数思路基本和`create_idom`一致，只是赋值改成doms合并。

#### Mem2Reg

* `dynamic_cast<……>(……)`用于多态类型的转换，运行时如果发现是不正确的指针类型会返回`NULL`。

* `Mem2Reg::run()`
  
* 生成支配树，从支配者获取信息。对于每一个有基本块的函数，生成PHI函数并重命名。对所有函数移除`alloca`指令。
  
* `Mem2Reg::generate_phi()`

  * 找到所有的活跃变量及其所在的基本块

    对于每个基本块中的`store`指令，在其左值既不是全局变量也不是数组元素的情况下，判断右值是否被杀死。如果未杀死，将左值放在活跃变量的栈中；如果杀死，将左值放在被杀死变量的栈中。将该基本块放在`live_var_2blocks[l_val]`中。
  * 插入PHI函数结点

  ​       `work_list`从`live_var_2blocks[l_val]`栈中获取所有要插入PHI函数的基本块。对于`work_list`中的每一个基本块，在汇      合点处插入PHI函数，将汇合点放入`work_list`栈中，并将标志`bb_has_var_phi`置为`true`

*  `Mem2Reg::re_name(BasicBlock *bb)`

      * 对于基本块中所有PHI结点，获取左值l_val，并将函数值放在`var_val_stack[l_val]`栈中
      * 对于基本块中所有`load`指令，在左值既不是全局变量也不是数组元素的情况下，用栈顶值替换load指令及后续所有用到该值的地方，并将该指令放在`wait_delete`栈中
      * 对于基本块中所有`store`指令，在左值既不是全局变量也不是数组元素的情况下，右值入栈，并将该指令放在`wait_delete`栈中
      * 对于每一个基本块的后继块中的所有PHI指令，如果左值存在且栈`var_val_stack[l_val]`非空，则将栈顶元素及其所在的基本块作为PHI函数的参数；否则，将参数置为`[ undef, bb ]`

       * 对支配树中基本块的每一个后继重命名。
       * 对于基本块中的每个`store`指令和PHI函数，将栈顶元素弹出，即为最近修改的值
       * 清空所有待删除`store`和`load`指令

* `Mem2Reg::remove_alloca()`

     * 删除所有整型或者浮点型的`alloca`指令。

### 实验反馈 （可选 不会评分）

建议Mem2Reg中说明第一步没有实现剪枝，稍微有些误导性。其他很好。

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息