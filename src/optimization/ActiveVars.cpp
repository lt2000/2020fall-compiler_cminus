#include "ActiveVars.hpp"
//#define DEBUG
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
            // else if (instr->is_store())
            // {
            //     auto l_val = static_cast<StoreInst *>(instr)->get_lval();
            //     auto r_val = static_cast<StoreInst *>(instr)->get_rval();
            //     if (NeedDef.find(r_val) != NeedDef.end())
            //     {
            //         used.insert(r_val);
            //         if (defed.find(r_val) == defed.end())
            //         {
            //             use_set[BB].insert(r_val);
            //             live_in[BB].insert(r_val);
            //         }
            //     }
            //     //不是数组，那必定是全局变量了
            //     if (!(static_cast<Instruction *>(l_val))->is_gep())
            //     {
            //         used.insert(l_val);
            //         use_set[BB].insert(l_val);
            //         live_in[BB].insert(l_val);
            //     }
            //     //是数组,l_val其实是由gep求出来的
            //     else
            //     {
            //         used.insert(l_val);
            //         if (defed.find(l_val) == defed.end())
            //         {
            //             use_set[BB].insert(l_val);
            //             live_in[BB].insert(l_val);
            //         }
            //     }
            // }
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

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
#ifdef DEBUG
    std::ofstream output_use_def;
    output_use_def.open("use_def.json", std::ios::out);
    output_use_def << "[";
#endif
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
#ifdef DEBUG
            output_use_def << use_def_print();
            output_use_def << ",";
#endif

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
#ifdef DEBUG
    output_use_def << "]";
    output_use_def.close();
#endif
    output_active_vars << "]";
    output_active_vars.close();
    return;
}

std::string ActiveVars::use_def_print()
{
    std::string use_def_set;
    func_->set_instr_name();
    use_def_set += "{\n";
    use_def_set += "\"function\": \"";
    use_def_set += func_->get_name();
    use_def_set += "\",\n";

    use_def_set += "\"use_set\": {\n";
    for (auto &p : use_set)
    {
        use_def_set += "  \"";
        use_def_set += p.first->get_name();
        use_def_set += "\": [";
        for (auto &v : p.second)
        {
            use_def_set += "\"%";
            use_def_set += v->get_name();
            use_def_set += "\",";
        }
        use_def_set += "]";
        use_def_set += ",\n";
    }
    use_def_set += "\n";
    use_def_set += "    },\n";

    use_def_set += "\"def_set\": {\n";
    for (auto &p : def_set)
    {
        use_def_set += "  \"";
        use_def_set += p.first->get_name();
        use_def_set += "\": [";
        for (auto &v : p.second)
        {
            use_def_set += "\"%";
            use_def_set += v->get_name();
            use_def_set += "\",";
        }
        use_def_set += "]";
        use_def_set += ",\n";
    }
    use_def_set += "\n";
    use_def_set += "    }\n";

    use_def_set += "}\n";
    use_def_set += "\n";
    return use_def_set;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    func_->set_instr_name();
    active_vars += "{\n";
    active_vars += "\"function\": \"";
    active_vars += func_->get_name();
    active_vars += "\",\n";

    active_vars += "\"live_in\": {\n";
    for (auto &p : live_in)
    {
        active_vars += "  \"";
        active_vars += p.first->get_name();
        active_vars += "\": [";
        for (auto &v : p.second)
        {
            active_vars += "\"%";
            active_vars += v->get_name();
            active_vars += "\",";
        }
        active_vars += "]";
        active_vars += ",\n";
    }
    active_vars += "\n";
    active_vars += "    },\n";

    active_vars += "\"live_out\": {\n";
    for (auto &p : live_out)
    {
        active_vars += "  \"";
        active_vars += p.first->get_name();
        active_vars += "\": [";
        for (auto &v : p.second)
        {
            active_vars += "\"%";
            active_vars += v->get_name();
            active_vars += "\",";
        }
        active_vars += "]";
        active_vars += ",\n";
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}