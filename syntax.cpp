#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <set>
#include "header.h"

using namespace std;

int curLevel = 0;
BaseType returnStatType;

BaseType expression();

void r_f_stat(bool reportIdenNotFindError);

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
}

static void error(string s) {
    cerr << s << " in syntax.cpp, cur = " << cur << ", curChar = " << curChar << endl;
    cerr << src + cur << endl;
}

void find_syn_part(string part) {
    outfile << part << endl;
}

bool assert_symbol_is(Symbol sy) {
    bool ret = true;
    if (symbol != sy) {
        ret = false;
        error();
        switch (sy) {
            case Symbol::SEMICN:
                globalErr.catch_e(line - sub_lines(), "k");
                sym_retract();
                break;
            case Symbol::RPARENT:
                globalErr.catch_e(line, "l");
                sym_retract();
                break;
            case Symbol::RBRACK:
                globalErr.catch_e(line, "m");
                sym_retract();
                break;
            case Symbol::WHILETK:
                globalErr.catch_e(line, "n");
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
        getsym();
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
            SymbolC symC(token, curLevel, line, spc);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            if (!is_int()) {
                globalErr.catch_e(line, "o");
            }
            symTable.add(symC, get_out_level());
            getsym();
        } while (symbol == Symbol::COMMA);
    } else if (token == "char") {
        do {
            Specifier spc(true, BaseType::CHAR);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            SymbolC symC(token, curLevel, line, spc);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            if (!assert_symbol_is(Symbol::CHARCON)) {
                globalErr.catch_e(line, "o");
            }
            symTable.add(symC, get_out_level());
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
        SymbolC sbc(token, curLevel, line, spc);
        symTable.add(sbc, get_out_level());
        getsym();
        if (token.at(0) == '[') {
            getsym();
            is_unsigned_int();
            getsym();
            if (token.at(0) != ']') error();
            getsym();
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
        SymbolC sbc(token, curLevel, line, spc);
        getsym();
        // var, else ret_func
        if (token.at(0) == '[' || token.at(0) == ',' || token.at(0) == ';') {
            if (token.at(0) == '[') {
                getsym();
                is_unsigned_int();
                getsym();
                if (token.at(0) != ']') error();
                getsym();
            }
            symTable.add(sbc, get_out_level());
            var_definition(varsym);
            if (token.at(0) != ';') error();
            getsym();
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

void factor() {
    if (symbol == Symbol::IDENFR) {
        if (!symTable.contain(token, curLevel, true)) {
            globalErr.catch_e(line, "c");
        }
        getsym();
        if (symbol == Symbol::LBRACK) {    // '['
            getsym();
            if (expression() != BaseType::INT) {
                globalErr.catch_e(line, "i");
            }
            assert_symbol_is(Symbol::RBRACK);
            getsym();
        } else if (symbol == Symbol::LPARENT) {
            sym_retract();
            sym_retract();
            getsym();
            bool returned = (*symTable.find(token, 0, true)).is_ret_func();
            if (!returned) {
                // use a void func as a factor
                // globalErr.catch_e(line, "e");
                n_r_f_stat();
            } else {
                r_f_stat(false);
            }
        }
    } else if (symbol == Symbol::LPARENT) {
        getsym();
        expression();
        assert_symbol_is(Symbol::RPARENT);
        getsym();
    } else if (symbol == Symbol::PLUS || symbol == Symbol::MINU
               || symbol == Symbol::INTCON) {
        is_int();
        getsym();
    } else if (symbol == Symbol::CHARCON) {
        getsym();
    } else {
        error();
    }
    sym_retract();
    sym_output();
    find_syn_part("<因子>");
    getsym();
}

void item() {
    factor();
    while (symbol == Symbol::MULT || symbol == Symbol::DIV) {
        getsym();
        factor();
    }
    sym_retract();
    sym_output();
    find_syn_part("<项>");
    getsym();
}

BaseType expression() {
    BaseType ret = BaseType::INT;
    bool okInt = false;
    if (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
        getsym();
        okInt = true;
    } else if (symbol == Symbol::LPARENT) {
        okInt = true;
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
    item();
    while (symbol == Symbol::PLUS || symbol == Symbol::MINU) {
        ret = BaseType::INT;
        getsym();
        item();
    }
    sym_retract();
    sym_output();
    find_syn_part("<表达式>");
    getsym();
    return okInt ? BaseType::INT : ret;
}

void val_para_list(SymbolC *func) {
    int paraCnt = 0;
    if (symbol != Symbol::RPARENT) {
        paraCnt++;
        if (paraCnt > (*func).args.size()) {
            expression();
        } else if (expression() != (*(*func).args.at(paraCnt - 1)).spec.baseType) {
            sym_retract();
            globalErr.catch_e(line, "e");
            getsym();
        }
        while (symbol == Symbol::COMMA) {
            getsym();
            paraCnt++;
            if (paraCnt > (*func).args.size()) {
                expression();
            } else if (expression() != (*(*func).args.at(paraCnt - 1)).spec.baseType) {
                sym_retract();
                globalErr.catch_e(line, "e");
                getsym();
            }
        }
    }
    if (paraCnt != (*func).args.size()) {
        globalErr.catch_e(line, "d");
    }
    sym_retract();
    sym_output();
    find_syn_part("<值参数表>");
    getsym();
}

void r_f_stat(bool reportIdenNotFindError) {
    assert_symbol_is(Symbol::IDENFR);
    SymbolC *fun = new SymbolC();
    if (symTable.contain(token, curLevel, true)) {
        fun = symTable.find(token, curLevel, true);
    } else if (reportIdenNotFindError) {
        globalErr.catch_e(line, "c");
    }
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    val_para_list(fun);
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    find_syn_part("<有返回值函数调用语句>");
    getsym();
}

void n_r_f_stat() {
    assert_symbol_is(Symbol::IDENFR);
    SymbolC *fun = new SymbolC();
    if (symTable.contain(token, curLevel, true)) {
        fun = symTable.find(token, curLevel, true);
    } else {
        globalErr.catch_e(line, "c");
    }
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    val_para_list(fun);
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    find_syn_part("<无返回值函数调用语句>");
    getsym();
}

void condition() {
    if (expression() != BaseType::INT) {
        globalErr.catch_e(line, "f");
    }
    if (bool_relat_op()) {
        getsym();
        if (expression() != BaseType::INT) {
            globalErr.catch_e(line, "f");
        }
    }
    sym_retract();
    sym_output();
    find_syn_part("<条件>");
    getsym();
}

void if_stat(bool returned) {
    assert_symbol_is(Symbol::IFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    condition();
    assert_symbol_is(Symbol::RPARENT);
    getsym();
    statement(returned);
    if (symbol == Symbol::ELSETK) {
        getsym();
        statement(returned);
    }
    sym_retract();
    sym_output();
    find_syn_part("<条件语句>");
    getsym();
}

void step_len() {
    is_unsigned_int();
    find_syn_part("<步长>");
    getsym();
}

void loop_stat(bool returned) {
    switch (symbol) {
        case Symbol::WHILETK:
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            condition();
            assert_symbol_is(Symbol::RPARENT);
            getsym();
            statement(returned);
            break;
        case Symbol::DOTK:
            getsym();
            statement(returned);
            assert_symbol_is(Symbol::WHILETK);
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            condition();
            assert_symbol_is(Symbol::RPARENT);
            getsym();
            break;
        case Symbol::FORTK:
            getsym();
            assert_symbol_is(Symbol::LPARENT);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            expression();
            assert_symbol_is(Symbol::SEMICN);
            getsym();
            condition();
            assert_symbol_is(Symbol::SEMICN);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            getsym();
            assert_symbol_is(Symbol::ASSIGN);
            getsym();
            assert_symbol_is(Symbol::IDENFR);
            getsym();
            if (symbol != Symbol::PLUS && symbol != Symbol::MINU) error();
            getsym();
            step_len();
            assert_symbol_is(Symbol::RPARENT);
            getsym();
            statement(returned);
            break;
        default:
            break;
    }
    sym_retract();
    sym_output();
    find_syn_part("<循环语句>");
    getsym();
}

void val_stat() {
    assert_symbol_is(Symbol::IDENFR);
    if (!symTable.contain(token, curLevel, true)) {
        globalErr.catch_e(line, "c");
    } else if (symTable.find(token, curLevel, true)->spec.isConst) {
        globalErr.catch_e(line, "j");
    }
    getsym();
    if (symbol == Symbol::LBRACK) {
        getsym();
        if (expression() != BaseType::INT) {
            globalErr.catch_e(line, "i");
        }
        assert_symbol_is(Symbol::RBRACK);
        getsym();
    }
    assert_symbol_is(Symbol::ASSIGN);
    getsym();
    expression();
    sym_retract();
    sym_output();
    find_syn_part("<赋值语句>");
    getsym();
}

void scan_stat() {
    assert_symbol_is(Symbol::SCANFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    assert_symbol_is(Symbol::IDENFR);
    if (!symTable.contain(token, curLevel, true)) {
        globalErr.catch_e(line, "c");
    }
    getsym();
    while (symbol == Symbol::COMMA) {
        getsym();
        assert_symbol_is(Symbol::IDENFR);
        if (!symTable.contain(token, curLevel, true)) {
            globalErr.catch_e(line, "c");
        }
        getsym();
    }
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    find_syn_part("<读语句>");
    getsym();
}

void prt_stat() {
    assert_symbol_is(Symbol::PRINTFTK);
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    if (symbol == Symbol::STRCON) {
        sym_output();
        find_syn_part("<字符串>");
        getsym();
        if (symbol == Symbol::COMMA) {
            getsym();
            expression();
        }
    } else {
        expression();
    }
    assert_symbol_is(Symbol::RPARENT);
    sym_output();
    find_syn_part("<写语句>");
    getsym();
}

bool ret_stat(bool returned) {
    bool forReturnFStat = false;
    assert_symbol_is(Symbol::RETURNTK);
    getsym();
    if (symbol == Symbol::LPARENT) {
        forReturnFStat = true;
        getsym();
        if (expression() != returnStatType && returned) {
            globalErr.catch_e(line, "h");
        }
        assert_symbol_is(Symbol::RPARENT);
        getsym();
    }
    sym_retract();
    sym_output();
    find_syn_part("<返回语句>");
    getsym();
    return forReturnFStat;
}

bool statement(bool returned) {
    bool haveRetStat = false;
    bool forReturnFStat;
    bool isret;
    if (symbol != Symbol::RBRACE) {
        switch (symbol) {
            case Symbol::IFTK:
                if_stat(returned);
                break;
            case Symbol::WHILETK:
            case Symbol::FORTK:
            case Symbol::DOTK:
                loop_stat(returned);
                break;
            case Symbol::LBRACE:    // '{'
                getsym();
                gen_new_level();
                stat_list(returned);
                assert_symbol_is(Symbol::RBRACE);
                level_retract();
                getsym();
                break;
            case Symbol::IDENFR:
                isret = (*symTable.find(token, 0, true)).is_ret_func();
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
                    globalErr.catch_e(line, "g");
                }
                if (returned && !forReturnFStat) {
                    globalErr.catch_e(line, "h");
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
    SymbolC syc(token, curLevel, line, spc);
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
        SymbolC *arg = new SymbolC(token, curLevel, line, spc);
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
            SymbolC *arg = new SymbolC(token, curLevel, line, spc);
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
    (*sbc).func = true;
    gen_new_level();
    assert_symbol_is(Symbol::LPARENT);
    sym_output();
    getsym();
    param_list(sbc);
    assert_symbol_is(Symbol::RPARENT);
    getsym();
    assert_symbol_is(Symbol::LBRACE);    // '{'
    getsym();
    returnStatType = (*sbc).spec.baseType;
    if (!comp_stat(true)) {
        globalErr.catch_e(line, "h");
    }
    assert_symbol_is(Symbol::RBRACE);
    sym_output();
    find_syn_part("<有返回值函数定义>");
    getsym();
    level_retract();
}

void un_returned_func_def() {
    assert_symbol_is(Symbol::VOIDTK);
    Specifier spc;
    spc.baseType = BaseType::VOID;
    getsym();
    assert_symbol_is(Symbol::IDENFR);
    SymbolC sbc(token, curLevel, line, spc);
    sbc.func = true;
    gen_new_level();
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    sym_output();
    getsym();
    param_list(&sbc);
    assert_symbol_is(Symbol::RPARENT);
    symTable.add(sbc, get_out_level());
    getsym();
    assert_symbol_is(Symbol::LBRACE);    // '{'
    getsym();
    comp_stat(false);
    assert_symbol_is(Symbol::RBRACE);
    sym_output();
    find_syn_part("<无返回值函数定义>");
    getsym();
    level_retract();
}

void main_f() {
    assert_symbol_is(Symbol::VOIDTK);
    getsym();
    assert_symbol_is(Symbol::MAINTK);
    gen_new_level();
    getsym();
    assert_symbol_is(Symbol::LPARENT);
    getsym();
    assert_symbol_is(Symbol::RPARENT);
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
    find_syn_part("<程序>");
}
