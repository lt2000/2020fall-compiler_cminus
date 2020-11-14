# lab3 实验报告
学号 PB18030972  姓名 李涛

## 问题1: cpp与.ll的对应
请描述你的cpp代码片段和.ll的每个BasicBlock的对应关系。描述中请附上两者代码。

**cpp代码片段和.ll的每个BasicBlock的对应关系通过对应的注释给出**

### assign

*  **.cpp**

```c++
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);
  
  auto retAlloca = builder->create_alloca(Int32Type);
  auto *arrayType = ArrayType::get(Int32Type, 10);            //定义数组类型
  auto  a = builder->create_alloca(arrayType);                //为数组分配空间
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0

  auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});  // GEP: 指向a[0]
  builder->create_store(CONST_INT(10), a0GEP);                        //a[0]=10
  a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});       // GEP: 指向a[0]
  auto a0load = builder->create_load(a0GEP);                          //a[0] load出来
  auto mul = builder->create_imul(a0load, CONST_INT(2));              //a[0]*2

  auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});  // GEP: 指向a[1]
  builder->create_store(mul, a1GEP);                                  //将结果store进a[1]
  a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});
  auto a1load = builder->create_load(a1GEP);                          //将结果从a[1]中load出来
  builder->create_ret(a1load);
```

* **.ll**

```c
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4             ;栈底返回值
  %2 = alloca  [10 x i32], align 4     ;给数组分配空间
  store i32 0, i32* %1, align 4
  ;为什么不能用这种形式,由于SSA？？
  ;store i32 90, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  %3 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 0 ; GEP: 指向a[0]
  store i32 10, i32* %3, align 4                                       ;a[0]=10
  %4 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 0 ;GEP: 指向a[0]
  %5 = load i32,  i32* %4, align 4                                     ;a[0] load出来
  %6 = mul i32 %5, 2                                                   ;a[0]*2
  %7 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 1 ; GEP: 指向a[1]
  store i32 %6, i32* %7, align 4                                       ;将结果store进a[1]
  %8 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 1
  %9 = load i32, i32* %8, align 4                                      ;将结果从a[1]中load出来
  ret i32 %9
}
```

### fun

*  **.cpp**

```c++
// callee函数
  // 函数参数类型的vector
  std::vector<Type *> Ints(1, Int32Type);

  //通过返回值类型与参数类型列表得到函数类型
  auto calleeFunTy = FunctionType::get(Int32Type, Ints);

  // 由函数类型得到函数
  auto calleeFun = Function::create(calleeFunTy,
                                 "callee", module);

  auto bb = BasicBlock::create(module, "entry", calleeFun);
  builder->set_insert_point(bb);                        // 一个BB的开始,将当前插入指令点的位置设在bb
  
  auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
  auto aAlloca = builder->create_alloca(Int32Type);     // 在内存中分配参数a的位置
  
  std::vector<Value *> args;  // 获取callee函数的形参,通过Function中的iterator
  for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
    args.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
  }

  builder->create_store(args[0], aAlloca);             // 将参数a store下来
  auto aLoad = builder->create_load(aAlloca);           // 将参数a load上来
  auto mul = builder->create_imul(aLoad, CONST_INT(2)); //mul=2*a
  builder->create_ret(mul);


  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);

  retAlloca = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(0), retAlloca);                 // 默认 ret 0

  auto call = builder->create_call(calleeFun, {CONST_INT(110)});  //调用callee       
  builder->create_ret(call);
```

* **.ll**

```c
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @callee(i32 %0) #0 {
  %2 = alloca i32, align 4       ;子函数为什么从%0开始，主函数从%1开始？？？？
  store i32 %0, i32* %2, align 4 ;子函数里面为什么没有%1？？？
  %3 = load i32, i32* %2, align 4
  %4 = mul i32 %3, 2  
  ret i32 %4         
}
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4  ;默认 ret 0
  %2 = call i32 @callee(i32 110) ;调用callee
  ret i32 %2
}
  
```
### if

*  **.cpp**

```c++
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);

  auto retAlloca = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0

  auto a = builder->create_alloca(floatType);                 //给a分配空间
  builder->create_store(CONST_FP(5.555), a);                  //给a赋值
  auto aload = builder->create_load(a);
  auto fcmp = builder->create_fcmp_lt(aload, CONST_INT(1));    //条件判断
  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
  auto retBB = BasicBlock::create(module, "retBB",mainFun);  
  builder->create_cond_br(fcmp, trueBB, falseBB);

  builder->set_insert_point(trueBB);                            //a>1
  builder->create_store(CONST_INT(233), retAlloca);
  builder->create_br(retBB);                                    //转到返回

  builder->set_insert_point(falseBB);                           //a<=1
  builder->create_store(CONST_INT(0), retAlloca);
  builder->create_br(retBB);                                     //转到返回

  builder->set_insert_point(retBB);                             // ret分支
  auto retLoad = builder->create_load(retAlloca);
  builder->create_ret(retLoad);
```

