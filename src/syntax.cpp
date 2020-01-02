#include <iostream>
#include <string>
#include <cctype>
#include <set>
#include "header.h"

using namespace std;

int curLevel = 0;
bool isNegative = false;
BaseType returnStatType;
bool expHasMinu = false;
InterSym *expression();

InterSym *r_f_stat(bool reportIdenNotFindError);

void n_r_f_stat();

bool statement(bool returned);

bool stat_list(bool returned);

LevelMng lvlMng;

int levelCnt = -1;

void gen_new_level() {
    lvlMng.levelVec.push_back(*new Level(levelCnt + 1, curLevel));
    curLevel = ++levelCnt;
}

int get_out_level() {
    return lvlMng.levelVec.at(curLevel).outid;
}

void level_retract() {
    curLevel = get_out_level();
}

static void error() {
    cerr << "err: in syntax.cpp, cur = " << cur << ", curChar = " << curChar << endl;
    cerr << src + cur << endl;
    //exit(-11);
}

static void error(string s) {
    cerr << s << " in syntax.cpp, cur = " << cur << ", curChar = " << curChar << endl;
    cerr << src + cur << endl;
    //exit(-11);
}

void find_syn_part(string part) {
    //outfile << part << endl;
}

bool assert_symbol_is(Symbol sy) {
    bool ret = true;
    if (symbol != sy) {
        ret = false;
        error();
        switch (sy) {
            case Symbol::SEMICN:
                globalErr.catch_e(curLineNum - sub_lines(), "k");
                sym_retract();
                break;
            case Symbol::RPARENT:
                globalErr.catch_e(curLineNum, "l");
                sym_retract();
                break;
            case Symbol::RBRACK:
                globalErr.catch_e(curLineNum, "m");
                sym_retract();
                break;
            case Symbol::WHILETK:
                globalErr.catch_e(curLineNum, "n");
                sym_retract();
                break;
            default:
                break;
        }
    }
    return ret;
}

void is_string() {
    assert_symbol_is(Symbol::STRCON);
    sym_output();
    find_syn_part("<字符串>");
}

bool is_unsigned_int() {
    bool ret = assert_symbol_is(Symbol::INTCON);
    sym_output();
    find_syn_part("<无符号整数>");
    return ret;
}

bool is_int() {
    if (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
        isNegative = (symbol == Symbol::MINU);
        getsym();
	} else {
		isNegative = false;
	}
    bool ret = is_unsigned_int();
    sym_output();
    find_syn_part("<整数>");
    return ret;
}

bool bool_type_iden() {
    return (symbol == Symbol::INTTK || symbol == Symbol::CHARTK);
}

bool bool_relat_op() {
    set<Symbol> sett({Symbol::LSS, Symbol::LEQ, Symbol::GRE, Symbol::
    GEQ, Symbol::NEQ, Symbol::EQL});
    return sett.find(symbol) != sett.end();
}

void const_definition() {
    if (token == "int") {
        do {
            Specifier spc(true, BaseType::INT);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            SymbolC symC(token, curLevel, curLineNum, spc);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            if (!is_int()) {
                globalErr.catch_e(curLineNum, "o");
            }
            symC.value = (isNegative) ? -number : number;
            if (!symTable.add(symC, get_out_level())->duplicate) {
                genInter.find_const_var_def(symC);
            }
            getsym();
        } while (symbol == Symbol::COMMA);
    } else if (token == "char") {
        do {
            Specifier spc(true, BaseType::CHAR);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            SymbolC symC(token, curLevel, curLineNum, spc);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            if (!assert_symbol_is(Symbol::CHARCON)) {
                globalErr.catch_e(curLineNum, "o");
            }
            symC.value = (int)character;
            if (!symTable.add(symC, get_out_level())->duplicate) {
                genInter.find_const_var_def(symC);
            }
            getsym();
        } while (symbol == Symbol::COMMA);
    } else {
        error();
    }
    sym_retract();
    sym_output();
    find_syn_part("<常量定义>");
    getsym();
}

void const_description() {
    assert_symbol_is(Symbol::CONSTTK);
    do {
        getsym();
        const_definition();
        assert_symbol_is(Symbol::SEMICN);
        getsym();
    } while (symbol == Symbol::CONSTTK);
    sym_retract();
    sym_output();
    find_syn_part("<常量说明>");
    getsym();
}

