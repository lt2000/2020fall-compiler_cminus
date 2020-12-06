# lab4 实验报告

李涛 PB18030972

汤颖超 PB18030954

刘莉 PB18111681

## 实验要求

**在助教提供的自动已完成Flex,Bison生成的语法树到C++语法树的转换的框架上，写出自动根据cminusf语法树生成LLVM IR文件的cpp代码。**

具体实现：通过参考cminusf_builder.hpp文件以及include中的文件，填充cminusf_builder.cpp文件中的16个与cminus语法中的终结符/非终结符相关联的函数，实现在语法树上该结点所要执行的语义动作。

## 实验难点

1. **ASTFunDeclaration函数的设计**

   ​      在将函数名push到符号表的时候应该先退出当前函数的作用域，进入到全局作用域，因为函数名应该是全局都可以索引到的。push完成后再次进入函数的作用域（**同时要注意此步骤应该放在push函数参数之前，否则随着退出当前作用域，push到符号表中的参数会消失**）

2. **ASTSelectionStmt函数的设计**

   在该函数的设计中遇到了许多问题：

   * **`if-else`不配对出现。**即只有`if`的情况，最初考虑不论`else`是否出现，都生成flaseBB这个block（尽管这可能会产生冗余），这条冗余无法解决某种特殊情况，后将冗余去掉，实验设计中详述。
   * **`if-else`的嵌套问题。**考虑到`else`总是和最靠近的`if`配对，为了保证嵌套中每对`if-else`生成的nextBB是按序生成，所以先处理`else`中的动作。
   * **`if-else`中含有return。**如果`if-else`都含有`return`语句，那么就不应该生成nextBB，如果`if-else`只有一个含有`return`语句，则应该生成nextBB；如果只有`if`且含有`return`语句，也应该生成nextBB。

3. **ASTVar函数的设计**

   ​         我们知道`visit(ASTVar &node)`的返回值为指向一个具体元素的指针，语义要求可以对一些数值进行强制类型转换，但是这个操作显然也不能太过非法，例如函数传参时，若形参是int，但是由传入参数为一个数组，这时要将它和一个普通变量区分开来，特设argload全局变量，1标识对于由`visit(ASTVar &node)`得到的指针能不能在传参中进行load操作。（**因为它是从一个id为pointer或array，且无expression的var得到的**）

4. 算不上是难点吧，可以看作是优化，就是源程序中有多余代码和缺少return语句的翻译处理，这个详见组间交流部分

## 实验设计

#### 一、全局变量设计

* **Value *ret**
  * 用于存储变量、数组以及数字所在的地址，方便在对一个变量或常量操作时进行取用。
* **std::vector<Type *> Ints**
  * ASTFunDeclaration与ASTParam之间的共享变量。因为创建函数时需要函数返回值的类型和所有参数的类型，所以进入ASTParam函数应该将参数的类型传递出来。
* **int argload**
  * 是共享在ASTVar和ASTCall之间的全局变量，标识由ASTVar得到的指针能不能在call之前load，如上所说，如果ASTVar的返回值来自没有expression的数组或指针变量，在call调用的时候不允许load
* **int return_flag**
  * 用于标识当前模块中是否已经有完整的return语句

#### 二、分析每个结点执行动作

##### 1. ASTProgram：

* 遍历program下层的declaration-list中的声明

##### 2. ASTNum ：

* 判断node的类型（整型或浮点型），将将相应的value值存入，并将地址赋给ret

##### 3. ASTVarDeclaration：

* 首先通过``scope.in_global``判断声明的变量是全局变量还是局部变量

* 如果是全局变量，先根据node.num判断是否为数组（或变量），再根据类型分配空间并赋初值，并将id push到作用域中。

  * 全局整型数组模块如下，其余类似
  
  * ```c
     if (node.num != nullptr && node.num->i_val) //全局数组
            {
                if (node.type == TYPE_INT) //整型数组
                {
                    auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
                  auto initializer = ConstantZero::get(arrayType, module.get());
                    auto Globle_IntArrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer); //为数组分配空间
                    scope.push(node.id, Globle_IntArrayAlloca);
                }
     }
    ```
  
    
  
* 如果是局部变量，同样先根据node.num判断是否为数组（或变量），再根据类型分配空间，并将id push到作用域中。

  * 局部浮点型变量模块如下，其余类似
  
  * ```c
    else //局部变量
            {
                else if (node.type == TYPE_FLOAT) //浮点型变量
                {
                    auto Local_FloatAlloca = builder->create_alloca(TyFloat); //为变量分配空间
                    scope.push(node.id, Local_FloatAlloca);
                }
            }
    ```

