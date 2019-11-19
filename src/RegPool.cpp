#include <iostream>
#include "RegPool.h"

void RegPool::err(string s) {
    cerr << "error in RegPool " << s << endl;
}

RegPool::RegPool() {
    int cnt = 0;
    for (auto &i : v) {
        i = new Register(RegType::V, cnt++);
    }
    cnt = 0;
    for (auto &i : a) {
        i = new Register(RegType::A, cnt++);
    }
    cnt = 0;
    for (auto &i : t) {
        i = new Register(RegType::T, cnt++);
    }
    cnt = 0;
    for (auto &i : s) {
        i = new Register(RegType::S, cnt++);
    }
}

Register* RegPool::apply(RegType regType) {
    switch (regType) {
        case RegType::ZERO:
            break;
        case RegType::AT:
            break;
        case RegType::V:
            break;
        case RegType::A:
            break;
        case RegType::T:
            for (auto &i : t) {
                if (i->idle) {
                    i->idle = false;
                    return i;
                }
            }
            break;
        case RegType::S:
            break;
        case RegType::GP:
            break;
        case RegType::SP:
            break;
        case RegType::FP:
            break;
        case RegType::RA:
            break;
    }
    err("in apply");
    exit(-7);
}

void RegPool::release(Register* reg) {
    reg->idle = true;
    for(auto it = sym2reg.begin(); it != sym2reg.end();) {
        if (it->second == reg) {
            it = sym2reg.erase(it);
        } else { ++it; }
    }
}

void RegPool::release(SymbolC* symbolC) {
    for(auto it = sym2reg.begin(); it != sym2reg.end(); ) {
        if (it->first == symbolC) {
            it->second->idle = true;
            it = sym2reg.erase(it);
        } else { ++it; }
    }
}

Register* RegPool::findReg(SymbolC* symbolC) {
    return sym2reg.find(symbolC)->second;
}

bool RegPool::isSymInReg(SymbolC *symbolC) {
    return sym2reg.find(symbolC) != sym2reg.end();

}

void RegPool::bindSym2Reg(SymbolC *symbolC, Register *reg) {
    sym2reg.insert(pair<SymbolC *, Register *>(symbolC, reg));
}
