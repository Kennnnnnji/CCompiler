#include <iostream>
#include "RegPool.h"
#include "header.h"

void RegPool::err(string s) {
    cerr << "error in RegPool " << s << endl;
}

void RegPool::erase(Register* reg) {
	if (reg->protect == true && reg == s[1]) {
		cout << "";
	}
	reg->protect = false;
	reg->idle = true;
	for (auto it = sym2reg.begin(); it != sym2reg.end();) {
		if (it->second == reg) {
			it = sym2reg.erase(it);
		} else { ++it; }
	}
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
        case RegType::V:
			for (auto& i : v) {
				if (i->idle) {
					i->idle = false;
					return i;
				}
			}
			releaseAll(RegType::V);
			v[0]->idle = false;
			return v[0];
            break;
        case RegType::T:
            for (auto &i : t) {
                if (i->idle) {
                    i->idle = false;
                    return i;
                }
            }
			releaseAll(RegType::T);
			for (auto& i : t) {
				if (i->idle) {
					i->idle = false;
					return i;
				}
			}
			err("cannot find idle reg T");
			return t[0];
			break;
		case RegType::S:

			return apply(RegType::T);
			for (auto& i : s) {
				if (i->idle) {
					i->idle = false;
					return i;
				}
			}
			/*releaseAll(RegType::S);
			for (auto& i : s) {
				if (i->idle) {
					i->idle = false;
					return i;
				}
			}
			err("cannot find idle reg S");
			return s[0];*/
			break;
		default:
			err("unrecognized apply type.");
			break;
    }
    err("in apply");
    exit(-7);
}

Register* RegPool::apply(RegType regType, int id) {
	switch (regType) {
		case RegType::V:
			release(v[id]);
			return v[id];
		case RegType::T:
			release(t[id]);
			return t[id];
		case RegType::A:
			release(a[id]);
			return a[id];
		case RegType::S:
			release(s[id]);
			s[id]->idle = false;
			return s[id];
		default:
			err("unrecognized apply type 2");
			break;
	}
	return nullptr;
}

Register* RegPool::apply_auto(SymbolC* symc) {
	if (symc->name[0] == '@' || symc->arr || symc->level == 0) {
		return apply(RegType::T);
	} else if (symc->isPara && symc->paraId < 3) {
		Register* reg = apply(RegType::A, symc->paraId + 1);
		reg->protect = true;
		return reg;
	} else if (symc->isVar && symc->varId < 8) {
		Register* reg = apply(RegType::S, symc->varId);
		reg->protect = true;
		return reg;
	} else if (symc->isVar && symc->varId >= 8) {
		return apply(RegType::T);
	} else {
		Register* reg = apply(RegType::T);
		return reg;
	}
}

void RegPool::release(Register* reg) {
	if (reg->protect) {
		return;
	}
    for(auto it = sym2reg.begin(); it != sym2reg.end();) {
        if (it->second == reg) {
			if (!reg->idle && !it->first->arr && it->first->name.find("@para_") == std::string::npos) {	// not consider arr
				genMips.prt("sw\t" + reg->name + "\t-" + to_string(it->first->addr) + (it->first->level < 1 ? "($gp)" : "($sp)") + "\n");
			}
            it = sym2reg.erase(it);
        } else { ++it; }
    }
	reg->idle = true;
}

void RegPool::release(SymbolC* symbolC) {
    for(auto it = sym2reg.begin(); it != sym2reg.end(); ) {
        if (it->first == symbolC) {
			if (it->second->protect) {
				return;
			}
			if (!it->second->idle && !it->first->arr && it->first->name.find("@para_") == std::string::npos)
				genMips.prt("sw\t" + it->second->name + "\t-" + to_string(it->first->addr) + (it->first->level < 1 ? "($gp)" : "($sp)") + "\n");
            it->second->idle = true;
            it = sym2reg.erase(it);
        } else { ++it; }
    }
}

void RegPool::release_save_gp(Register* reg) {
	if (reg->protect) {
		return;
	}
	for (auto it = sym2reg.begin(); it != sym2reg.end();) {
		if (it->second == reg) {
			if (!reg->idle && !it->first->arr && it->first->level < 1) {	// not consider arr
				genMips.prt("sw\t" + reg->name + "\t-" + to_string(it->first->addr) + "($gp)" + "\n");
			}
			it = sym2reg.erase(it);
		} else { ++it; }
	}
	reg->idle = true;
 }