##### 4. ASTFunDeclaration：

* 参数列表不为空时

  * 通过node.type判断新声明的function的返回值类型

  ```c
  if (node.type == TYPE_FLOAT)
          funType = TyFloat;
      else
      {
          funType = (node.type == TYPE_VOID) ? TYPEV : TYPE32;
      }
  ```

  * 再遍历Fundeclaration下的param节点获得参数类型，然后根据参数类型和返回值类型创建     函数的BasicBlock，然后将函数的名字push到作用域中，**此时应该先退出当前作用域，     进入到全局的作用域，之后再次进入函数的作用域，且此动作应该再push其他内容前进行，否     则会将其其他内容丢弃**

  ```c
  auto fun = Function::create(FunctionType::get(funType, Ints), node.id, module.get());
          // BB的名字在生成中无所谓,但是可以方便阅读
          auto bb = BasicBlock::create(module.get(), "entry", fun);
          builder->set_insert_point(bb);
          scope.exit();
          scope.push(node.id, fun); //函数名放进作用域
          scope.enter();
  ```

  * 然后根据参数的类型分配空间，并将函数名push到符号表中（**参数分配空间不能在param节点中执行，因为那时候还没有创建函数**，没有执行`builder->set_insert_point(bb)`）。

* 参数列表为空时，只需第二步
  
* 最后进入ASTCompoundStmt。

##### 5. ASTParam ：

* 对于每一个传进来的参数node，将其类型存入到全局变量Ints中。

##### 6. ASTCompoundStmt ：

* 因为可能有局部变量，所以先进入到新的作用域

* 遍历所有local_declarations和statement-list

##### 7. ASTExpressionStmt ：

* 对expression_stmt的expression结点调用accept

##### 8. ASTSelectionStmt ：

* 创建if（**true**）分支

* 根据是否有else分支进行选择处理

  **有else分支**

  * 创建false分支及条件转移

  * 进入到false分支的BB，然后进入到else中的`statement`

  * 如果else中没有`return`，将标记`insertedflag`置为1，然后创建nextBB和无条件转移。

  * 进入到true分支的BB，然后进入到if中的`statement`

  * 如果if中没有`return`，且else中有`return`（通过`insertedflag==0`判断），将标记`insertedflag`置为1，然后创建nextBB**（因为if中没有return时，nextBB一定存在，故无论else中是否有`return`都要无条件转移到nextBB**）。

    ```c++
     if (builder->get_insert_block()->get_terminator() == nullptr)
            { // no return inside the block
                if (insertedflag == 0)
                {
                    insertedflag = 1;
                    nextBB = BasicBlock::create(module.get(), "", currentFunc);
                }
                builder->create_br(nextBB);
            }
    ```

  * 最后当`insertedflag`为一时，即创建了nextBB，进入到此BasicBlock中。

  **没有else分支**

  * 没有else分支时，一定会有nextBB，因此创建nextBB，并创建trueBB和nextBB的条件转移指令
  * 进入到trueBB，然后进入到if中的`statement`
  * 无条件转移到nextBB，最后进入到nextBB

* 上述结构可以解决if-else中都有return的情况，即

  ```c++
  if(……)
     return；
  else
     return；
  ```

* 也可以解决else缺省，只有if中有return的情况，即

  ```
  if(……)
     return；
  return；
  ```

##### 9. ASTIterationStmt：

* 基本和`if-else`一样，但是三个模块`loopJudge` ，`loopBody`，`out`都是必须的

##### 10. ASTReturnStmt ：

* 这个函数要求强制类型转换，所以要先得到`return_type`，对比`ret`的`type`，就是一番常规的类型转化，`（pointer，i 1，i32，float）-> (void, i 32, float)`

##### 11. ASTVar：

* 我们从语法中得知例``a = b``这样的表达式，等式左值和右值都是var，所以限制了``visit(ASTVar &node)``的返回值为指向一个具体元素的指针，var→ID ∣ ID [ expression],这里最重要的是对每一种情况都进行分析，不遗漏

* 对数组下标，有强制类型转换，有小于0的异常处理（需要生成下标正常和非正常的BB）