* **.ll**

```c
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca float, align 4                          ;%2是指针
  store i32 0, i32* %1, align 4
  store float 0x40163851E0000000, float* %2, align 4  ;5.555怎么存？？？ IEEE 754 double
  %3 = load float, float* %2
  %4 = fcmp ogt float %3, 1.000000                    ;条件判断
  br i1 %4, label %5, label %6

5:                                              
  store i32 233, i32* %1, align 4                     ;a>1
  br label %7                                         ;转到返回
             
6:                                                
  store i32 0, i32* %1, align 4                       ;a<=1
  br label %7                                         ;转到返回

7:                                               
  %8 = load i32, i32* %1, align 4                     ;ret分支
  ret i32 %8
}
```
### while

*  **.cpp**

```c++
auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);

  auto retAlloca = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(0), retAlloca);             // 默认 ret 0
  auto a = builder->create_alloca(Int32Type);                 //给a,i分配内存
  auto i = builder->create_alloca(Int32Type);
  builder->create_store(CONST_INT(10), a);                    //给a,i赋值
  builder->create_store(CONST_INT(0), i); 
  auto whilecond = BasicBlock::create(module, "whilecond", mainFun); 
  auto whilebody = BasicBlock::create(module, "whilebody", mainFun); 
  auto whileend = BasicBlock::create(module, "whileend", mainFun);   
  builder->create_br(whilecond);                               //转到循环条件分支

  builder->set_insert_point(whilecond);                       //循环条件分支
  auto iteration = builder->create_load(a);
  auto icmp = builder->create_icmp_lt(a,CONST_INT(10));       //循环条件判断
  builder->create_cond_br(icmp,whilebody,whileend);           

  builder->set_insert_point(whilebody);                       //循环体分支
  iteration = builder->create_load(i);
  auto add = builder->create_iadd(iteration,CONST_INT(1));
  builder->create_store(add, i);                              //i++
  iteration = builder->create_load(i);
  auto a_iteration = builder->create_load(a);  
  add = builder->create_iadd(a_iteration,iteration);
  builder->create_store(add, a);                              //a=a+i
  builder->create_br(whilecond);

  builder->set_insert_point(whileend);                       //循环结束分支
  a_iteration = builder->create_load(a);
  builder->create_ret(a_iteration);
```

* **.ll**

```c
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* %2, align 4
  store i32 0, i32* %3, align 4
  br label %4                    ;转到循环条件分支

4: ;循环判断条件
  %5 = load i32, i32* %3, align 4
  %6 = icmp slt i32 %5, 5         ;循环条件判断
  br i1 %6, label %7, label %13

7: ;循环体
  %8 = load i32, i32* %3, align 4
  %9 = add  i32 %8, 1             
  store i32 %9, i32* %3, align 4    ;i++
  %10 = load i32, i32* %3, align 4
  %11 = load i32, i32* %2, align 4
  %12 = add  i32 %10, %11      
  store i32 %12, i32* %2, align 4   ; a=a+i
  br label %4
13: ;序号一定要按照顺序，循环结束
  %14 = load i32, i32* %2, align 4
  ret i32 %14
}
```
## 问题2: Visitor Pattern
请指出visitor.cpp中，`treeVisitor.visit(exprRoot)`执行时，以下几个Node的遍历序列:numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。  
序列请按如下格式指明：  
exprRoot->numberF->exprE->exprD->numberA->exprD->numberB->exprD->exprE->exprC->numberB->exprC->numberA->exprC->exprE->exprRoot

## 问题3: getelementptr
请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 

  - `%2 = getelementptr i32, i32* %1 i32 %0` 

  - 区别：

    * 第一种用法的第一个索引的值与第一个ty[10 x i32]（索引的基本类型）共同决定偏移量，返回的指针类型为[10 x i32]*

      第二个索引将索引的基本类型变为i32，且返回的指针类型变为i32*

    * 第二种用法索引的值与第一个ty i32（索引的基本类型）共同决定偏移量，返回的指针类型为i32*

## 实验难点
* 手动实现.ll文件遇到的问题
  * 创建的局部数组不能使用如下形式直接取出数组中的元素：`store i32 90, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4`
    * 解决：`TA`从全局变量与局部变量的区别来看，全局变量是从全局数据段分配内存，是确定的位置，所以是可以用内联的方式写出来，而局部变量在堆栈上分配空间，可能起始地址存在寄存器里，故需要一个寄存器来存储取偏移后的结果，用内联的方式可能达不到这样的效果
  * 子函数中的寄存器号从0开始，主函数从1开始，且子函数中没有寄存器号1
    * 与返回值有关
  * 5.555怎么存
    * 解决：`IEEE 754` 先通过5.555得到float值的hex表示(40B1C28F),再将这个float(40B1C28F)扩展成double的表示(40163851E0000000)
* 实现cpp文件遇到的问题
  * 对比写的.ll文件和助教给的示例可以轻松写出cpp文件

## 实验反馈
文档很好。