void RegPool::release_save_gp(SymbolC* symbolC) {
	for (auto it = sym2reg.begin(); it != sym2reg.end(); ) {
		if (it->first == symbolC) {
			if (it->second->protect) {
				return;
			}
			if (!it->second->idle && !it->first->arr && symbolC->level < 1)
				genMips.prt("sw\t" + it->second->name + "\t-" + to_string(it->first->addr) + "($gp)" + "\n");
			it->second->idle = true;
			it = sym2reg.erase(it);
		} else { ++it; }
	}
}

void RegPool::releaseAll(RegType regType) {
	switch (regType) {
		case RegType::ZERO:
			release(zero);
			break;
		case RegType::AT:
			release(at);
			break;
		case RegType::V:
			release(v[0]);
			release(v[1]);
			break;
		case RegType::A:
			for (auto& ar : a) {
				release(ar);
			}
			break;
		case RegType::T:
			for (auto& r : t) {
				release(r);
			}
			break;
		case RegType::S:
			for (auto& r : s) {
				release(r);
			}
			break;
		case RegType::GP:
			release(gp);
			break;
		case RegType::SP:
			release(sp);
			break;
		case RegType::FP:
			release(fp);
			break;
		case RegType::RA:
			release(ra);
			break;
		default:
			break;
	}
}

// need manually cancel S's protect
void RegPool::releaseAllFull(bool exceptTmpVar) {
	for (auto& i : sym2reg) {
		if (i.second->type != RegType::S)
			i.second->protect = false;
	}
	releaseAll(RegType::A);
	releaseAll(RegType::V);
	if (!exceptTmpVar) { releaseAll(RegType::T); }
	else {
		// 对每个出现在多个block中的tmp, sw
		for (auto it = sym2reg.begin(); it != sym2reg.end();) {
			if (it->first->name.front() == '@' && opt.find_var_in_n_blocks(it->first->name) > 1
					|| it->first->isVar && it->first->varId >= 8) {
				auto reg = it->second;
				++it;
				release(reg);
				continue;
			}
			++it;
		}
	}
	releaseAll(RegType::S);
}

void RegPool::releaseAll() {
	releaseAll(RegType::A);
	releaseAll(RegType::V);
	releaseAll(RegType::T);
	releaseAll(RegType::S);
}

void RegPool::release_all_except_s_tmp() {
	releaseAll(RegType::A);
	releaseAll(RegType::V);
	//releaseAll(RegType::T);
	// 对每个出现在多个block中的tmp, sw
	for (auto it = sym2reg.begin(); it != sym2reg.end();) {
		if (it->first->name.front() == '@' && opt.find_var_in_n_blocks(it->first->name) > 1
			|| it->first->isVar && it->first->varId >= 8) {
			auto reg = it->second;
			++it;
			release(reg);
			continue;
		}
		++it;
	}
}

void RegPool::release_all_except_s() {
	releaseAll(RegType::A);
	releaseAll(RegType::V);
	releaseAll(RegType::T);
}

void RegPool::release_all_save_gp() {
	for (auto& reg : t) {
		release_save_gp(reg);
	}
	for (auto& reg : s) {
		release_save_gp(reg);
	}
	for (auto& reg : a) {
		release_save_gp(reg);
	}
}

Register* RegPool::findReg(SymbolC* symbolC) {
	if (!isSymInReg(symbolC)) {
 		err("not in pool: " + symbolC->name + ", reapplied.");
		Register* reg = apply(RegType::T);
		bindSym2Reg(symbolC, reg);
		genMips.prt("lw\t" + reg->name + "\t-" + to_string(symbolC->addr) + (symbolC->level < 1 ? "($gp)" : "($sp)") + "\n");
		//return reg;
		exit(-8);
	}
    return sym2reg.find(symbolC)->second;
}

bool RegPool::isSymInReg(SymbolC *symbolC) {
    return sym2reg.find(symbolC) != sym2reg.end();
}

Register* RegPool::bindSym2Reg(SymbolC *symbolC, Register *reg) {
    sym2reg.insert(pair<SymbolC *, Register *>(symbolC, reg));
    return reg;
}