* 
    | id类型  | expression | argload | 正常出现的地方                   |
    | ------- | ---------- | ------- | -------------------------------- |
    | array   | 有         | 1       | 基本所有地方读取数组元素         |
    | pointer | 有         | 1       | 子函数使用传入的数组             |
    | 其他    | 有         | 1       | 不应该出现                       |
    | array   | 无         | 0       | 调用子函数时，将数组作为传入参数 |
    | pointer | 无         | 0       | 将传入的数组作为参数传入子函数   |
    | int     | 无         | 1       | 基本所有地方                     |
    | float   | 无         | 1       | 基本所有地方                     |


##### 12.ASTAssignExpression：

   * 等号左边是var，我们会得到一个指针，自然，要求强制类型转换，要将等号右边的返回值，符合var指向的那个元素的type，又是一个（pointer，i 1，i32，float）-> (void, i 32, float)的转换
   * 在这里我们，可以想到i 1只在比较时生成，那么是否可以在SimpleExpression中直接将i 1转化为i 32，那样在其他函数中就不用考虑i 1到 i 32的转换，这样固然可以，但是就是我们在if-else或while中要用到时，就必须将这个i 32和int 的 0进行比较了，就多了一些冗余代码

##### 13. ASTSimpleExpression：

- 如果没有运算符，则通过accept调用下一层

- 如果有运算符，先获取左值再获取右值。如果是指针，load上来；否则，直接赋值给相应的表达式

  ```c++
  node.additive_expression_l->accept(*this);
          if(ret->get_type()->is_pointer_type())
              AdditiveLoad_l = builder->create_load(ret);
          else
              AdditiveLoad_l = ret;
  ```

- 判断是不是浮点数，如果两个数中有且仅有一个是浮点数，那么将另外一个数转换成浮点数。并且在检测到浮点数时，将标志flag设为1，便于以后选择是否浮点数比较。同时，如果是布尔型表达式，则转换成32位整数。

  ```c++
  if(AdditiveLoad_l->get_type()->is_float_type()){
              flag = 1;
              if(AdditiveLoad_r->get_type()->is_integer_type())
                  AdditiveLoad_r = builder->create_sitofp(AdditiveLoad_r, FloatType);     
           }
  ```

  ```c++
  if(AdditiveLoad_l->get_type() == Int1Type)
                      AdditiveLoad_l = builder->create_zext(AdditiveLoad_l,Int32Type);
  ```

- 根据op的值选择相应的比较，将结果赋值给icmp，最后将icmp赋值给返回值ret

##### 14. ASTAdditiveExpression：

- 如果没有additive_expression，则通过accept调用下一层；如果有，则先获取左值，再获取右值。判断是否为浮点数，如果是，将flag置1，根据flag和op选择计算，并返回结果。


##### 15. ASTTerm ：

* 如果没有term，通过accept调用下一层；如果有term，先获取左值，再获取右值。判断是否为浮点数，如果是，将flag置1，根据flag和op选择计算，并返回结果

##### 16. ASTCall ：

- 调用scope.find()找ID对应的值，如果没有找到，直接返回。

- 检查函数类型，如果不匹配，直接返回。

  ```C++
  if(!fun->is_function_type())
          return;
  ```

- 检查参数类型是否匹配。设置计数器i，统计参数的数量。每接受一个参数，判断参数类型，如果不匹配，则进行强制类型转换，将i++。最后比较i和要求参数数量，如果不相等，则退出。最后生成调用语句。

  ```c++
  for(auto Args : node.args){
          auto arg_type = callfun->get_param_type(i);
          i++;
      ……
  if(arg_type == Int32Type)
          {
              if(argload && ret->get_type()->is_pointer_type())
                  ret = builder->create_load(ret);
              if(ret->get_type()->is_pointer_type())
                  return ;
              else if(ret ->get_type() == FloatType)
                  ret = builder->create_fptosi(ret,Int32Type);
              value_args = ret;
          }   
      ……
  function.push_back(value_args);
  if(i != callfun->get_num_of_args())
      ……
  ret = builder->create_call(value, function);
  ```

## cminus文件测试（冒泡排序）

```c
void exchange(int x[],int i)
{
    int temp;
    temp = x[i];
    x[i] = x[i+1];
    x[i+1]=temp;
}

void sort(int y[],int size)
{
    int j;
    int k;
    j = 0;
    while(j < size)
    {
        k = size - 1;
        while(k>j)
        {
            if(y[k] < y[k-1])
                exchange(y,k-1);
            k = k - 1;
        }
        j = j + 1;
    }
}
void main(void) {
    int array[20];
    int num;
    int n;
    n = 0;
    num = input();
    while(n < num)
    {
        array[n] = input();
        n = n + 1;
    }
    sort(array,num);
    n = 0;
    while(n < num)
    {
        output(array[n]);
        n = n + 1;
    }
    return;
}

```