void sbtype2bstype(Symbol sym, Specifier *spc) {
    (*spc).isConst = false;
    switch (sym) {
        case Symbol::INTTK:
            (*spc).baseType = BaseType::INT;
            break;
        case Symbol::CHARTK:
            (*spc).baseType = BaseType::CHAR;
            break;
        default:
            break;
    }
}

void var_definition(Symbol varsym) {
    Specifier spc;
    sbtype2bstype(varsym, &spc);
    while (symbol == Symbol::COMMA) {
        Specifier spc;
        sbtype2bstype(varsym, &spc);
        getsym();
        assert_symbol_is(Symbol::IDENFR);
        SymbolC sbc(token, curLevel, curLineNum, spc);
        getsym();
        if (token.at(0) == '[') {
            getsym();
            is_unsigned_int();
            sbc.arr = true;
            sbc.size = number * 4;
            getsym();
            if (token.at(0) != ']') error();
            getsym();
        }
        if (!symTable.add(sbc, get_out_level())->duplicate) {
            genInter.find_normal_var_def(sbc);
        }
    }
    sym_retract();
    sym_output();
    find_syn_part("<变量定义>");
    getsym();
}

void var_description() {
    bool have_var_def = false;
    if (!bool_type_iden()) error();
    Symbol varsym = symbol;
    while (bool_type_iden()) {
        Specifier spc;
        sbtype2bstype(symbol, &spc);
        getsym();
        assert_symbol_is(Symbol::IDENFR);
        SymbolC sbc(token, curLevel, curLineNum, spc);
        getsym();
        // var, else ret_func
        if (token.at(0) == '[' || token.at(0) == ',' || token.at(0) == ';') {
            if (token.at(0) == '[') {
                getsym();
                is_unsigned_int();
                sbc.arr = true;
                sbc.size = number * 4;
                getsym();
                if (token.at(0) != ']') error();
                getsym();
            }
            if (!symTable.add(sbc, get_out_level())->duplicate) {
                genInter.find_normal_var_def(sbc);
            }
            var_definition(varsym);
            if (token.at(0) != ';') error();
            getsym();
            varsym = symbol;
            have_var_def = true;
        } else {    // func
            sym_retract();
            sym_retract();
            break;
        }
    }
    sym_retract();
    sym_output();
    if (have_var_def) {
        find_syn_part("<变量说明>");
    }
    getsym();
}

InterSym *factor() {
    string toPrint;
    InterSym *iS = new InterSym();
    if (symbol == Symbol::IDENFR) {
        string varName = token;
        iS->interType = InterType::SYMC;
        iS->name = token;
        if (!symTable.contain(token, curLevel, true)) {
            globalErr.catch_e(curLineNum, "c");
        }
        getsym();
        if (symbol == Symbol::LBRACK) {    // '[', array
            iS->set_inter_type(InterType::INTER);
            iS->set_tmp_name();
            getsym();
            InterSym *exp = expression();
            if (exp->baseType != BaseType::INT) {
                globalErr.catch_e(curLineNum, "i");
            }
            genInter.prt(iS->name + " = _" + varName + '[' + exp->name + "]\n");
            assert_symbol_is(Symbol::RBRACK);
            getsym();
        } else if (symbol == Symbol::LPARENT) {    // func result
            iS->interType = InterType::INTER;
            sym_retract();
            sym_retract();
            getsym();
            bool returned = (*symTable.find(token, 0, true)).is_ret_func();
            if (!returned) {
                // use a void func as a factor
                //globalErr.catch_e(curLineNum, "e");
                n_r_f_stat();
            } else {
                iS = r_f_stat(false);   // @tmp saves RET
            }
        } else {
            iS->symc = symTable.find(iS->name, curLevel, true);
            if (iS->symc->spec.isConst) {
                if (iS->symc->spec.baseType == BaseType::INT) {
                    iS->interType = InterType::INTCONST;
                    iS->name = to_string(iS->symc->value);
                    iS->intcon = iS->symc->value;
                } else if (iS->symc->spec.baseType == BaseType::CHAR) {
                    iS->interType = InterType::CHARCONST;
                    iS->name = string(1, (char)(iS->symc->value));
                    iS->singleChar = true;
                    iS->intcon = iS->symc->value;
                }
            }
        }
    } else if (symbol == Symbol::LPARENT) { // (exp)
        iS->interType = InterType::INTER;
        getsym();
        InterSym *exp = expression();
        if (exp->interType == InterType::CHARCONST) {   // char -> int
            iS->name = to_string((int)exp->name[1]);
        } else {
            iS->name = exp->name;
        }
        if (iS->name[0] == '-') {
            iS->name = iS->name.substr(1);
            genInter.prt(iS->name + " = -" + iS->name + "\n");
        }
        assert_symbol_is(Symbol::RPARENT);
        getsym();
    } else if (symbol == Symbol::PLUS || symbol == Symbol::MINU
               || symbol == Symbol::INTCON) {
        if (symbol == Symbol::MINU) { iS->name += '-'; }
        iS->interType = InterType::INTCONST;
        is_int();
        iS->name += to_string(number);
        getsym();
    } else if (symbol == Symbol::CHARCON) {
        iS->interType = InterType::CHARCONST;
        iS->name = token;	// fuck this off
        iS->intcon = int(token[0]);
        iS->singleChar = true;
        getsym();
    } else {
        error();
    }
    sym_retract();
    sym_output();
    find_syn_part("<因子>");
    getsym();
    return iS;
}

