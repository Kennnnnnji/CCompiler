#pragma once

#include <string>
#include "Register.h"
#include "table.h"
using namespace std;

class RegPool {
public:
    static int status[32];
    Register *zero = new Register(RegType::ZERO, 0);
    Register *at = new Register(RegType::AT, 0);
    Register* v[2]{};
    Register* a[4]{};
    Register* t[10]{};    // t0-t7, s0-s7, t8-t9
    Register* s[8]{};
    Register* gp = new Register(RegType::GP, 0);
    Register* sp = new Register(RegType::SP, 0);
    Register* fp = new Register(RegType::FP, 0);
    Register* ra = new Register(RegType::RA, 0);
    unordered_map<SymbolC*, Register*> sym2reg;

    RegPool();
    Register* apply(RegType regType); // apply for a reg
    Register* apply(RegType regType, int id); // apply for a reg
    Register* apply_auto(SymbolC* symc); // t for @tmp, s for local var

    Register* bindSym2Reg(SymbolC* symbolC, Register* reg);
    bool isSymInReg(SymbolC* symbolC);

    void err(string s);
    void erase(Register* reg);
    void release(Register* reg);
    void release(SymbolC *symbolC);
	void releaseAll(RegType regType);
	void releaseAll();
	void releaseAllFull(bool exceptTmpVar);
    void release_save_gp(SymbolC* symbolC);
    void release_save_gp(Register* reg);
    void release_all_save_gp();
    void release_all_except_s();
    void release_all_except_s_tmp();

    Register *findReg(SymbolC *symbolC);
};

