#include "cminusf_builder.hpp"

#define CONST_INT(num) \
    ConstantInt::get((int)num, module.get())

#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get()) // 得到常数值的表示,方便后面多次用到

//global variables
Value *ret; //存储返回值



/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */


void CminusfBuilder::visit(ASTProgram &node) {
    //program -> declaration-list
    for(auto decl : node.declarations )
    {
        decl->accept(*this);
    }
    //TO DO 如何保证最后一个声明是main()
 }

void CminusfBuilder::visit(ASTNum &node) { 
   if(node.type==TYPE_INT)
   {
       ret = CONST_INT(node.i_val);
   }
   else if(node.type==TYPE_FLOAT)
   {
       ret = CONST_FP(node.f_val);
   }
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    //var-declaration -> type-specifier ID ; ∣ type-specifier ID [INTEGER ]
    auto TyInt32 = Type::get_int32_type(module.get());
    auto TyFloat = Type::get_float_type(module.get());
    //局部变量,不能是空类型，push作用将id储存进作用域，以便之后赋值时查找是否声明
    if(!scope.in_global())
    {
        if(node.num->i_val)//局部数组
        {
            if(node.type==TYPE_INT)//整型数组
            {
               auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
               auto Local_IntArrayAlloca = builder->create_alloca(arrayType);  //为数组分配空间
               scope.push(node.id,Local_IntArrayAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型数组
            {
               auto *arrayType = ArrayType::get(TyFloat, node.num->i_val);
               auto Local_FloatArrayAlloca = builder->create_alloca(arrayType);  //为数组分配空间
               scope.push(node.id,Local_FloatArrayAlloca);
            }
            
        }
        else //局部变量
        {
             if(node.type==TYPE_INT)//整型变量
            {
               auto Local_IntAlloca = builder->create_alloca(TyInt32); //为变量分配空间
               scope.push(node.id,Local_IntAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型变量
            {
               auto Local_FloatAlloca = builder->create_alloca(TyFloat); //为变量分配空间
               scope.push(node.id,Local_FloatAlloca);
            }
        }
    }
    else//全局变量
    {
       if(node.num->i_val)//全局数组
        {
            if(node.type==TYPE_INT)//整型数组
            {
               auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
               auto initializer = ConstantZero::get(arrayType, module.get());
               auto Globle_IntArrayAlloca = builder->create_alloca(arrayType);  //为数组分配空间
               scope.push(node.id,Globle_IntArrayAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型数组
            {
               auto *arrayType = ArrayType::get(TyFloat, node.num->i_val);
               auto initializer = ConstantZero::get(arrayType, module.get());//初始值赋为零
               auto Globle_FloatArrayAlloca = builder->create_alloca(arrayType);  //为数组分配空间
               scope.push(node.id,Globle_FloatArrayAlloca);
            }
            
        }
        else //全局变量
        {
             if(node.type==TYPE_INT)//整型变量
            {
               auto initializer = ConstantZero::get(TyInt32, module.get());
               auto Globle_IntAlloca = builder->create_alloca(TyInt32); //为变量分配空间
               scope.push(node.id,Globle_IntAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型变量
            {
               auto initializer = ConstantZero::get(TyFloat, module.get());
               auto Globle_FloatAlloca = builder->create_alloca(TyFloat); //为变量分配空间
               scope.push(node.id,Globle_FloatAlloca);
            }
        }
    }
    
 }

void CminusfBuilder::visit(ASTFunDeclaration &node) { 
    node.params.size();
}

void CminusfBuilder::visit(ASTParam &node) { }

void CminusfBuilder::visit(ASTCompoundStmt &node) { }

void CminusfBuilder::visit(ASTExpressionStmt &node) { }

void CminusfBuilder::visit(ASTSelectionStmt &node) { }

void CminusfBuilder::visit(ASTIterationStmt &node) { }

void CminusfBuilder::visit(ASTReturnStmt &node) { }

void CminusfBuilder::visit(ASTVar &node) { }

void CminusfBuilder::visit(ASTAssignExpression &node) { }

void CminusfBuilder::visit(ASTSimpleExpression &node) { }

void CminusfBuilder::visit(ASTAdditiveExpression &node) { }

void CminusfBuilder::visit(ASTTerm &node) { }

void CminusfBuilder::visit(ASTCall &node) { }