InterSym *item() {
    string toPrint;
    InterSym *iS = new InterSym();
    InterSym *firstFact = factor();
    int cnt = 1;
    bool factorOver1 = false;
    while (symbol == Symbol::MULT || symbol == Symbol::DIV) {
        if (!factorOver1) {
            iS->interType = InterType::INTER;
            iS->set_tmp_name();
            if (firstFact->interType == InterType::SYMC) {
                toPrint += iS->name + " = _" + firstFact->name;
            } else if (firstFact->interType == InterType::CHARCONST) {
                toPrint += iS->name + " = " + to_string((int)firstFact->name[0]);
            } else {
                toPrint += iS->name + " = " + firstFact->name;
            }
            factorOver1 = true;
        }
        if (++cnt > 2) {
            toPrint += '\n' + iS->name + " = " + iS->name;
            cnt = 2;
        }
        toPrint += (symbol == Symbol::MULT ? " * " : " / ");
        getsym();
        InterSym *fact = factor();
        if (fact->interType == InterType::SYMC) {
            toPrint += "_" + fact->name;
        } else if (fact->interType == InterType::CHARCONST) {
            toPrint += to_string(int(fact->name[0]));    // attention: charcon!
        } else {
            toPrint += fact->name;
        }
    }
    sym_retract();
    sym_output();
    if (!factorOver1) {
        find_syn_part("<项>");
        getsym();
        return firstFact;
    }
    genInter.prt(toPrint + "\n");
    find_syn_part("<项>");
    getsym();
    return iS;
}

