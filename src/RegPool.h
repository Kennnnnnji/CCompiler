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
    void release(Register* reg);
    void bindSym2Reg(SymbolC* symbolC, Register* reg);
    bool isSymInReg(SymbolC* symbolC);

    void err(string s);

    void release(SymbolC *symbolC);

    Register *findReg(SymbolC *symbolC);
};

