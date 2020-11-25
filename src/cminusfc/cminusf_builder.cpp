#include "cminusf_builder.hpp"

#define CONST_INT(num) \
    ConstantInt::get((int)num, module.get())

#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get()) // 得到常数值的表示,方便后面多次用到

//global variables
Value *ret; //存储返回值
std::vector<Type *> Ints;//储存参数的类型，以确定函数的类型


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
        if(node.num!=nullptr&&node.num->i_val)//局部数组
        {
            if(node.type==TYPE_INT)//整型数组
            {
               auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
               auto Local_IntArrayAlloca = builder->create_alloca(arrayType);  //为数组分配空间
               scope.push(node.id,Local_IntArrayAlloca);//一定要push吗？？
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
       if(node.num!=nullptr&&node.num->i_val)//全局数组
        {
            if(node.type==TYPE_INT)//整型数组
            {
               auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
               auto initializer = ConstantZero::get(arrayType, module.get());
               auto Globle_IntArrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);  //为数组分配空间
               scope.push(node.id,Globle_IntArrayAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型数组
            {
               auto *arrayType = ArrayType::get(TyFloat, node.num->i_val);
               auto initializer = ConstantZero::get(arrayType, module.get());//初始值赋为零
               auto Globle_FloatArrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);  //为数组分配空间
               scope.push(node.id,Globle_FloatArrayAlloca);
            }
            
        }
        else //全局变量
        {
             if(node.type==TYPE_INT)//整型变量
            {
               auto initializer = ConstantZero::get(TyInt32, module.get());
               auto Globle_IntAlloca = GlobalVariable::create(node.id, module.get(), TyInt32, false, initializer); //为变量分配空间
               scope.push(node.id,Globle_IntAlloca);
            }
            else if(node.type==TYPE_FLOAT)//浮点型变量
            {
               auto initializer = ConstantZero::get(TyFloat, module.get());
               auto Globle_FloatAlloca = GlobalVariable::create(node.id, module.get(), TyFloat, false, initializer);//为变量分配空间
               scope.push(node.id,Globle_FloatAlloca);
            }
        }
    }
    
 }

void CminusfBuilder::visit(ASTFunDeclaration &node) { 
    //fun-declaration → type-specifier ID ( params ) compound-stmt
    //四个内置函数定义在io.c中
    //TO DO 函数不能重名？？？？
    scope.enter();//进入函数的作用域
    Type* TYPE32 = Type::get_int32_type(module.get());
    Type *TyFloat = Type::get_float_type(module.get());
    Type* TYPEV = Type::get_void_type(module.get());
    Type* TYPEARRAY_32 = PointerType::get_int32_ptr_type(module.get());
    Type * funType = (node.type == TYPE_VOID) ? TYPEV : TYPE32;

    // 函数参数的vector
    std::vector<Value *> args;
    if(node.params.size()>0)//参数列表非空
    {
        for(auto param : node.params )
            {
                param->accept(*this);//得到参数类型并分配空间
            }
    // 由函数类型得到函数
    auto fun =  Function::create(FunctionType::get(funType,Ints), node.id, module.get());
    // BB的名字在生成中无所谓,但是可以方便阅读
    auto bb = BasicBlock::create(module.get(), "entry", fun);
    builder->set_insert_point(bb); 
    
    for(auto arg = fun->arg_begin(); arg!=fun->arg_end(); arg++)
    {
       args.push_back(*arg);
    }
    int i = 0;
    //将参数store下来
    for(auto param : node.params )
    {
        auto pAlloca = scope.find(param->id);
        if(pAlloca == nullptr)
             exit(0);
        else builder->create_store(args[i++], pAlloca);
        Ints.pop_back();//清空向量
    }
    scope.push(node.id,fun);//函数名放进作用域
    }
    else //参数列表为空
    {
        auto fun =  Function::create(FunctionType::get(funType,Ints), node.id, module.get());
        // BB的名字在生成中无所谓,但是可以方便阅读
        auto bb = BasicBlock::create(module.get(), "entry", fun);
        builder->set_insert_point(bb);
        scope.push(node.id,fun);
    }

    node.compound_stmt->accept(*this);
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {
    //param -> type-specifier ID | type-specifier ID [] 
    Type* TYPE32 = Type::get_int32_type(module.get());
    Type *TyFloat = Type::get_float_type(module.get());
    Type* TYPEARRAY_INT_32 = PointerType::get_int32_ptr_type(module.get());
    Type* TYPEARRAY_FLOAT_32 = PointerType::get_float_ptr_type(module.get());
    Value *pAlloca;
    //返回参数类型并分配空间
    if(node.isarray)//数组参数
    {
        if(node.type==TYPE_INT)
         {
             Ints.push_back(TYPEARRAY_INT_32);
             pAlloca = builder->create_alloca(TYPEARRAY_INT_32);//在内存中分配空间
             scope.push(node.id,pAlloca);
         } 
        else if(node.type==TYPE_FLOAT)
        {
            Ints.push_back(TYPEARRAY_FLOAT_32);
            pAlloca = builder->create_alloca(TYPEARRAY_FLOAT_32);
            scope.push(node.id,pAlloca);
        }
    }
    else if(node.type==TYPE_INT)//整型
        {
            Ints.push_back(TYPE32);
            pAlloca = builder->create_alloca(TYPE32);
            scope.push(node.id,pAlloca);
        }
    else if(node.type==TYPE_FLOAT)//浮点型
         {
            Ints.push_back(TyFloat);
            pAlloca = builder->create_alloca(TyFloat);
            scope.push(node.id,pAlloca);
         }
 }

void CminusfBuilder::visit(ASTCompoundStmt &node) { 
    // compound-stmt -> { local-declarations statement-list } 
     scope.enter();
     for(auto loc_decl: node.local_declarations)
     {
        loc_decl->accept(*this);
     }
     for(auto stmt :node.statement_list )
     {
         stmt->accept(*this);
     }
     scope.exit();
}

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