InterSym *expression() {
    string toPrint;
    BaseType ret = BaseType::INT;
    bool sureInt = false;
    InterSym *iSym = new InterSym();
    iSym->interType = InterType::INTER;
    iSym->set_tmp_name();
    toPrint += iSym->name + " = ";
    bool minu = false;
    if (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
        sureInt = true;
        toPrint += (symbol == Symbol::PLUS ? "+" : "-");
        minu = (symbol == Symbol::PLUS) ? false : true;
        Symbol preOp = symbol;
        getsym();
        if (symbol == Symbol::PLUS || symbol == Symbol::MINU) { // --n x, -n v .--1 v
            getsym();
            if (symbol == Symbol::INTCON) {
                sym_retract();
                sym_retract();
                getsym();
            } else {    // cant happen? a = --n;
                sym_retract();
                sym_retract();
                getsym();
                toPrint[toPrint.size() - 1] = (symbol == preOp) ? '+' : '-';
                minu = (symbol == Symbol::PLUS) ? -minu : minu;
                getsym();
            }
        }
    } else if (symbol == Symbol::LPARENT) {
        sureInt = true;
    }
    if (symbol == Symbol::CHARCON) {
        ret = BaseType::CHAR;
    } else if (symbol == Symbol::IDENFR) {
        if (symTable.contain(token, curLevel, true)) {
            SymbolC *symc = symTable.find(token, curLevel, true);
            if (((*symc).is_ret_func() || !(*symc).func)
                && (*symc).spec.baseType == BaseType::CHAR) {
                ret = BaseType::CHAR;
            }
        }
    }
    auto tmp = item();  // first item
    int itemCnt = 1;
    if (symbol == Symbol::PLUS || symbol == Symbol::MINU) { // item > 1
        if (tmp->singleChar) {
            iSym->singleChar = true;
            toPrint += to_string((int)tmp->name[0]);
        } else {
            toPrint += (tmp->interType == InterType::SYMC) ? "_" + tmp->name : tmp->name;    // 项
        }
        iSym->singleChar = false;
        iSym->baseType = BaseType::INT;
    } else {
        if (tmp->singleChar) {
            iSym->singleChar = true;
            toPrint += "\'" + tmp->name + "\'";
        } else {
            toPrint += (tmp->interType == InterType::SYMC) ? "_" + tmp->name : tmp->name;    // 项
        }
    }
    while (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
        if (++itemCnt > 2) {
            toPrint += '\n';
            toPrint += iSym->name + " = " + iSym->name;
            itemCnt = 2;
        }
        ret = BaseType::INT;
		Symbol preOp = symbol;
        toPrint += (symbol == Symbol::PLUS ? " + " : " - ");
        getsym();
		if (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
			toPrint[toPrint.size() - 2] = (symbol == preOp)? '+':'-';
			getsym();
		}
        auto tmp = item();
		if (tmp->singleChar) {
			toPrint += to_string(tmp->intcon);
		} else {
			toPrint += (tmp->interType == InterType::SYMC) ? "_" + tmp->name : tmp->name;
		}
    }
	if (itemCnt == 1) {	// A = B, 注意类型转化
		if (sureInt) {
			toPrint += " + 0";	// attend in operation
		}
	}
    sym_retract();
    sym_output();
    find_syn_part("<表达式>");
    getsym();
    if (itemCnt == 1) {
        tmp->baseType = sureInt ? BaseType::INT : ret;
        switch (tmp->interType) {
            case InterType::SYMC:
                tmp->name = "_" + tmp->name;
                if (minu) {
                    iSym->baseType = sureInt ? BaseType::INT : ret;
                    genInter.prt(iSym->name + " = -" + tmp->name + "\n");
                    return iSym;
                }
                break;
            case InterType::INTER:
                if (minu) {
                    iSym->baseType = sureInt ? BaseType::INT : ret;
                    genInter.prt(iSym->name + " = -" + tmp->name + "\n");
                    return iSym;
                }
                break;
            case InterType::INTCONST:
                tmp->name = minu ? "-" + tmp->name : tmp->name;
                break;
            case InterType::CHARCONST:
                if (minu) {
                    tmp->name = "-" + to_string((int)tmp->name[0]);
                    tmp->interType = InterType::INTCONST;
                } else {
                    tmp->name = "\'" + tmp->name + "\'";    // push '42'
                }
            default:
                break;
        }
        return tmp;
    }
    iSym->baseType = sureInt ? BaseType::INT : ret;
    genInter.prt(toPrint + "\n");
    return iSym;
}

vector<InterSym *> val_para_list(SymbolC *func) {
    vector<InterSym *> vec;
    int paraCnt = 0;
    if (symbol != Symbol::RPARENT) {
        paraCnt++;
        InterSym *exp = expression();
        vec.push_back(exp);
        if (paraCnt > (*func).args.size()) { ;
        } else if (exp->baseType != (*(*func).args.at(paraCnt - 1)).spec.baseType) {
            sym_retract();
            globalErr.catch_e(curLineNum, "e");
            getsym();
        }
        while (symbol == Symbol::COMMA) {
            getsym();
            paraCnt++;
            InterSym *exp = expression();
            vec.push_back(exp);
            if (paraCnt > (*func).args.size()) { ;
            } else if (exp->baseType != (*(*func).args.at(paraCnt - 1)).spec.baseType) {
                sym_retract();
                globalErr.catch_e(curLineNum, "e");
                getsym();
            }
        }
    }
    if (paraCnt != (*func).args.size()) {
        globalErr.catch_e(curLineNum, "d");
    }
    sym_retract();
    sym_output();
    find_syn_part("<值参数表>");
    getsym();
    return vec;
}

