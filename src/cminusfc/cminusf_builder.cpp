#include "cminusf_builder.hpp"
#include "logging.hpp"
// #define Debug 1
#define CONST_INT(num) \
    ConstantInt::get(num, module.get())

#define CONST_FP(num) \
    ConstantFP::get(num, module.get()) // 得到常数值的表示,方便后面多次用到

//global variables
Value *ret;               //存储返回值
std::vector<Type *> Ints; //储存参数的类型，以确定函数的类型
int argload;
int return_flag = 0;       //全局变量标识当前模块是否已经有return语句

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node)
{
    //program -> declaration-list
#ifdef Debug
    printf("program!\n");
#endif
    for (auto decl : node.declarations)
    {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node)
{
#ifdef Debug
    printf("num!\n");
#endif
    if(return_flag)
        return;
    if (node.type == TYPE_INT)
    {
        ret = CONST_INT(node.i_val);
    }
    else if (node.type == TYPE_FLOAT)
    {
        ret = CONST_FP(node.f_val);
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    //var-declaration -> type-specifier ID ; ∣ type-specifier ID [INTEGER ]
#ifdef Debug
    printf("var_declaration!\n");
#endif
    if(return_flag)
        return;
    auto TyInt32 = Type::get_int32_type(module.get());
    auto TyFloat = Type::get_float_type(module.get());
    //局部变量,不能是空类型，push作用将id储存进作用域，以便之后赋值时查找是否声明
    if (!scope.in_global())
    {
        if (node.num != nullptr) //局部数组
        {
            if(!node.num->i_val)
            {
            Value * call_error = scope.find("neg_idx_except");//数组定义是大小为零时，打印报错信息
            builder->create_call(call_error,{});
            }
            if (node.type == TYPE_INT) //整型数组
            {
                auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
                auto Local_IntArrayAlloca = builder->create_alloca(arrayType); //为数组分配空间
                scope.push(node.id, Local_IntArrayAlloca);
            }
            else if (node.type == TYPE_FLOAT) //浮点型数组
            {
                auto *arrayType = ArrayType::get(TyFloat, node.num->i_val);
                auto Local_FloatArrayAlloca = builder->create_alloca(arrayType); //为数组分配空间
                scope.push(node.id, Local_FloatArrayAlloca);
            }
        }
        else //局部变量
        {
            if (node.type == TYPE_INT) //整型变量
            {
                auto Local_IntAlloca = builder->create_alloca(TyInt32); //为变量分配空间
                scope.push(node.id, Local_IntAlloca);
            }
            else if (node.type == TYPE_FLOAT) //浮点型变量
            {
                auto Local_FloatAlloca = builder->create_alloca(TyFloat); //为变量分配空间
                scope.push(node.id, Local_FloatAlloca);
            }
        }
    }
    else //全局变量
    {
        if (node.num != nullptr ) //全局数组
        {
            if(!node.num->i_val)
            {
            Value * call_error = scope.find("neg_idx_except");
            builder->create_call(call_error,{});
            }
            if (node.type == TYPE_INT) //整型数组
            {
                auto *arrayType = ArrayType::get(TyInt32, node.num->i_val);
                auto initializer = ConstantZero::get(arrayType, module.get());
                auto Globle_IntArrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer); //为数组分配空间
                scope.push(node.id, Globle_IntArrayAlloca);
            }
            else if (node.type == TYPE_FLOAT) //浮点型数组
            {
                auto *arrayType = ArrayType::get(TyFloat, node.num->i_val);
                auto initializer = ConstantZero::get(arrayType, module.get());                                               //初始值赋为零
                auto Globle_FloatArrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer); //为数组分配空间
                scope.push(node.id, Globle_FloatArrayAlloca);
            }
        }
        else //全局变量
        {
            if (node.type == TYPE_INT) //整型变量
            {
                auto initializer = ConstantZero::get(TyInt32, module.get());
                auto Globle_IntAlloca = GlobalVariable::create(node.id, module.get(), TyInt32, false, initializer); //为变量分配空间
                scope.push(node.id, Globle_IntAlloca);
            }
            else if (node.type == TYPE_FLOAT) //浮点型变量
            {
                auto initializer = ConstantZero::get(TyFloat, module.get());
                auto Globle_FloatAlloca = GlobalVariable::create(node.id, module.get(), TyFloat, false, initializer); //为变量分配空间
                scope.push(node.id, Globle_FloatAlloca);
            }
        }
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    //fun-declaration → type-specifier ID ( params ) compound-stmt
    //四个内置函数定义在io.c中
    //TO DO 函数不能重名？？？？
#ifdef Debug
    printf("fun_declaration!\n");
#endif
    scope.enter(); //进入函数的作用域
    Type *TYPE32 = Type::get_int32_type(module.get());
    Type *TyFloat = Type::get_float_type(module.get());
    Type *TYPEV = Type::get_void_type(module.get());
    Type *TYPEARRAY_32 = PointerType::get_int32_ptr_type(module.get());
    Type *funType;

    Type *TYPEARRAY_INT_32 = PointerType::get_int32_ptr_type(module.get());
    Type *TYPEARRAY_FLOAT_32 = PointerType::get_float_ptr_type(module.get());

    if (node.type == TYPE_FLOAT)
        funType = TyFloat;
    else
    {
        funType = (node.type == TYPE_VOID) ? TYPEV : TYPE32;
    }

    // 函数参数的vector
    std::vector<Value *> args;
    if (node.params.size() > 0) //参数列表非空
    {
        for (auto param : node.params)
        {
            param->accept(*this); //得到参数类型并分配空间
        }
        // 由函数类型得到函数
        auto fun = Function::create(FunctionType::get(funType, Ints), node.id, module.get());
        // BB的名字在生成中无所谓,但是可以方便阅读
        auto bb = BasicBlock::create(module.get(), "entry", fun);
        builder->set_insert_point(bb);
        scope.exit();
        scope.push(node.id, fun); //函数名放进作用域
        scope.enter();
        for (auto param : node.params) //alloca
        {

            if (param->isarray)
            {
                if (param->type == TYPE_INT)
                {
                    auto pAlloca = builder->create_alloca(TYPEARRAY_INT_32); //在内存中分配空间
                    scope.push(param->id, pAlloca);
                }
                else if (param->type == TYPE_FLOAT)
                {
                    auto pAlloca = builder->create_alloca(TYPEARRAY_FLOAT_32);
                    scope.push(param->id, pAlloca);
                }
            }
            else if (param->type == TYPE_INT) //整型
            {
                auto pAlloca = builder->create_alloca(TYPE32);
                scope.push(param->id, pAlloca);
            }
            else if (param->type == TYPE_FLOAT) //浮点型
            {
                auto pAlloca = builder->create_alloca(TyFloat);
                scope.push(param->id, pAlloca);
            }
        }
        for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++)
        {
            args.push_back(*arg);
        }
        int i = 0;
        //将参数store下来
        for (auto param : node.params)
        {
            auto pAlloca = scope.find(param->id);
            if (pAlloca == nullptr)
                exit(0);
            else
                builder->create_store(args[i++], pAlloca);
            Ints.pop_back(); //清空向量
        }
    }
    else //参数列表为空
    {
        auto fun = Function::create(FunctionType::get(funType, Ints), node.id, module.get());
        // BB的名字在生成中无所谓,但是可以方便阅读
        auto bb = BasicBlock::create(module.get(), "entry", fun);
        builder->set_insert_point(bb);
        scope.exit();
        scope.push(node.id, fun); //函数名放进作用域
        scope.enter();
    }
    node.compound_stmt->accept(*this);
    if(return_flag == 0)
    {
        auto return_type = builder->get_insert_block()->get_parent()->get_return_type();
        if(return_type->is_void_type())
            builder->create_void_ret();
        else if(return_type->is_integer_type())
            builder->create_ret(CONST_INT(0));
        else
            builder->create_ret(CONST_FP(0));
    }
    return_flag = 0;
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node)
{
#ifdef Debug
    printf("param!\n");
#endif
    //param -> type-specifier ID | type-specifier ID []
    Type *TYPE32 = Type::get_int32_type(module.get());
    Type *TyFloat = Type::get_float_type(module.get());
    Type *TYPEARRAY_INT_32 = PointerType::get_int32_ptr_type(module.get());
    Type *TYPEARRAY_FLOAT_32 = PointerType::get_float_ptr_type(module.get());

    //返回参数类型并分配空间
    if (node.isarray) //数组参数
    {
        if (node.type == TYPE_INT)
        {
            Ints.push_back(TYPEARRAY_INT_32);
        }
        else if (node.type == TYPE_FLOAT)
        {
            Ints.push_back(TYPEARRAY_FLOAT_32);
        }
    }
    else if (node.type == TYPE_INT) //整型
    {
        Ints.push_back(TYPE32);
    }
    else if (node.type == TYPE_FLOAT) //浮点型
    {
        Ints.push_back(TyFloat);
    }
    return;
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    // compound-stmt -> { local-declarations statement-list }
#ifdef Debug
    printf("compound_stmt!\n");
#endif
    if(return_flag)
        return;
    scope.enter();
    for (auto loc_decl : node.local_declarations)
    {
        loc_decl->accept(*this);
    }
    for (auto stmt : node.statement_list)
    {
        stmt->accept(*this);
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
//expression-stmt→expression ; ∣ ;
//expression→assign-expression ∣ simple-expression
{
#ifdef Debug
    printf("expression_stmt!\n");
#endif
    if(return_flag)
        return;
    if (node.expression != nullptr)
    {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
//selection-stmt→ ​if ( expression ) statement∣ if ( expression ) statement else statement​
{
#ifdef Debug
    printf("selection_stmt!\n");
#endif
    if(return_flag)
        return;
    Type *TYPE32 = Type::get_int32_type(module.get());
    node.expression->accept(*this);
    if (ret->get_type()->is_pointer_type())
        ret = builder->create_load(ret);
    if (ret->get_type()->is_float_type())
        ret = builder->create_fcmp_ne(ret, CONST_FP(0));
    else if (ret->get_type() == TYPE32)
        ret = builder->create_icmp_ne(ret, CONST_INT(0));
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto trueBB = BasicBlock::create(module.get(), "", currentFunc);
    BasicBlock *falseBB;
    BasicBlock *nextBB;
    BranchInst *br;
    int insertedflag = 0;

    //falseBB,假分支放在前面是为了保证在ifelse嵌套时，nextbb的序号按序
    if (node.else_statement != nullptr) //有else
    {
        falseBB = BasicBlock::create(module.get(), "", currentFunc);
        br = builder->create_cond_br(ret, trueBB, falseBB);
        //falseBB
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if (builder->get_insert_block()->get_terminator() == nullptr)
        { // no return inside the block
            insertedflag = 1;
            nextBB = BasicBlock::create(module.get(), "", currentFunc);
            builder->create_br(nextBB);
        }
        return_flag = 0;
        //tureBB
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        if (builder->get_insert_block()->get_terminator() == nullptr)
        { // no return inside the block
            if (insertedflag == 0)
            {
                insertedflag = 1;
                nextBB = BasicBlock::create(module.get(), "", currentFunc);
            }
            builder->create_br(nextBB);
        }
        return_flag = !insertedflag;
        //nextBB
        if (insertedflag == 1)
        {
            builder->set_insert_point(nextBB);
        }
    }
    else
    {
        //tureBB
        nextBB = BasicBlock::create(module.get(), "", currentFunc);
        br = builder->create_cond_br(ret, trueBB, nextBB);
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        if (return_flag == 0)
        {
            builder->create_br(nextBB);
        }
        return_flag = 0;

        //nextBB
        builder->set_insert_point(nextBB);
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node)
//iteration-stmt→while ( expression ) statement
{
#ifdef Debug
    printf("iteration_stmt!\n");
#endif
    if(return_flag)
        return;
    Type *TYPE32 = Type::get_int32_type(module.get());
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto loopJudge = BasicBlock::create(module.get(), "", currentFunc);
    auto loopBody = BasicBlock::create(module.get(), "", currentFunc);
    auto out = BasicBlock::create(module.get(), "", currentFunc);
    if (builder->get_insert_block()->get_terminator() == nullptr)
        builder->create_br(loopJudge);

    //loopJudge BB
    builder->set_insert_point(loopJudge);
    node.expression->accept(*this);
    if (ret->get_type()->is_pointer_type())
        ret = builder->create_load(ret);
    if (ret->get_type()->is_float_type())
        ret = builder->create_fcmp_ne(ret, CONST_FP(0));
    else if (ret->get_type() == TYPE32)
        ret = builder->create_icmp_ne(ret, CONST_INT(0));
    auto br = builder->create_cond_br(ret, loopBody, out);

    //loopBody BB
    builder->set_insert_point(loopBody);
    node.statement->accept(*this);
    if (builder->get_insert_block()->get_terminator() == nullptr)
        builder->create_br(loopJudge);
    return_flag = 0;

    //outloop BB
    builder->set_insert_point(out);
}
void CminusfBuilder::visit(ASTReturnStmt &node)
//return-stmt→return ; ∣ return expression ;
{
#ifdef Debug
    printf("return_stmt!\n");
#endif
    if(return_flag)
        return;
    Type *TYPE32 = Type::get_int32_type(module.get());
    Type *TYPE1 = Type::get_int1_type(module.get());
    Type *TyFloat = Type::get_float_type(module.get());
    auto return_type = builder->get_insert_block()->get_parent()->get_return_type();
    if (node.expression == nullptr)
    {
        if (!return_type->is_void_type())
            printf("return_type is not void, but expression is empty\n");
        builder->create_void_ret();
    }
    else
    {
        node.expression->accept(*this);
        if (return_type->is_void_type())
        {
            printf("return_type is void, but expression is not empty\n");
            builder->create_void_ret();
            return;
        }
        if (ret->get_type()->is_pointer_type())
            ret = builder->create_load(ret);
        if (return_type == TYPE32)
        {
            if (ret->get_type() == TYPE1)
                ret = builder->create_zext(ret, TYPE32);
            else if (ret->get_type() == TyFloat)
                ret = builder->create_fptosi(ret, TYPE32);
        }
        if (return_type == TyFloat)
        {
            if (ret->get_type()->is_integer_type())
                ret = builder->create_sitofp(ret, TyFloat);
        }

        builder->create_ret(ret);
    }
    return_flag = 1;
}
void CminusfBuilder::visit(ASTVar &node)
//var→ID ∣ ID [ expression]
{
#ifdef Debug
    printf("var!\n");
#endif
    if(return_flag)
        return;
    Type *FloatPtrType = Type::get_float_ptr_type(module.get());
    Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
    Type *TYPE32 = Type::get_int32_type(module.get());
    Type *TYPE1 = Type::get_int1_type(module.get());
    //currentFunction
    auto currentFunc = builder->get_insert_block()->get_parent();
    auto var = scope.find(node.id);
    argload = 1;
    if (var)
    {
        if (node.expression != nullptr)
        //id is an array
        {
            // printf("\t\tvar-expression\n");
            node.expression->accept(*this);
            Value *num = ret;
            //transfer num to int
            if (num->get_type()->is_pointer_type())
                num = builder->create_load(num);
            if (num->get_type() == TYPE1)
                num = builder->create_zext(num, TYPE32);
            else if (num->get_type()->is_float_type())
                num = builder->create_fptosi(num, TYPE32);
            //if num < 0; enter exphandBB
            auto exphandBB = BasicBlock::create(module.get(), "", currentFunc);
            auto normalBB = BasicBlock::create(module.get(), "", currentFunc);
            auto outBB = BasicBlock::create(module.get(), "", currentFunc);
            auto flagnum = builder->create_icmp_ge(num, CONST_INT(0));
            auto br = builder->create_cond_br(flagnum, normalBB, exphandBB);

            //normalBB
            builder->set_insert_point(normalBB);
            if (var->get_type()->get_pointer_element_type()->is_pointer_type())
            {
                //var is an array that sub func get from main func
                auto var_load = builder->create_load(var);
                var = builder->create_gep(var_load, {num});
                // printf("var-exp-array in sub func\n");
            }
            else if (var->get_type()->get_pointer_element_type()->is_array_type())
            {
                //var is an id of array,get address of id[num]
                var = builder->create_gep(var, {CONST_INT(0), num});
                // printf("var-exp-arrary\n");
            }
            else
            {
                printf("id is a float or int, but expression is not empty\n");
            }

            ret = var;
            builder->create_br(outBB);

            //exphandBB
            builder->set_insert_point(exphandBB);
            Value * call_error = scope.find("neg_idx_except");
            builder->create_call(call_error, {});
            builder->create_br(outBB);
            //outBB
            builder->set_insert_point(outBB);
        }
        else
        {
            if (var->get_type()->get_pointer_element_type()->is_float_type() || var->get_type()->get_pointer_element_type()->is_integer_type())
            {
                argload = 1;
            }
            else if (var->get_type()->get_pointer_element_type()->is_array_type())
            {
                var = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
                // printf("arrary_arg\n");
                argload = 0;
            }
            else
            {
                var = builder->create_load(var);
                argload = 0;
            }
            ret = var;
        }
    }
    else
    {
        printf("cannot find the var\n");
        return;
    }
}
void CminusfBuilder::visit(ASTAssignExpression &node)
//assign-expression→var = expression
{
#ifdef Debug
    printf("assign_expression!\n");
#endif
    if(return_flag)
        return;
    Type *TYPE32 = Type::get_int32_type(module.get());
    Type *TYPE1 = Type::get_int1_type(module.get());
    Type *TYPEFLOAT = Type::get_float_type(module.get());
    node.var.get()->accept(*this);
    Value *var = ret;
    node.expression.get()->accept(*this);
    if (var->get_type()->get_pointer_element_type()->is_float_type())
    {
        if (ret->get_type()->is_pointer_type())
            ret = builder->create_load(ret);
        if (ret->get_type()->is_integer_type())
            ret = builder->create_sitofp(ret, TYPEFLOAT);
        builder->create_store(ret, var);
    }
    else
    {
        if (ret->get_type()->is_pointer_type())
            ret = builder->create_load(ret);
        if (ret->get_type() == TYPE1)
            ret = builder->create_zext(ret, TYPE32);
        else if (ret->get_type()->is_float_type())
            ret = builder->create_fptosi(ret, TYPE32);
        builder->create_store(ret, var);
    }
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
#ifdef Debug
    printf("simple_expression!\n");
#endif
    if(return_flag)
        return;
    //simple-expression -> additive-expression relop additive- expression | additive-expression
    //simple-expression -> additive-expression
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Type *Int1Type = Type::get_int1_type(module.get());
    //简单加法表达式，通过accept调用下一层级
    if (!node.additive_expression_r)
    {
        node.additive_expression_l->accept(*this);
    }
    //simple-expression -> additive-expression relop additive- expression
    //关系表达式，运算结果为整型1 或者 0
    else
    {
        //获取左值和右值
        Value *AdditiveLoad_l;
        Value *AdditiveLoad_r;
        Value *icmp;
        node.additive_expression_l->accept(*this);
        if (ret->get_type()->is_pointer_type())
            AdditiveLoad_l = builder->create_load(ret);
        else
            AdditiveLoad_l = ret;
        node.additive_expression_r->accept(*this);
        if (ret->get_type()->is_pointer_type())
            AdditiveLoad_r = builder->create_load(ret);
        else
            AdditiveLoad_r = ret;
        //标志是否为浮点数
        int flag = 0;
        //如果两个数中至少有一个是浮点数
        if (AdditiveLoad_l->get_type()->is_float_type())
        {
            flag = 1;
            if (AdditiveLoad_r->get_type()->is_integer_type())
                AdditiveLoad_r = builder->create_sitofp(AdditiveLoad_r, FloatType);
        }
        else
        {
            if (AdditiveLoad_r->get_type()->is_float_type())
            {
                flag = 1;
                AdditiveLoad_l = builder->create_sitofp(AdditiveLoad_l, FloatType);
            }
            else
            {
                flag = 0;
                if (AdditiveLoad_l->get_type() == Int1Type)
                    AdditiveLoad_l = builder->create_zext(AdditiveLoad_l, Int32Type);
                if (AdditiveLoad_r->get_type() == Int1Type)
                    AdditiveLoad_r = builder->create_zext(AdditiveLoad_r, Int32Type);
            }
        }

        if (flag == 1)
        {
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
        else
        {
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

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    //additive-expression -> additive-expression addop term | term
#ifdef Debug
    printf("additive_expression!\n");
#endif
    if(return_flag)
        return;
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *Int1Type = Type::get_int1_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Value *AdditiveExpression;
    Value *Term;
    Value *icmp;
    //additive-expression -> term
    //如果只是简单的项，转到下一层
    if (node.additive_expression == nullptr)
    {
        node.term->accept(*this);
    }
    //additive-expression -> additive-expression addop term
    else
    {
        node.additive_expression->accept(*this);
        if (ret->get_type()->is_pointer_type())
            AdditiveExpression = builder->create_load(ret);
        else
            AdditiveExpression = ret;
        node.term->accept(*this);
        if (ret->get_type()->is_pointer_type())
            Term = builder->create_load(ret);
        else
            Term = ret;
        int flag = 0;
        //如果是浮点数相加
        if (AdditiveExpression->get_type()->is_float_type())
        {
            flag = 1;
            if (Term->get_type()->is_integer_type())
                Term = builder->create_sitofp(Term, FloatType);
        }
        else
        {
            if (Term->get_type()->is_float_type())
            {
                flag = 1;
                AdditiveExpression = builder->create_sitofp(AdditiveExpression, FloatType);
            }
            else
            {
                flag = 0;
                if (AdditiveExpression->get_type() == Int1Type)
                    AdditiveExpression = builder->create_zext(AdditiveExpression, Int32Type);
                if (Term->get_type() == Int1Type)
                    Term = builder->create_zext(Term, Int32Type);
            }
        }

        if (flag == 1)
        {
            if (node.op == OP_PLUS)
            {
                icmp = builder->create_fadd(AdditiveExpression, Term);
            }
            else
            {
                icmp = builder->create_fsub(AdditiveExpression, Term);
            }
        }
        else
        {
            if (node.op == OP_PLUS)
            {
                icmp = builder->create_iadd(AdditiveExpression, Term);
            }
            else
            {
                icmp = builder->create_isub(AdditiveExpression, Term);
            }
        }
        ret = icmp;
    }
}

void CminusfBuilder::visit(ASTTerm &node)
{
    //term -> term mulop factor | factor
#ifdef Debug
    printf("term!\n");
#endif
    if(return_flag)
        return;
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *Int1Type = Type::get_int1_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Value *Term;
    Value *Factor;
    Value *icmp;
    //term -> factor
    if (!node.term)
    {
        node.factor->accept(*this);
    }
    //term -> term mulop factor
    else
    {
        node.term->accept(*this);
        if (ret->get_type()->is_pointer_type())
            Term = builder->create_load(ret);
        else
            Term = ret;
        node.factor->accept(*this);
        if (ret->get_type()->is_pointer_type())
            Factor = builder->create_load(ret);
        else
            Factor = ret;
        int flag = 0;
        if (Term->get_type()->is_float_type())
        {
            flag = 1;
            if (Factor->get_type()->is_integer_type())
                Factor = builder->create_sitofp(Factor, FloatType);
        }
        else
        {
            if (Factor->get_type()->is_float_type())
            {
                flag = 1;
                Term = builder->create_sitofp(Term, FloatType);
            }
            else
            {
                flag = 0;
                if (Factor->get_type() == Int1Type)
                    Factor = builder->create_zext(Factor, Int32Type);
                if (Term->get_type() == Int1Type)
                    Term = builder->create_zext(Term, Int32Type);
            }
        }

        if (flag == 1)
        {
            if (node.op == OP_MUL)
            {
                icmp = builder->create_fmul(Term, Factor);
            }
            else
            {
                icmp = builder->create_fdiv(Term, Factor);
            }
        }
        else
        {
            if (node.op == OP_MUL)
            {
                icmp = builder->create_imul(Term, Factor);
            }
            else
            {
                icmp = builder->create_isdiv(Term, Factor);
            }
        }
        ret = icmp;
    }
}

void CminusfBuilder::visit(ASTCall &node)
{
    //根据名字寻找到对应的值
#ifdef Debug
    printf("call!");
#endif
    if(return_flag)
        return; 
    Value *value;
    value = scope.find(node.id);
    if (value == nullptr)
    {
        printf("cannot find the fun\n");
        return;
    }
        
    auto fun = value->get_type();
    if (!fun->is_function_type())
        return;
        
    auto callfun = static_cast<FunctionType *>(fun);
    Value *value_args;
    int i = 0;
    std::vector<Value *> function;
    Type *Int32Type = Type::get_int32_type(module.get());
    Type *FloatType = Type::get_float_type(module.get());
    Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
    Type *Int1Type = Type::get_int1_type(module.get());
    for (auto Args : node.args)
    {
        auto arg_type = callfun->get_param_type(i);
        i++;
        Args->accept(*this);
        //如果ret是布尔型，ret先转换成32位整型
        if (ret->get_type() == Int1Type)
        {
            ret = builder->create_zext(ret, Int32Type);
        }
        //要求的参数为整型
        if (arg_type == Int32Type)
        {
            if (argload && ret->get_type()->is_pointer_type())
                ret = builder->create_load(ret);
            if (ret->get_type()->is_pointer_type())
                return;
            else if (ret->get_type() == FloatType)
                ret = builder->create_fptosi(ret, Int32Type);
            value_args = ret;
        }
        //要求的参数为浮点数
        else if (arg_type == FloatType)
        {
            if (argload && ret->get_type()->is_pointer_type())
                ret = builder->create_load(ret);
            if (ret->get_type()->is_pointer_type())
                return;
            else if (ret->get_type() == Int32Type)
                ret = builder->create_sitofp(ret, FloatType);
            value_args = ret;
        }
        //要求的参数为指针
        else
        {
            if (ret->get_type() == Int32Type || ret->get_type() == FloatType || argload)
                return;
            value_args = ret;
        }
        function.push_back(value_args);
    }
    if (i != callfun->get_num_of_args())
    {
        printf("\t the num of arg error\n");
        return;
    }
    //call,get into sub func
    ret = builder->create_call(value, function);
}