**生成的.ll文件如下**

```c
; ModuleID = 'cminus'
source_filename = "/home/cp/2020fall-Compiler_CMinus/tests/lab4/testcases/li.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @exchange(i32* %0, i32 %1) {
label.entry:
  %2 = alloca i32*
  %3 = alloca i32
  store i32* %0, i32** %2
  store i32 %1, i32* %3
  %4 = alloca i32
  %5 = load i32, i32* %3
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %8, label %7
7:
  call void @neg_idx_except()
  br label %11
8:
  %9 = load i32*, i32** %2
  %10 = getelementptr i32, i32* %9, i32 %5
  br label %11
11:
  %12 = load i32, i32* %10
  store i32 %12, i32* %4
  %13 = load i32, i32* %3
  %14 = icmp sge i32 %13, 0
  br i1 %14, label %16, label %15
15:
  call void @neg_idx_except()
  br label %19
16:
  %17 = load i32*, i32** %2
  %18 = getelementptr i32, i32* %17, i32 %13
  br label %19
19:
  %20 = load i32, i32* %3
  %21 = add i32 %20, 1
  %22 = icmp sge i32 %21, 0
  br i1 %22, label %24, label %23
23:
  call void @neg_idx_except()
  br label %27
24:
  %25 = load i32*, i32** %2
  %26 = getelementptr i32, i32* %25, i32 %21
  br label %27
27:
  %28 = load i32, i32* %26
  store i32 %28, i32* %18
  %29 = load i32, i32* %3
  %30 = add i32 %29, 1
  %31 = icmp sge i32 %30, 0
  br i1 %31, label %33, label %32
32:
  call void @neg_idx_except()
  br label %36
33:
  %34 = load i32*, i32** %2
  %35 = getelementptr i32, i32* %34, i32 %30
  br label %36
36:
  %37 = load i32, i32* %4
  store i32 %37, i32* %35
}
define void @sort(i32* %0, i32 %1) {
label.entry:
  %2 = alloca i32*
  %3 = alloca i32
  store i32* %0, i32** %2
  store i32 %1, i32* %3
  %4 = alloca i32
  %5 = alloca i32
  store i32 0, i32* %4
  br label %6
6:
  %7 = load i32, i32* %4
  %8 = load i32, i32* %3
  %9 = icmp slt i32 %7, %8
  br i1 %9, label %10, label %13
10:
  %11 = load i32, i32* %3
  %12 = sub i32 %11, 1
  store i32 %12, i32* %5
  br label %14
13:
14:
  %15 = load i32, i32* %5
  %16 = load i32, i32* %4
  %17 = icmp sgt i32 %15, %16
  br i1 %17, label %18, label %21
18:
  %19 = load i32, i32* %5
  %20 = icmp sge i32 %19, 0
  br i1 %20, label %25, label %24
21:
  %22 = load i32, i32* %4
  %23 = add i32 %22, 1
  store i32 %23, i32* %4
  br label %6
24:
  call void @neg_idx_except()
  br label %28
25:
  %26 = load i32*, i32** %2
  %27 = getelementptr i32, i32* %26, i32 %19
  br label %28
28:
  %29 = load i32, i32* %27
  %30 = load i32, i32* %5
  %31 = sub i32 %30, 1
  %32 = icmp sge i32 %31, 0
  br i1 %32, label %34, label %33
33:
  call void @neg_idx_except()
  br label %37
34:
  %35 = load i32*, i32** %2
  %36 = getelementptr i32, i32* %35, i32 %31
  br label %37
37:
  %38 = load i32, i32* %36
  %39 = icmp slt i32 %29, %38
  br i1 %39, label %40, label %44
40:
  %41 = load i32*, i32** %2
  %42 = load i32, i32* %5
  %43 = sub i32 %42, 1
  call void @exchange(i32* %41, i32 %43)
44:
  %45 = load i32, i32* %5
  %46 = sub i32 %45, 1
  store i32 %46, i32* %5
  br label %14
}
define void @main() {
label.entry:
  %0 = alloca [20 x i32]
  %1 = alloca i32
  %2 = alloca i32
  store i32 0, i32* %2
  %3 = call i32 @input()
  store i32 %3, i32* %1
  br label %4
4:
  %5 = load i32, i32* %2
  %6 = load i32, i32* %1
  %7 = icmp slt i32 %5, %6
  br i1 %7, label %8, label %11
8:
  %9 = load i32, i32* %2
  %10 = icmp sge i32 %9, 0
  br i1 %10, label %15, label %14
11:
  %12 = getelementptr [20 x i32], [20 x i32]* %0, i32 0, i32 0
  %13 = load i32, i32* %1
  call void @sort(i32* %12, i32 %13)
  store i32 0, i32* %2
  br label %21
14:
  call void @neg_idx_except()
  br label %17
15:
  %16 = getelementptr [20 x i32], [20 x i32]* %0, i32 0, i32 %9
  br label %17
17:
  %18 = call i32 @input()
  store i32 %18, i32* %16
  %19 = load i32, i32* %2
  %20 = add i32 %19, 1
  store i32 %20, i32* %2
  br label %4
21:
  %22 = load i32, i32* %2
  %23 = load i32, i32* %1
  %24 = icmp slt i32 %22, %23
  br i1 %24, label %25, label %28
25:
  %26 = load i32, i32* %2
  %27 = icmp sge i32 %26, 0
  br i1 %27, label %30, label %29
28:
  ret void
29:
  call void @neg_idx_except()
  br label %32
30:
  %31 = getelementptr [20 x i32], [20 x i32]* %0, i32 0, i32 %26
  br label %32
32:
  %33 = load i32, i32* %31
  call void @output(i32 %33)
  %34 = load i32, i32* %2
  %35 = add i32 %34, 1
  store i32 %35, i32* %2
  br label %21
}
```