InterSym *r_f_stat(bool reportIdenNotFindError) {
    auto *iS = new InterSym;
    iS->interType = InterType::INTER;
    iS->name = "RET";

    InterSym *iSym = new InterSym();
    iSym->interType = InterType::INTER;
    iSym->set_tmp_name();
    string toPrint;
    assert_symbol_is(Symbol::IDENFR);
    SymbolC *fun = new SymbolC();
    if (symTable.contain(token, curLevel, true)) {
        fun = symTable.find(token, curLevel, true);
    } else if (reportIdenNotFindError) {
        globalErr.catch_e(curLineNum, "c");
    }
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    vector<InterSym *> paras_list = val_para_list(fun);
    for (int i = 0; i < paras_list.size(); i++) {
        toPrint += "push " + paras_list.at(i)->name + '\n';
    }
    toPrint += "call _" + fun->name + '\n';

    toPrint += iSym->name + " = RET\n";
    iSym->interType = InterType::INTER;
	iSym->baseType = fun->spec.baseType; // BaseType::INT;
    assert_symbol_is(Symbol::RPARENT);
    genInter.prt(toPrint);
    sym_output();
    find_syn_part("<有返回值函数调用语句>");
    getsym();
    return iSym;
}

void n_r_f_stat() {
    string toPrint = "";
    assert_symbol_is(Symbol::IDENFR);
    SymbolC *fun = new SymbolC();
    if (symTable.contain(token, curLevel, true)) {
        fun = symTable.find(token, curLevel, true);
    } else {
        globalErr.catch_e(curLineNum, "c");
    }
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    vector<InterSym *> paras_list = val_para_list(fun);
    for (int i = 0; i < paras_list.size(); i++) {
        toPrint += "push " + paras_list.at(i)->name + '\n';
    }
    toPrint += "call _" + fun->name + '\n';
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    genInter.prt(toPrint);
    find_syn_part("<无返回值函数调用语句>");
    getsym();
}

void condition_stat() {
    string toPrint = "";
    auto exp = expression();
    if (exp->baseType != BaseType::INT) {
        globalErr.catch_e(curLineNum, "f");
    }
    toPrint += exp->name;
    if (bool_relat_op()) {
        toPrint += " " + token + " ";
        getsym();
        auto exp = expression();
        if (exp->baseType != BaseType::INT) {
            globalErr.catch_e(curLineNum, "f");
        }
        toPrint += exp->name;
    }
    sym_retract();
    sym_output();
    genInter.prt(toPrint + "\n");
    find_syn_part("<条件>");
    getsym();
}

