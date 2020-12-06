## 贡献详述

### 成员 李涛

1. 完成了 `void CminusfBuilder::visit(ASTProgram &node)` 
    `void CminusfBuilder::visit(ASTNum &node)` 
    `void CminusfBuilder::visit(ASTVarDeclaration &node)` 
    `void CminusfBuilder::visit(ASTFunDeclaration &node)`
   `void CminusfBuilder::visit(ASTParam &node)` 
   `void CminusfBuilder::visit(ASTCompoundStmt &node)`函数的编写

2. 解决了与`if-else`相关的问题（if-else嵌套、if-else中含有return、if-else不完全配对）

3. 解决了函数调用参数无法传递的问题

### 成员 汤颖超

1. 完成了`void CminusfBuilder::visit(ASTExpressionStmt &node)` 
   `void CminusfBuilder::visit(ASTSelectionStmt &node)` 
   `voidCminusfBuilder::visit(ASTIterationStmt &node)` 
   `void CminusfBuilder::visit(ASTReturnStmt &node)` 
   `void CminusfBuilder::visit(ASTVar &node)` 
   `void CminusfBuilder::visit(ASTAssignExpression &node)`函数的编写
2. 解决了变量赋值的问题、实现了布尔表达式的翻译、解决了while后没有nextBB的问题等等。
3. 汤颖超同学发现了许多不易察觉的bug，大大提高了程序的健壮性。

### 成员 刘莉

1. 完成了
   `void CminusfBuilder::visit(ASTSimpleExpression &node)` 
   `void CminusfBuilder::visit (ASTAdditiveExpression &node)` 
   `void CminusfBuilder::visit(ASTTerm &node)` 
   `void CminusfBuilder::visit(ASTCall &node)`函数的编写

2. 解决了四则运算的正确性

3. 解决了call函数生成出错


## 评定结果

|  名字  | 百分比 |
| :----: | :----: |
|  李涛  | 32.5%  |
| 汤颖超 |  35%   |
|  刘莉  | 32.5%  |