**结果验证**

```bash
$ /home/cp/2020fall-Compiler_CMinus/tests/lab4/testcases/li
5
6
5
12
1
10
1
5
6
10
12
```

如上所示，第一个数字表示排序的个数，该二进制文件将6、5、12、1、10按增序排列。

## 实验总结

​       通过本次实验，首先，我们从单个结点入手，更加熟悉和理解语法树的生成方式；其次，我们熟悉了C++语法，学会通过类来查找函数的属性，更加深刻地体验了VScode代码自动补全功能的强大；再者，我们明白了scope域和全局变量的相关使用；最后，我们也学会了如何与同组队员进行及时讨论交流，相互学习。整个过程中，我们不断考虑极端样例进行测试，力求代码更加智能，能够尽量容忍程序的出错，而不是遇到错误就退出编译。总之，通过这次实验，我们的能力均有提升，也更加明白编译原理这门课的重要性！

## 实验反馈

​       文档很好。

## 组间交流

* 和PB18000227小组的吕瑞讨论了源程序中有冗余代码和缺少return语句的处理

​        例如

```c
void main(void){
  int i;
  i = 0;
    if(i)
    {
        return ;
        i = 7;
    } 
    while(i >= 7)
    {
        i = 0;
    }
    output(i);
}
```

生成的ll文件的主体内容如下：

```livescript
define void @main() {
label.entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = load i32, i32* %0
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %3, label %4
3:
  ret void
4:
  br label %5
5:
  %6 = load i32, i32* %0
  %7 = icmp sge i32 %6, 7
  br i1 %7, label %8, label %9
8:
  store i32 0, i32* %0
  br label %5
9:
  %10 = load i32, i32* %0
  call void @output(i32 %10)
  ret void
}

```



i = 7是一条多余的代码，我们可以在翻译的时候不予翻译（有中生无），对于output（i）后面已经没有代码了，但对于整个函数要补全一条return代码（无中生有）

自然地，解决这个问题需要知道当前模块已经翻译的代码中是不是已经有完整的return，我们用一个全局变量return_flag来表示

主要逻辑如下

1. 有中生无，cpp文件中几乎对于每一个visit函数（除了前几个），都要在函数开头加上 对return_flag的判断，如果是1，则抛弃子节点，不翻译

2. 无中生有，在``` visit(ASTFunDeclaration &node)```的最后，整个函数都翻译完毕的时候，如果return_flag为0，根据return_type补上类型正确的return语句，最后将return_flag置为0

3. 维护这个return_flag，除了``` visit(ASTFunDeclaration &node)```和``` visit(ASTReturnStmt &node)```对return_flag有操作以外，要注意if-else,while结构中的return_flag，while模块翻译完毕之后，return—flag置为0，因为return_flag是标识当前模块的，相应地，`` if-else模块结束后，只有if和else都有return时，才使return_flag为1 ``