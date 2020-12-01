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


void CminusfBuilder::visit(ASTExpressionStmt &node)
//expression-stmt→expression ; ∣ ;
//expression→assign-expression ∣ simple-expression
{
    if(node.expression != nullptr){
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
//selection-stmt→ ​if ( expression ) statement∣ if ( expression ) statement else statement​
{
    Type* TYPE32 = Type::get_int32_type(module.get());
    node.expression->accept(*this);
    if(ret->get_type()->is_pointer_type())
        ret = builder->create_load(ret);
    if(ret->get_type()->is_float_type())
        ret = builder->create_fptosi(ret,TYPE32);
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto flag = builder->create_icmp_ne(ret,CONST_INT(0));
    auto trueBB = BasicBlock::create(module.get(),"trueBB",currentFunc);
    auto falseBB = BasicBlock::create(module.get(),"falseBB",currentFunc);
    auto nextBB = BasicBlock::create(module.get(),"nextBB",currentFunc);
    auto br = builder->create_cond_br(flag,trueBB,falseBB);
    int insertedflag = 0;

    //tureBB
    builder->set_insert_point(trueBB); 
    node.if_statement->accept(*this);
    if(builder->get_insert_block()->get_terminator() == nullptr)
    { // no return inside the block
        insertedflag = 1;
        builder->create_br(nextBB);
    }

    //falseBB
    builder->set_insert_point(falseBB);
    if(node.else_statement != nullptr)
    {
        node.else_statement->accept(*this);
        if(builder->get_insert_block()->get_terminator() == nullptr)
        { // no return inside the block
            insertedflag = 1;
            builder->create_br(nextBB);
        }
    }
    else
        builder->create_br(nextBB);
    
    // out
    builder->set_insert_point(nextBB); 
    // if(insertedflag) 
    //     nextBB->get_num_of_instr();
}

void CminusfBuilder::visit(ASTIterationStmt &node)
//iteration-stmt→while ( expression ) statement
{
    Type* TYPE32 = Type::get_int32_type(module.get());
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto loopJudge = BasicBlock::create(module.get(), "loopJudge", currentFunc);
    auto loopBody = BasicBlock::create(module.get(), "loopBody", currentFunc);
    auto out = BasicBlock::create(module.get(), "outloop", currentFunc);
    if(builder->get_insert_block()->get_terminator() == nullptr)
        builder->create_br(loopJudge);

    //loopJudge BB
    builder->set_insert_point(loopJudge);
    node.expression->accept(*this);
    if(ret->get_type()->is_pointer_type())
        ret = builder->create_load(ret);
    if(ret->get_type()->is_float_type())
        ret = builder->create_fptosi(ret,TYPE32);
    auto flag = builder->create_icmp_ne(ret,CONST_INT(0));
    builder->create_cond_br(flag,loopBody,out);

    //loopBody BB
    builder->set_insert_point(loopBody);
    node.statement->accept(*this);
    if(builder->get_insert_block()->get_terminator() == nullptr)
        builder->create_br(loopJudge);

    //outloop BB
    builder->set_insert_point(out);

}
void CminusfBuilder::visit(ASTReturnStmt &node)
//return-stmt→return ; ∣ return expression ;
{
    if(node.expression == nullptr)
    {
        builder->create_void_ret();
    }
    else
    {
        node.expression->accept(*this);
        if(ret->get_type()->is_pointer_type())
            ret = builder->create_load(ret);
        builder->create_ret(ret);
    }
    
}
void CminusfBuilder::visit(ASTVar &node)
//var→ID ∣ ID [ expression]
{
    Type* TYPE32 = Type::get_int32_type(module.get());
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto var = scope.find(node.id);
    if(var)
    {
        if(node.expression != nullptr)
        //id is an array
        {
            node.expression->accept(*this);
            auto num = ret;
            //transfer num to int
            if(num->get_type()->is_pointer_type())
                num = builder->create_load(num);
            if(num->get_type()->is_float_type())
                num = builder->create_fptosi(num,TYPE32);
            //if num < 0; enter exphandBB
            auto exphandBB = BasicBlock::create(module.get(),"exphandBB",currentFunc);
            auto normalBB = BasicBlock::create(module.get(),"normalBB",currentFunc);
            auto flagnum = builder->create_icmp_ge(num,CONST_INT(0));
            auto br = builder->create_cond_br(flagnum,normalBB,exphandBB);

            //normalBB
            builder->set_insert_point(normalBB);
            if(ret->get_type()->is_array_type())
            {
                //get first address of array
                var = builder->create_gep(var,{CONST_INT(0),CONST_INT(0)});
            }
            var = builder->create_gep(var,{num});
            ret = var;

            //exphandBB
            builder->set_insert_point(exphandBB);
            printf("var[expression],expression error\n");
        }
        else
        {
            ret = var;
        }
        
    }
    else
    {
        exit(0);
    }
    
}
void CminusfBuilder::visit(ASTAssignExpression &node)
//assign-expression→var = expression
{
    Type* TYPE32 = Type::get_int32_type(module.get());
    Type* TYPEFLOAT = Type::get_float_type(module.get());
    node.var.get()->accept(*this);
    Value* var = ret;
    node.expression.get()->accept(*this);
    auto ret_type = ret->get_type();
    if(var->get_type()->get_pointer_element_type()->is_float_type())
    {
        if(ret_type->is_integer_type())
            ret = builder->create_sitofp(ret,TYPEFLOAT);
        builder->create_store(ret,var);
    }
    else
    {
        if(ret_type->is_float_type())
            ret = builder->create_fptosi(ret,TYPE32);
        builder->create_store(ret,var);
    }
    
}

void CminusfBuilder::visit(ASTSimpleExpression &node) { 
    //simple-expression -> additive-expression relop additive- expression | additive-expression
    //simple-expression -> additive-expression
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    //简单加法表达式，通过accept调用下一层级
    if(!node.additive_expression_r){
        node.additive_expression_l->accept(*this);
    }
    //simple-expression -> additive-expression relop additive- expression
    //关系表达式，运算结果为整型1 或者 0
    else{
        //获取左值和右值
        Value* ret;
        Value* AdditiveLoad_l;
        Value* AdditiveLoad_r;
        Value* icmp;
        node.additive_expression_l->accept(*this);
        AdditiveLoad_l = builder->create_load(ret);
        node.additive_expression_r->accept(*this);
        AdditiveLoad_r = builder->create_load(ret);
        //标志是否为浮点数
        int flag = 0;
        //如果两个数中至少有一个是浮点数
        if(AdditiveLoad_l - (int)AdditiveLoad_l != 0 || AdditiveLoad_r - (int)AdditiveLoad_r != 0){
            //将两个数和结果都转换成浮点数
            AdditiveLoad_l = builder->create_sitofp(AdditiveLoad_l, FloatType);
            AdditiveLoad_r = builder->create_sitofp(AdditiveLoad_r, FloatType);
            icmp = builder->create_sitofp(icmp, FloatType);
            flag = 1;
         }
        if(flag == 1){
            switch (node.op)
            {
            case OP_GE:
                icmp = builder->create_fcmp_ge(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_GT:
                icmp = builder->create_fcmp_gt(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_LE:
                icmp = builder->create_fcmp_le(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_LT:
                icmp = builder->create_fcmp_lt(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_EQ:
                icmp = builder->create_fcmp_eq(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_NEQ:
                icmp = builder->create_fcmp_ne(AdditiveLoad_l, AdditiveLoad_r);
                break;
            default:
                break;
            }
        }
        else{
            switch (node.op)
            {
            case OP_GE:
                icmp = builder->create_icmp_ge(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_GT:
                icmp = builder->create_icmp_gt(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_LE:
                icmp = builder->create_icmp_le(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_LT:
                icmp = builder->create_icmp_lt(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_EQ:
                icmp = builder->create_icmp_eq(AdditiveLoad_l, AdditiveLoad_r);
                break;
            case OP_NEQ:
                icmp = builder->create_icmp_ne(AdditiveLoad_l, AdditiveLoad_r);
                break;
            default:
                break;
            }
        }
    ret = icmp;
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) { 
    //additive-expression -> additive-expression addop term | term
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Value* AdditiveExpression;
    Value* Term;
    Value* icmp;
    //additive-expression -> term
    //如果只是简单的项，转到下一层
    if(!node.additive_expression){
        node.additive_expression->accept(*this);
    }
    //additive-expression -> additive-expression addop term
    else{
        node.additive_expression->accept(*this);
        AdditiveExpression = builder->create_load(ret);
        node.term->accept(*this);
        Term = builder->create_load(ret);
        int flag = 0;
        //如果是浮点数相加
        if(AdditiveExpression - (int)AdditiveExpression != 0 || Term - (int)Term != 0){
            flag = 1;
            AdditiveExpression = builder->create_sitofp(AdditiveExpression, FloatType);
            Term = builder->create_sitofp(Term, FloatType);
            icmp = builder->create_sitofp(icmp, FloatType);
        }
        if(flag == 1){
            if(node.op == OP_PLUS){
                icmp = builder->create_fadd(AdditiveExpression, Term);
            }
            else{
                icmp = builder->create_fsub(AdditiveExpression, Term);
            }
        }
        else{
            if(node.op == OP_PLUS){
                icmp = builder->create_iadd(AdditiveExpression, Term);
            }
            else{
                icmp = builder->create_isub(AdditiveExpression, Term);
            }
        }
        ret = icmp;
    }
}

void CminusfBuilder::visit(ASTTerm &node) {
    //term -> term mulop factor | factor
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Value* Term;
    Value* Factor;
    Value* icmp;
    //term -> factor
    if(!node.term){
        node.term->accept(*this);
    }
    //term -> term mulop factor
    else{
        node.term->accept(*this);
        Term = builder->create_load(ret);
        node.factor->accept(*this);
        Factor = builder->create_load(ret);
        int flag = 0;
        if(Term - (int)Term != 0 || Factor - (int)Factor != 0){
            flag = 1;
            Term = builder->create_sitofp(Term, FloatType);
            Factor = builder->create_sitofp(Factor, FloatType);
            icmp = builder->create_sitofp(icmp, FloatType);
        }
        if(flag == 1){
            if(node.op == OP_MUL){
                icmp = builder->create_fmul(Term, Factor);
            }
            else{
                icmp = builder->create_fdiv(Term, Factor);
            }
        }
        else{
            if(node.op == OP_DIV){
                icmp = builder->create_imul(Term, Factor);
            }
            else{
                icmp = builder->create_isdiv(Term, Factor);
            }
        }
        ret = icmp;
    }
 }

void CminusfBuilder::visit(ASTCall &node) { 
    //根据名字寻找到对应的值
    Value* value;
    value = scope.find(node.id);
    Value* value_args;
    if(value == nullptr){
        return;
    }
    else{
        std::vector<Value *> function;
        Type *Int32Type = Type::get_int32_type(module.get());
        Type *FloatType = Type::get_float_type(module.get());
        Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
        Type *Int1Type = Type::get_int1_type(module.get());
        for(auto Args : node.args){
            Args->accept(*this);
            //如果是整型，存放地址
            if(ret->get_type() == Int32Type || ret->get_type() == FloatType){
                value_args = ret;
            }
            //如果是布尔型，转换成32位整型
            else if(ret->get_type() == Int1Type){
                value_args = builder->create_zext(ret, Int32Type);
            }
            //如果是指针
            else if(ret->get_type() == Int32PtrType){
                value_args = builder->create_load(ret);
            }
            function.push_back(value_args);
        }
        CallInst* call;
        ret = value_args;
        call = builder->create_call(value, function);
    }
}