bool if_stat(bool returned) {
    bool haveRet = false;
    string lb1 = genInter.gen_label();
    assert_symbol_is(Symbol::IFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    condition_stat();
    assert_symbol_is(Symbol::RPARENT);
    genInter.prt("BZ " + lb1 + '\n');
    getsym();
    haveRet |= statement(returned);
    if (symbol == Symbol::ELSETK) {
        string lb2 = genInter.gen_label();
        genInter.prt("GOTO " + lb2 + '\n');
        genInter.prt(lb1 + " :\n");
        getsym();
        haveRet |= statement(returned);
        genInter.prt(lb2 + " :\n");
    } else {
        genInter.prt(lb1 + " :\n");
    }
    sym_retract();
    sym_output();
    find_syn_part("<条件语句>");
    getsym();
    return haveRet;
}

int step_len() {
    is_unsigned_int();
    int ret = number;
    find_syn_part("<步长>");
    getsym();
    return ret;
}

void loop_stat(bool returned) {
    string toPrint = "";
    string lb1 = genInter.gen_label_loop_start(), lb2;
    string idfr, idfr2, op;
    int stepLen;
    switch (symbol) {
        case Symbol::WHILETK:
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            toPrint += (lb1 + " :\n");
            genInter.prt(toPrint);
            toPrint = "";
            condition_stat();
            lb2 = genInter.gen_label_loop_end();
            toPrint += ("BZ " + lb2 + '\n');
            assert_symbol_is(Symbol::RPARENT);
            getsym();
            genInter.prt(toPrint);
            toPrint = "";
            statement(returned);
            toPrint += ("GOTO " + lb1 + "\n");
            toPrint += (lb2 + " :\n");
            break;
        case Symbol::DOTK:
            getsym();
            toPrint += (lb1 + " :\n");
            genInter.prt(toPrint);
            toPrint = "";
            statement(returned);
            assert_symbol_is(Symbol::WHILETK);
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            condition_stat();
            assert_symbol_is(Symbol::RPARENT);
            toPrint += ("BNZ " + lb1 + '\n');
            getsym();
            break;
        case Symbol::FORTK:
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            toPrint += ("_" + token + " = ");
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            toPrint += (expression()->name + "\n");
            assert_symbol_is(Symbol::SEMICN);
            toPrint += (lb1 + " :\n");
            getsym();
            genInter.prt(toPrint);
            toPrint = "";
            condition_stat();
            lb2 = genInter.gen_label_loop_end();
            toPrint += ("BZ " + lb2 + "\n");
            assert_symbol_is(Symbol::SEMICN);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            idfr = token;
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            idfr2 = token;
            getsym();
            if (symbol != Symbol::PLUS && symbol != Symbol::MINU) 
                error();
            op = token;
            getsym();
            stepLen = step_len();
            assert_symbol_is(Symbol::RPARENT);
            getsym();
            genInter.prt(toPrint);
            toPrint = "";
            statement(returned);
            toPrint += ("_" + idfr + " = _" + idfr2 + " " + op + " " + to_string(stepLen) + "\n");
            toPrint += ("GOTO " + lb1 + '\n');
            toPrint += (lb2 + " :\n");
            break;
        default:
            break;
    }
    sym_retract();
    genInter.prt(toPrint);
    sym_output();
    find_syn_part("<循环语句>");
    getsym();
}

void val_stat() {
    string toPrint = "";
    assert_symbol_is(Symbol::IDENFR);
    toPrint += "_" + token;
    if (!symTable.contain(token, curLevel, true)) {
        globalErr.catch_e(curLineNum, "c");
    } else if (symTable.find(token, curLevel, true)->spec.isConst) {
        globalErr.catch_e(curLineNum, "j");
    }
    getsym();
    if (symbol == Symbol::LBRACK) {    // array
        getsym();
        InterSym *exp = expression();
        if (exp->baseType != BaseType::INT) {
            globalErr.catch_e(curLineNum, "i");
        }
        toPrint += "[" + exp->name + "]";
        assert_symbol_is(Symbol::RBRACK);
        getsym();
    }
    assert_symbol_is(Symbol::ASSIGN);
    getsym();
    toPrint += " = " + expression()->name + "\n";
    sym_retract();
    genInter.prt(toPrint);
    sym_output();
    find_syn_part("<赋值语句>");
    getsym();
}

void scan_stat() {
    string toPrint = "";
    assert_symbol_is(Symbol::SCANFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    assert_symbol_is(Symbol::IDENFR);
    toPrint += "push _" + token + "\n";
    if (!symTable.contain(token, curLevel, true)) {
        globalErr.catch_e(curLineNum, "c");
    }
    getsym();
    while (symbol == Symbol::COMMA) {
        getsym();
        assert_symbol_is(Symbol::IDENFR);
        if (!symTable.contain(token, curLevel, true)) {
            globalErr.catch_e(curLineNum, "c");
        }
        toPrint += "push _" + token + "\n";
        getsym();
    }
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    genInter.prt(toPrint + "SCAN\n");
    find_syn_part("<读语句>");
    getsym();
}

void prt_stat() {
    string toPrint = "";
    assert_symbol_is(Symbol::PRINTFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    if (symbol == Symbol::STRCON) {
        sym_output();
        static int strCnt = 0;
        string strName = "$str" + to_string(strCnt++);
        SymbolC *strSym = new SymbolC(strName, curLevel, curLineNum, *new Specifier(true, BaseType::STRING));
        strSym->strVal = token;
        symTable.add(*strSym, get_out_level());
        find_syn_part("<字符串>");
        //toPrint += "push \"" + token + "\"\n";
        toPrint += "push " + strName + "\n";
        getsym();
        if (symbol == Symbol::COMMA) {
            getsym();
            toPrint += "push " + expression()->name + "\n";
        }
    } else {
        toPrint += "push " + expression()->name + "\n";
    }
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    genInter.prt(toPrint + "PRINT\n");
    find_syn_part("<写语句>");
    getsym();
}

bool ret_stat(bool returned) {
    string toPrint = "";
    bool forReturnFStat = false;
    assert_symbol_is(Symbol::RETURNTK);
    toPrint += "ret";
    getsym();
    if (symbol == Symbol::LPARENT) {
        forReturnFStat = true;
        getsym();
        InterSym *exp = expression();
        if (exp->baseType != returnStatType && returned) {
            globalErr.catch_e(curLineNum, "h");
        }
        toPrint += " " + exp->name;
        assert_symbol_is(Symbol::RPARENT);
        getsym();
    }
    sym_retract();
    genInter.prt(toPrint + "\n");
    sym_output();
    find_syn_part("<返回语句>");
    getsym();
    return forReturnFStat;
}

bool statement(bool returned) {
    bool haveRetStat = false;
    bool forReturnFStat = false;
    bool isret;
    if (symbol != Symbol::RBRACE) {
        switch (symbol) {
            case Symbol::IFTK:
                haveRetStat |= if_stat(returned);
                break;
            case Symbol::WHILETK:
            case Symbol::FORTK:
            case Symbol::DOTK:
                loop_stat(returned);
                break;
            case Symbol::LBRACE:    // '{' for, if ... a new area
                getsym();
                //gen_new_level();
                haveRetStat |= stat_list(returned);
                assert_symbol_is(Symbol::RBRACE);
                //level_retract();
                getsym();
                break;
            case Symbol::IDENFR:
                isret = (*symTable.find(token, curLevel, true)).is_ret_func();
                getsym();
                if (symbol == Symbol::LPARENT) {
                    sym_retract();
                    sym_retract();
                    getsym();
                    if (isret) {
                        r_f_stat(true);
                    } else {
                        n_r_f_stat();
                    }
                } else {
                    sym_retract();
                    sym_retract();
                    getsym();
                    val_stat();
                }
                assert_symbol_is(Symbol::SEMICN);
                getsym();
                break;
            case Symbol::SCANFTK:
                scan_stat();
                assert_symbol_is(Symbol::SEMICN);
                getsym();
                break;
            case Symbol::PRINTFTK:
                prt_stat();
                assert_symbol_is(Symbol::SEMICN);
                getsym();
                break;
            case Symbol::RETURNTK:
                haveRetStat = true;
                forReturnFStat = ret_stat(returned);
                if (!returned && forReturnFStat) {
                    globalErr.catch_e(curLineNum, "g");
                }
                if (returned && !forReturnFStat) {
                    globalErr.catch_e(curLineNum, "h");
                }
                assert_symbol_is(Symbol::SEMICN);
                getsym();
                break;
            case Symbol::SEMICN:
                // null state
                getsym();
                break;
            default:
                error("Error: illegal statement in statelist, dead loop");
                break;
        }
    }
    sym_retract();
    sym_output();
    find_syn_part("<语句>");
    getsym();
    return haveRetStat;
}

bool stat_list(bool returned) {
    bool b = false;
    while (symbol != Symbol::RBRACE) {
        b |= statement(returned);
    }
    sym_retract();
    sym_output();
    find_syn_part("<语句列>");
    getsym();
    return b;
}

bool comp_stat(bool returned) {
    bool b = false;
    if (symbol == Symbol::CONSTTK) {
        const_description();
    }
    if (bool_type_iden()) {
        var_description();
    }
    b = stat_list(returned);
    find_syn_part("<复合语句>");
    return b;
}

SymbolC *def_head() {
    // must be returned
    if (!bool_type_iden()) error();
    Specifier spc;
    sbtype2bstype(symbol, &spc);
    getsym();
    assert_symbol_is(Symbol::IDENFR);
    SymbolC syc(token, curLevel, curLineNum, spc);
    sym_output();
    find_syn_part("<声明头部>");
    SymbolC *ret = symTable.add(syc, get_out_level());
    getsym();
    return ret;
}

void param_list(SymbolC *sbc) {
    vector<SymbolC *> args;
    if (symbol != Symbol::RPARENT) {    // not void param
        if (!bool_type_iden()) error();
        Specifier spc;
        sbtype2bstype(symbol, &spc);
        getsym();
        assert_symbol_is(Symbol::IDENFR);
        SymbolC *arg = new SymbolC(token, curLevel, curLineNum, spc);
        args.push_back(arg);
        symTable.add(*arg, curLevel);
        getsym();
        while (symbol == Symbol::COMMA) {
            getsym();
            if (!bool_type_iden()) error();
            Specifier spc;
            sbtype2bstype(symbol, &spc);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            SymbolC *arg = new SymbolC(token, curLevel, curLineNum, spc);
            args.push_back(arg);
            symTable.add(*arg, curLevel);
            getsym();
        }
    }
    (*sbc).args = args;
    sym_retract(); // back )
    sym_output();
    find_syn_part("<参数表>");
    getsym();
}

void returned_func_def() {
    SymbolC *sbc = def_head();
    string funcLabel = genInter.gen_func_label(sbc->name);
    string endLabel = genInter.gen_func_end(sbc->name);
    //genInter.prt("GOTO\t" + endLabel + "\n");
    genInter.prt(funcLabel + " :\n");
    (*sbc).func = true;
    gen_new_level();
    (*sbc).inLevel = curLevel;
    assert_symbol_is(Symbol::LPARENT);
    sym_output();
    getsym();
    param_list(sbc);
    assert_symbol_is(Symbol::RPARENT);
    if (!sbc->duplicate) {
        genInter.find_func_def(*sbc);    // write func head to inter file
    }
    getsym();
    assert_symbol_is(Symbol::LBRACE);    // '{'
    getsym();
    returnStatType = (*sbc).spec.baseType;
    if (!comp_stat(true)) {
        globalErr.catch_e(curLineNum, "h");
    }
    assert_symbol_is(Symbol::RBRACE);
    sym_output();
    find_syn_part("<有返回值函数定义>");
    getsym();
    level_retract();
    genInter.prt(endLabel + " :\n");
}

void un_returned_func_def() {
    assert_symbol_is(Symbol::VOIDTK);
    Specifier spc;
    spc.baseType = BaseType::VOID;
    getsym();
    assert_symbol_is(Symbol::IDENFR);
    SymbolC sbc(token, curLevel, curLineNum, spc);
    string funcLabel = genInter.gen_func_label(sbc.name);
    string endLabel = genInter.gen_func_end(sbc.name);
    //genInter.prt("GOTO\t" + endLabel + "\n");
    genInter.prt(funcLabel + " :\n");
    sbc.func = true;
    gen_new_level();
    sbc.inLevel = curLevel;
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    sym_output();
    getsym();
    param_list(&sbc);
    assert_symbol_is(Symbol::RPARENT);
    if (!symTable.add(sbc, get_out_level())->duplicate) {
        genInter.find_func_def(sbc);
    }
    getsym();
    assert_symbol_is(Symbol::LBRACE);    // '{'
    getsym();
    comp_stat(false);
    assert_symbol_is(Symbol::RBRACE);
    sym_output();
    find_syn_part("<无返回值函数定义>");
    getsym();
    level_retract();
    genInter.prt(endLabel + " :\n");
}

void main_f() {
    assert_symbol_is(Symbol::VOIDTK);
    Specifier spc;
    spc.baseType = BaseType::VOID;
    getsym();
    assert_symbol_is(Symbol::MAINTK);
    SymbolC sbc(token, curLevel, curLineNum, spc);
    sbc.func = true;
    gen_new_level();
    sbc.inLevel = curLevel;
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    assert_symbol_is(Symbol::RPARENT);
    if (!symTable.add(sbc, get_out_level())->duplicate) {
        genInter.find_func_def(sbc);
    }
    getsym();
    assert_symbol_is(Symbol::LBRACE);    // '{'
    getsym();
    comp_stat(false);
    assert_symbol_is(Symbol::RBRACE);
    sym_output();
    find_syn_part("<主函数>");
    level_retract();
    if (getsym() != 1) error();    // should end program
}

void program() {
    gen_new_level();
    if (symbol == Symbol::CONSTTK) {    // const variable description
        const_description();
    }
    if (symbol == Symbol::INTTK || symbol == Symbol::CHARTK || symbol == Symbol::VOIDTK) {
        if (symbol == Symbol::INTTK || symbol == Symbol::CHARTK) {
            var_description();
        }
        while (symbol != Symbol::EOFF) {
            if (bool_type_iden()) {
                returned_func_def();
            } else {
                getsym();
                if (token == "main") {
                    sym_retract();
                    sym_retract();
                    getsym();
                    main_f();
                    break;
                } else {
                    sym_retract();
                    sym_retract();
                    getsym();
                    un_returned_func_def();
                }
            }
        }
    } else {
        error();
    }
    genInter.prt("#EOF :");
    find_syn_part("<程序>");
}
