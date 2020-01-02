#include "header.h"
#include "InterLex.h"
#include <unordered_map>

InterLex* interLex;
vector<string> line;
static int level = 0;
static int levelCnt = -1;
bool isFunc = false;
SymbolC* thisFunc;
static Register* load_var_val_2_reg_in_need(SymbolC* symc, RegType regT);
static void load_var_val_2_reg(SymbolC* symc, Register* reg);

static void err(const string& str) {
	cerr << "ERROR in MIPS.CPP! " + str + " line:" << interLex->curLine << endl;
}

static bool assert_type(const string& s, InterSymbol type, bool warn) {
	if (InterLex::reserve(line, s) != type && warn) {
		err("assert failed, " + s);
	}
	return InterLex::reserve(line, s) == type;
}

static void plus_level() {
	level = ++levelCnt;
}

static void init() {
	genMips.prt("j\tMain\n");
}

static void minu_level() {
	level = lvlMng.levelVec.at(level).outid;
}

static bool is_arr(string s) {
	return s.back() == ']';
}

static int get_index(const string& s) {
	int left = s.find('[');
	int right = s.find(']');
	string ss = s.substr(left + 1, right - 1);
	if (isdigit(ss[0]) || (ss[0] == '+' && isdigit(ss[1]))) { return stoi(ss); }
	return -1;
}

static Register* get_arr_off(const string& s) {
	int left = s.find('[');
	string ss = s.substr(left + 1);
	ss.pop_back();
	Register* reg;
	if (isdigit(ss[0]) || ((ss[0] == '+' || ss[0] == '-') && isdigit(ss[1]))) {
		reg = regPool.apply(RegType::T);
		genMips.prt("li\t" + reg->name + "\t" + to_string(4 * stoi(ss)) + "\n");
	} else {
		ss = ss[0] == '_' ? ss.substr(1) : ss;
		SymbolC* symc = symTable.find(ss, level, true);
		//regPool.release(symc);
		reg = regPool.apply(RegType::T);
		if (regPool.isSymInReg(symc)) {
			genMips.prt("move\t" + reg->name + "\t" + regPool.findReg(symc)->name + "\t# load index " + symc->name + "\n");
		} else {
			genMips.prt("lw\t" + reg->name + "\t-" + to_string(symc->addr) +
				(symc->level < level ? "($gp)" : "($sp)") + "\t# load index " + symc->name + "\n");
		}
		genMips.prt("sll\t" + reg->name + "\t" + reg->name + "\t2\n");
	}
	reg->protect = true;
	return reg;
}

static void global_const_def() {
	while (assert_type(line.at(0), InterSymbol::CONSTTK, false)) {
		if (!interLex->hasNextLine()) { break; }
		line = interLex->getLine();
	}
}

static void global_var_def() {
	int offset = 0;
	while (assert_type(line.at(0), InterSymbol::VARTK, false)) {
		SymbolC* sym = symTable.find(line.at(2).substr(1), level, false);
		sym->addr = offset;
		offset += sym->size;
		if (interLex->hasNextLine()) {
			line = interLex->getLine();
		} else { break; }
	}
}

static bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
static void deal_string_con() {
	auto symMap = symTable.sym_map;
	for (auto& it : symMap) {
		if (it.second.spec.baseType == BaseType::STRING) {
			::replace(it.second.strVal, "\\", "\\\\");
			genMips.prt(it.second.name + ":\t.asciiz\t\"" + it.second.strVal + "\"\n");
		}
	}
}

static void plus(bool useImmed, SymbolC* left, int immed, Register* lr, Register* rightReg) {
	if (useImmed) {
		genMips.prt("addi\t" + regPool.findReg(left)->name + "\t" + lr->name +
			"\t" + to_string(immed) + "\n");
	} else {
		genMips.prt("addu\t" + regPool.findReg(left)->name + "\t" + lr->name +
			"\t" + rightReg->name + "\n");
	}
}

static void minus(bool useImmed, SymbolC* left, int immed, Register* lr, Register* rightReg) {
	if (useImmed) {
		if (immed >= 0) {
			genMips.prt("addi\t" + regPool.findReg(left)->name + "\t" + lr->name +
				"\t-" + to_string(immed) + "\n");
		} else {
			genMips.prt("addi\t" + regPool.findReg(left)->name + "\t" + lr->name +
				"\t" + to_string(-immed) + "\n");
		}
	} else {
		genMips.prt("subu\t" + regPool.findReg(left)->name + "\t" + lr->name +
			"\t" + rightReg->name + "\n");
	}
}

static void mult(bool useImmed, SymbolC* left, int immed, Register* lr, Register* rightReg) {
	if (useImmed) {
		rightReg = regPool.apply(RegType::T);
		genMips.prt("li\t" + rightReg->name + "\t" + to_string(immed) + "\n");
	}
	genMips.prt("mul\t" + regPool.findReg(left)->name + "\t" + lr->name + "\t" + rightReg->name + "\n");
}

static void div(bool useImmed, SymbolC* left, int immed, Register* lr, Register* rightReg) {
	if (useImmed) {
		rightReg = regPool.apply(RegType::T);
		genMips.prt("li\t" + rightReg->name + "\t" + to_string(immed) + "\n");
	}
	genMips.prt("div\t" + lr->name + "\t" + rightReg->name + "\n");
	genMips.prt("mflo\t" + regPool.findReg(left)->name + "\n");
}

static SymbolC* find_sym_maybearr(string s) {
	SymbolC* ret;
	if (is_arr(s)) {
		ret = symTable.find(s.substr(1, s.find('[') - 1), level, true);     // arr
	} else {
		ret = symTable.find(s[0] == '_' ? s.substr(1) : s, level, true);    // _var or @intervar
	}
	return ret;
}

static void load_arr_value(SymbolC* left, SymbolC* right, const string& indexS) { // left <- right
	int index = get_index(indexS);
	// left shouldnt be an arr!
	if (!regPool.isSymInReg(left)) regPool.bindSym2Reg(left, regPool.apply_auto(left));
	if (index != -1) {    // a intcon
		genMips.prt("lw\t" + regPool.findReg(left)->name + "\t-" +
			to_string(right->addr + index * 4) + (right->level < level ? "($gp)" : "($sp)") + "\n");
	} else {    // index is a var or intervar
		Register* off = get_arr_off(indexS);	// offset
		genMips.prt("subu\t" + off->name + "\t" + (right->level < level ? "$gp\t" : "$sp\t") + off->name + "\n");		// plus fp/gp base addr
		genMips.prt(
			"lw\t" + regPool.findReg(left)->name + "\t-" + to_string(right->addr) + "(" +
			off->name + ")\n");		// plus base off
		off->protect = false;
	//regPool.release(off);
	}
}
static void load_arr_value_to_reg(Register* leftReg, SymbolC* right, const string& indexS) { // left <- right
	int index = get_index(indexS);
	if (index != -1) {    // a intcon
		genMips.prt("lw\t" + leftReg->name + "\t-" +
			to_string(right->addr + index * 4) + (right->level < level ? "($gp)" : "($sp)") + "\n");
	} else {    // index is a var or intervar
		Register* off = get_arr_off(indexS);	// offset
		genMips.prt("subu\t" + off->name + "\t" + (right->level < level ? "$gp\t" : "$sp\t") + off->name + "\n");		// plus fp/gp base addr
		genMips.prt(
			"lw\t" + leftReg->name + "\t-" + to_string(right->addr) + "(" +
			off->name + ")\n");		// plus base off
		off->protect = false;
	}
}


static Register* load_var_val_2_reg_in_need(SymbolC* symc, RegType regT) {
	Register* target;
	if (regPool.isSymInReg(symc)) {
		target = regPool.findReg(symc);  // left in regPool
	} else {
		target = regPool.bindSym2Reg(symc, regPool.apply_auto(symc));
		genMips.prt("lw\t" + target->name + "\t-" + to_string(symc->addr) +
			(symc->level < level ? "($gp)" : "($sp)") + "\n");
	}
	return target;
}

static void load_var_val_2_reg_in_need(SymbolC* symc, Register* reg) {
	if (regPool.isSymInReg(symc)) {
		Register* preReg = regPool.findReg(symc);
		if (preReg != reg) {
			genMips.prt("move\t" + reg->name + "\t" + preReg->name + "\n");
		}
	} else {
		genMips.prt("lw\t" + reg->name + "\t-" + to_string(symc->addr) +
			(symc->level < level ? "($gp)" : "($sp)") + "\n");
	}
}
static Register* find_bind_sym_if_need(SymbolC* right) {
	Register* ret = NULL;
	if (!regPool.isSymInReg(right)) {
		ret = regPool.apply_auto(right);
		regPool.bindSym2Reg(right, ret);
	} else {
		ret = regPool.findReg(right);
	}
	return ret;
}

static void val_stat() {	// 不会被中断, 但可能寄存器用完. 所以protect
	if (line.at(0) == "_j" && line.at(2) == "_j") {
		cout << "";
	}
	Register* rightReg = NULL, *leftReg = NULL;
	SymbolC* left, * right;
	string reals;
	left = find_sym_maybearr(line.at(0));
	bool newBindLeft = false;
	if (!regPool.isSymInReg(left)) {
		newBindLeft = true;
		leftReg = regPool.apply_auto(left);
		// 对数组元素,不可绑定
		if (!is_arr(line.at(0))) regPool.bindSym2Reg(left, leftReg);
	} else {
		leftReg = regPool.findReg(left);
	}
	leftReg->protect = true;
	switch (InterLex::reserve(line, line.at(2))) {
		case InterSymbol::INTCON:
			genMips.prt("li\t" + leftReg->name + "\t" + line[2] + "\n");
			break;
		case InterSymbol::CHARCON:
			genMips.prt(
				"ori\t" + leftReg->name + "\t$zero\t" + to_string((int)line.at(2).at(1)) +
				"\n");
			break;
		case InterSymbol::IDENFR:
		case InterSymbol::INTERVAR:
		POSI:
			right = find_sym_maybearr(line.at(2));
			if (right->isPara && right->paraId < 3 && !regPool.isSymInReg(right)) {
				Register* rParaReg = regPool.bindSym2Reg(right, regPool.apply_auto(right));
				genMips.prt("lw\t" + rParaReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
			}
			if (right->isVar && right->varId < 8 && !regPool.isSymInReg(right)) {
				Register* rParaReg = regPool.bindSym2Reg(right, regPool.apply_auto(right));
				genMips.prt("lw\t" + rParaReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
			}
			// shouldnt appear: arr[a] = arr[b]
			if (is_arr(line.at(2))) {
				load_arr_value(left, right, line.at(2));
			} else if (left == right && newBindLeft) {
				load_var_val_2_reg(left, regPool.findReg(left));
			} else {
				if (right->spec.isConst) {    // const, replace to an immediate
					genMips.prt(
						"li\t" + leftReg->name + "\t" + to_string(right->value) + "\n");
				} else {	// when para?
					if (regPool.isSymInReg(right)) {
						if (left != right && line.size() == 3) {
							genMips.prt("move\t" + leftReg->name + "\t" + regPool.findReg(right)->name + "\n");
						} else if (line.size() == 5) {	// if size == 5, no need to move
							leftReg = regPool.findReg(right);
						}
					} else {
						genMips.prt("lw\t" + leftReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
					}
				}
			}
			break;
		case InterSymbol::MINUIDNF:
		case InterSymbol::MINUINTV:	// n = -m or n = +m
			if (line.at(2)[0] == '+') {
				line.at(2) = line.at(2).substr(1);
				goto POSI;
			}
			reals = line.at(2).substr(1);
			right = find_sym_maybearr(reals);
			if (right->isPara && right->paraId < 3 && !regPool.isSymInReg(right)) {
				Register* rParaReg = regPool.bindSym2Reg(right, regPool.apply_auto(right));
				genMips.prt("lw\t" + rParaReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
			}
			if (right->isVar && right->varId < 8 && !regPool.isSymInReg(right)) {
				Register* rParaReg = regPool.bindSym2Reg(right, regPool.apply_auto(right));
				genMips.prt("lw\t" + rParaReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
			}
			if (is_arr(reals)) {
				load_arr_value(left, right, reals);
				genMips.prt("neg\t" + leftReg->name + "\t" + leftReg->name + "\n");
			} else {
				if (right->spec.isConst) {    // const, replace to an immediate
					genMips.prt(
						"li\t" + leftReg->name + "\t-" + to_string(right->value) + "\n");
				} else {	// when para?
					if (regPool.isSymInReg(right)) {
						genMips.prt("neg\t" + leftReg->name + "\t" + regPool.findReg(right)->name + "\n");
					} else {
						genMips.prt("lw\t" + leftReg->name + "\t-" + to_string(right->addr) + (right->level < level ? "($gp)" : "($sp)") + "\n");
						genMips.prt("neg\t" + leftReg->name + "\t" + leftReg->name + "\n");
					}
				}
			}
			break;
		case InterSymbol::RETVALTK:
			genMips.prt("move\t" + leftReg->name + "\t$v0\n");
			break;
		default:
			// ... = RET
			err("in val_stat(): switch 3, incomplete");
	}
	if (line.at(2) == "_j") {
		cout << "";
	}
	// operation
	if (line.size() == 5) {
		int immed = 0;
		bool useImmed = false;
		switch (InterLex::reserve(line, line.at(4))) {
			case InterSymbol::INTCON:
				immed = stoi(line[4]);
				if (-0x7fff <= immed && immed <= 0x7fff - 1) {
					useImmed = true;
				} else {
					rightReg = regPool.apply(RegType::T);
					genMips.prt("li\t" + rightReg->name + "\t" + line[4] + "\n");
				}
				break;
			case InterSymbol::CHARCON:
				immed = (int)line[4][1];
				useImmed = true;
				break;
			case InterSymbol::IDENFR:
			case InterSymbol::INTERVAR: // no arr
				right = find_sym_maybearr(line.at(4));
				if (right->spec.isConst) {
					immed = right->value;
					if (-0x7fff <= immed && immed <= 0x7fff - 1) {
						useImmed = true;
					} else {
						rightReg = find_bind_sym_if_need(right);
						genMips.prt("li\t" + rightReg->name + "\t" + to_string(right->value) + "\n");
					}
				} else {
					rightReg = load_var_val_2_reg_in_need(right, RegType::T);
				}
				break;
			case InterSymbol::RETVALTK:
				//rightReg = regPool.apply(RegType::T);
				//genMips.prt("move\t" + rightReg->name + "\t$v0\n");
				rightReg = regPool.v[0];
				break;
			default:
				err("in val_stat(): switch 3");
		}
		if (rightReg) rightReg->protect = true;
		// operation
		switch (InterLex::reserve(line, line.at(3))) {
			case InterSymbol::PLUS:
				::plus(useImmed, left, immed, leftReg, rightReg);
				break;
			case InterSymbol::MINU:
				::minus(useImmed, left, immed, leftReg, rightReg);
				break;
			case InterSymbol::MULT:
				::mult(useImmed, left, immed, leftReg, rightReg);
				break;
			case InterSymbol::DIV:
				::div(useImmed, left, immed, leftReg, rightReg);
				break;
			default:
				err("fuck, undefined operation");
				break;
		}
	} else if (line.size() != 3) {
		err("in val_stat(): wrong length");
	}
	// save to mem
	if (left->arr) {
		Register* add = get_arr_off(line.at(0)[0] == '_'? line.at(0).substr(1) : line.at(0));
		genMips.prt("subu\t" + add->name + "\t" +
			(left->level < level ? "$gp" : "$sp") + "\t" + add->name + "\n");
		genMips.prt("sw\t" + leftReg->name + "\t-" + to_string(left->addr) + "(" + add->name + ")\n");
		add->protect = false;
	}
	if (rightReg && rightReg->type != RegType::A && !(rightReg->type == RegType::S && rightReg->id < 8)) rightReg->protect = false;
	if (leftReg && leftReg->type != RegType::A && !(leftReg->type == RegType::S && leftReg->id < 8)) leftReg->protect = false;
	if (regPool.isSymInReg(left) && regPool.findReg(left)->type != RegType::A && !(regPool.findReg(left)->type == RegType::S && regPool.findReg(left)->id < 8))
		regPool.findReg(left)->protect = false;
}

static void scan_stat(const vector<SymbolC*>& vec) {
	Register* tmp;// = regPool.apply(RegType::T
	for (auto i : vec) {
		if (regPool.isSymInReg(i)) tmp = regPool.findReg(i);
		else {
			tmp = regPool.apply_auto(i);
			regPool.bindSym2Reg(i, tmp);
		}
		if (i->spec.baseType == BaseType::CHAR) {
			genMips.read_char(tmp->name);
		} else {
			genMips.read_int(tmp->name);
		}
	}
}

static void prt_stat(const vector<SymbolC*>& vec) {
	for (auto i : vec) {		// EXSIST PROBLEM: PRINT \n as "\n"
		if (i->spec.baseType == BaseType::STRING) {
			genMips.print_str(i->name);
		} else {
			if (i->name == "@para_RET") {
				if (i->spec.baseType == BaseType::CHAR) {
					genMips.print_char(regPool.v[0]->name);
				} else {
					genMips.print_int(regPool.v[0]->name);
				}
			} else if (regPool.isSymInReg(i)) {
				if (i->spec.baseType == BaseType::CHAR) {
					genMips.print_char(regPool.findReg(i)->name);
				} else {
					genMips.print_int(regPool.findReg(i)->name);
				}
			} else {
				//Register* tmp = regPool.apply(RegType::T);
				Register* tmp = regPool.apply_auto(i);
				regPool.bindSym2Reg(i, tmp);
				if (i->spec.isConst) {
					genMips.prt("li\t" + tmp->name + "\t" + to_string(i->value) + "\n");
				} else {
					genMips.prt("lw\t" + tmp->name + "\t-" + to_string(i->addr) + (i->level < level ? "($gp)" : "($sp)") + "\n");
				}
				if (i->spec.baseType == BaseType::CHAR) {
					genMips.print_char(tmp->name);
				} else {
					genMips.print_int(tmp->name);
				}
				//regPool.release(tmp);
			}
		}
	}
	genMips.print_enter();
}

static void func_call(const vector<SymbolC*>& vec) {
	SymbolC* func = symTable.find(line.at(1).substr(1, line.at(1).size() - 1), level, true);	// f to be called
	int funcVarCntExceptArr = symTable.lvl_var_cnt_except_para_arr(func->inLevel);
	for (int i = 0; i < funcVarCntExceptArr && i < 8; i++) {
		regPool.s[i]->protect = false;
	}
	if (func->name == "mod") {
		cout << "";
	}
	regPool.releaseAllFull(true);		// should know if @tmp in multiple blocks
	// init pushCnt
	int thisArSize = genMips.pushCnt = 2 + thisFunc->args.size() + symTable.lvl_var_cnt(thisFunc->inLevel, true);
	genMips.push("$ra");
	genMips.push("$sp");
	int argCnt = 0;
	for (auto& para : vec) {
		if (argCnt < 3) {
			//regPool.erase(regPool.a[argCnt + 1]); // ?
			Register* reg = regPool.apply(RegType::A, argCnt + 1);	// apply full
			reg->protect = true;
			if (para->name == "@para_RET") {
				genMips.prt("move\t" + reg->name + "\t$v0\n");
			} else if (para->spec.isConst) {
				genMips.prt("li\t" + reg->name + "\t" + to_string(para->value) + "\n");
			} else {
				load_var_val_2_reg_in_need(para, reg);
			}
			genMips.pushCnt += 1;
		} else {
			Register* reg;
			if (para->name == "@para_RET") {
				reg = regPool.v[0];
			} else if (para->spec.isConst) {
				reg = regPool.apply(RegType::T);
				genMips.prt("li\t" + reg->name + "\t" + to_string(para->value) + "\n");
			} else {
				reg = load_var_val_2_reg_in_need(para, RegType::T);
			}
			genMips.push(reg->name);
			regPool.release_save_gp(reg);
		}
		argCnt++;
	}
	regPool.release_all_save_gp();	// release @tmp
	/* move $sp to next Ar */
	genMips.prt("addi\t$sp\t$sp\t-" + to_string(4 * thisArSize) + "\n");
	genMips.jal("Func_" + func->name);
	/* restore $ra , $sp */
	genMips.prt("lw\t$ra\t($sp)\n");
	genMips.prt("lw\t$sp\t-4($sp)\n");
	for (auto& i : regPool.a) regPool.erase(i);
	for (auto& i : regPool.s) {
		if (i->id < funcVarCntExceptArr)	regPool.erase(i);
	}
	for (auto& sym : symTable.sym_map) {
		SymbolC* symc = &sym.second;
		if (symc->isVar && symc->varId < 8 && symc->varId < funcVarCntExceptArr && symc->level == level) {
			load_var_val_2_reg(symc, regPool.s[symc->varId]);
		}
	}
	// need to load A
}

static void func_stat() {
	vector<SymbolC*> paras;
	// push: left -> right
	int paraCnt = 0;
	while (InterLex::reserve(line, line.at(0)) == InterSymbol::PUSHTK) {
		SymbolC* sym;
		static int paraTmpCnt = 0;
		if (InterLex::reserve(line, line.at(1)) == InterSymbol::INTCON) {
			string name = "@para_" + to_string(paraTmpCnt++);
			sym = new SymbolC(name, level, 0, Specifier(true, BaseType::INT));
			sym->value = stoi(line.at(1));
		} else if (InterLex::reserve(line, line.at(1)) == InterSymbol::CHARCON) {
			string name = "@para_" + to_string(paraTmpCnt++);
			sym = new SymbolC(name, level, 0, Specifier(true, BaseType::CHAR));
			sym->value = (int)line.at(1)[1];
		} else if (InterLex::reserve(line, line.at(1)) == InterSymbol::MINUIDNF ||
			InterLex::reserve(line, line.at(1)) == InterSymbol::MINUINTV) {
			if (line.at(1)[0] == '+') goto POSIT;
			sym = find_sym_maybearr(line.at(1).substr(1));
			Register* reg = load_var_val_2_reg_in_need(sym, RegType::T);
			genMips.prt("neg\t" + reg->name + "\t" + reg->name + "\n");
		} else if (InterLex::reserve(line, line.at(1)) == InterSymbol::RETVALTK) {
			sym = new SymbolC("@para_RET", level, 0, Specifier(true, BaseType::INT));
			auto preLine = interLex->preGetLine(interLex->curLine - 2);
			sym->spec.baseType = symTable.find(preLine.at(1).substr(1, preLine.at(1).find('(') - 1), level, true)->spec.baseType;
		} else {
			POSIT:
			sym = find_sym_maybearr(line.at(1));
		}
		line = interLex->getLine();
		paras.push_back(sym);
		//sym->paraId = paraCnt;
		paraCnt++;
	}
	switch (InterLex::reserve(line, line.at(0))) {
		case InterSymbol::SCANTK:
			scan_stat(paras);
			break;
		case InterSymbol::PRINTTK:
			prt_stat(paras);
			break;
		case InterSymbol::CALLTK:
			func_call(paras);
		default:
			break;
	}
}


static void load_var_val_2_reg(SymbolC* symc, Register* reg) {
	genMips.prt("lw\t" + reg->name + "\t-" + to_string(symc->addr) +
		(symc->level < level ? "($gp)" : "($sp)") + "\n");
}

static string get_real_label(string s) {
	return s.substr(1);
}
static void condition_stat() {
	if (line.at(0) == "GOTO") {
		regPool.release_all_except_s_tmp();
		//regPool.releaseAll();
		genMips.prt("j\t" + get_real_label(line.at(1)) + "\n");
		return;
	}
	SymbolC* left = new SymbolC, *right = new SymbolC;
	vector<string> preNextLine = interLex->preGetLine();
	string jump = preNextLine[0];   // BZ, GOTO, BNZ
	string lb = get_real_label(preNextLine[1]);
	Register* leftReg, *rightReg;
	// left is intcon	eg. 1 <= _a
	if (InterLex::reserve(line, line.at(0)) == InterSymbol::INTCON) {
		if (stoi(line.at(0)) == 0) {
			leftReg = regPool.zero;
		}else {
			leftReg = regPool.apply(RegType::T);
			genMips.prt("li\t" + leftReg->name + "\t" + line.at(0) + "\n");
		}
	} else if (InterLex::reserve(line, line.at(0)) == InterSymbol::RETVALTK) {
		leftReg = regPool.v[0];
	} else {
		// left is var
		left = find_sym_maybearr(line.at(0));
		leftReg = load_var_val_2_reg_in_need(left, RegType::T);
	}
	if (line.size() <= 1) {
		regPool.release_all_except_s_tmp();
		//regPool.releaseAll();
		if (jump == "BZ") {
			genMips.prt("beqz\t" + leftReg->name + "\t" + lb + "\n");
		} else if (jump == "BNZ") {
			genMips.prt("bnez\t" + leftReg->name + "\t" + lb + "\n");
		}
		return;
	}
	if (InterLex::reserve(line, line.at(2)) == InterSymbol::INTCON) {
		if (stoi(line.at(2)) == 0) {
			rightReg = regPool.zero;
		} else {
			rightReg = regPool.apply(RegType::T);
			genMips.prt("li\t" + rightReg->name + "\t" + line.at(2) + "\n");
		}
	} else if (InterLex::reserve(line, line.at(2)) == InterSymbol::RETVALTK) {
		rightReg = regPool.v[0];
	} else {
		right = find_sym_maybearr(line.at(2));
		rightReg = load_var_val_2_reg_in_need(right, RegType::T);
	}
	//regPool.release_all_except_s();
	regPool.release_all_except_s_tmp();
	//regPool.releaseAll();
	switch (InterLex::reserve(line, line.at(1))) {
		case InterSymbol::EQL:
			if (jump == "BZ") {
				genMips.prt("bne\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") {
				genMips.prt("beq\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		case InterSymbol::NEQ:
			if (jump == "BZ") {
				genMips.prt("beq\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") {
				genMips.prt("bne\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		case InterSymbol::LEQ:
			if (jump == "BZ") { // false
				genMips.prt("bgt\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") { // true
				genMips.prt("ble\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		case InterSymbol::LESS:
			if (jump == "BZ") { // false
 				genMips.prt("bge\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") { // true
				genMips.prt("blt\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		case InterSymbol::GEQ:
			if (jump == "BZ") { // false
				genMips.prt("blt\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") { // true
				genMips.prt("bge\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		case InterSymbol::GRT:
			if (jump == "BZ") { // false
				genMips.prt("ble\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			} else if (jump == "BNZ") { // true
				genMips.prt("bgt\t" + leftReg->name + "\t" + rightReg->name + "\t" + lb + "\n");
			}
			break;
		default:
			err("w t hell! in condition_stat");
			break;
	}
	//regPool.release(leftReg);
	//regPool.release(rightReg);
	if (!leftReg->protect) {
		if (!left->name.empty() && left->name.front() == '@' && opt.find_var_in_n_blocks(left->name) > 1) regPool.release(leftReg);
		else regPool.erase(leftReg);
	}
	if (!rightReg->protect) {
		if (!right->name.empty() && right->name.front() == '@' && opt.find_var_in_n_blocks(right->name) > 1) regPool.release(rightReg);
		else regPool.erase(rightReg);
	}
	line = interLex->getLine(); // not for next stat, just goto branch stat
}

static void print_label() {
	if (line.at(0)[1] == 'L') {
		//regPool.releaseAll();
		regPool.release_all_except_s();
	}
	genMips.prt(line.at(0).substr(1) + ":\n");
}

static void ret_stat() {
	if (line.size() < 2) {
		if (thisFunc->name == "main") {
			genMips.prt_exit();
			return;
		}
		regPool.release_all_save_gp();
		genMips.prt("jr\t$ra\n");
		return;
	}
	if (InterLex::reserve(line, line.at(1)) == InterSymbol::INTCON) {
		genMips.prt("li\t$v0\t" + line.at(1) + "\n");
	} else if (InterLex::reserve(line, line.at(1)) == InterSymbol::CHARCON) {
		genMips.prt("li\t$v0\t" + to_string((int)(line.at(1)[1])) + "\n");
	} else if (InterLex::reserve(line, line.at(1)) == InterSymbol::RETVALTK) {
		;
	} else {
		bool neg = line.at(1)[0] == '-';
		if (neg) line.at(1) = line.at(1).substr(1);
		if (InterLex::reserve(line, line.at(1)) == InterSymbol::RETVALTK) {
			if (neg) genMips.prt("neg\t$v0\t$v0\n");
			goto ENDRET;
		}
		SymbolC* retSym = find_sym_maybearr(line.at(1));
		Register* retReg = regPool.apply(RegType::V, 0);
		if (is_arr(line.at(1))) {
			load_arr_value_to_reg(retReg, retSym, line.at(1));
			if (neg) genMips.prt("neg\t$v0\t$v0\n");
		} else {
			if (retSym->spec.isConst) {    // const, replace to an immediate
				if (neg) {
					genMips.prt("li\t" + retReg->name + "\t-" + to_string(retSym->value) + "\n");
				} else {
					genMips.prt("li\t" + retReg->name + "\t" + to_string(retSym->value) + "\n");
				}
			} else {
				if (regPool.isSymInReg(retSym)) {
					if (neg) genMips.prt("neg\t$v0\t" + regPool.findReg(retSym)->name + "\n");
					else genMips.prt("move\t$v0\t" + regPool.findReg(retSym)->name + "\n");
				} else {
					genMips.prt("lw\t" + retReg->name + "\t-" + to_string(retSym->addr) +
						(retSym->level < level ? "($gp)" : "($sp)") + "\n");
					if (neg) genMips.prt("neg\t$v0\t$v0\n");
				}
			}
		}
	}
	ENDRET:
	regPool.release_all_save_gp();
	//for (auto& i : regPool.a) regPool.erase(i);
	genMips.prt("jr\t$ra\n");
}

static void statement() {
	if (line.at(0) == "#Func_ibase") {
		cout << "";
	}
	InterSymbol op;
	switch (InterLex::reserve(line, line.at(0))) {
		case InterSymbol::IDENFR:
		case InterSymbol::INTERVAR: // val stat or cond stat
			if (line.size() <= 1) {
				condition_stat();
				break;
			}
			op = InterLex::reserve(line, line.at(1));
			if (op == InterSymbol::ASSIGN) {    // val
				val_stat();
			} else if (InterLex::isRelatOp(op)) {
				condition_stat();
			} else {
				err("unknown op!");
			}
			break;
		case InterSymbol::PUSHTK:    // func stat
		case InterSymbol::CALLTK:
			func_stat();
			break;
		case InterSymbol::GOTOTK:
		case InterSymbol::INTCON:
		case InterSymbol::RETVALTK:
			condition_stat();
			break;
		case InterSymbol::LABELTK:
			print_label();
			break;
		case InterSymbol::RETTK:
			ret_stat();
			break;
		default:
			if (line.at(0) != "int" && line.at(0) != "char" && line.at(0) != "const" && line.at(0) != "para" && line.at(0) != "var" && line.at(0) != "void")
				cout << "skip line: " << line.at(0) << endl;
			break;
	}
	line = interLex->getLine();
}

static void mapLocalConst() {

}

void map_var_id(vector<SymbolC*> vars, int lineBak) {
	int varId = 0;
	set<SymbolC*> st(vars.begin(), vars.end());
	vars = vector<SymbolC*>(st.begin(), st.end());
	vector<SymbolC*> varscopy = vector<SymbolC*>(st.begin(), st.end());
	for (auto& sym : vars) {
		if (sym->arr) sym->varId = 100;
		else {
			sym->appear = interLex->getAppear(sym, lineBak);
		}
	}
	while (varId < 8) {
		if (vars.empty()) break;
		int maxAppear = -1;
		SymbolC* maxS = vars.front();
		int i = 0, j = 0;
		for (auto& sym : vars) {
			if (sym->appear > maxAppear) {
				maxAppear = sym->appear;
				maxS = sym;
				j = i;
			}
			i++;
		}
		if (maxS->name == "j") {
			cout << "";
		}
		maxS->varId = varId++;
		regPool.bindSym2Reg(maxS, regPool.s[maxS->varId]);
		regPool.s[maxS->varId]->protect = true;
		regPool.s[maxS->varId]->idle = false;
		vars.erase(vars.begin() + j);
	}
}

static void mapLocalVar() {
	int lineBak = interLex->curLine;
	int offset = 4 * 2;	// preRetAddr, preFp
	int paraId = 0;	// 0~3, ...
	vector<SymbolC*> vars;
	while (true) {
		if (line.at(0) == "#EOF" || line.at(0)[0] == '#' && line.at(0)[1] == 'E' || (line.size() > 1 && line.at(1) == "_main()")) {
			break;
		}
		if (InterLex::reserve(line, line.at(0)) == InterSymbol::PARATK || InterLex::reserve(line, line.at(0)) == InterSymbol::VARTK) {
			string idfName = line.at(2).substr(1);
			if (is_arr(line.at(2))) {
				idfName = line.at(2).substr(1, line.at(2).find('[') - 1);
			}
			if (symTable.contain(idfName, level, false)) {
				SymbolC* sym = symTable.find(idfName, level, false);
				sym->isPara = InterLex::reserve(line, line.at(0)) == InterSymbol::PARATK;
				if (!sym->isMapped) {
					if (sym->isPara) {
						sym->paraId = paraId++;
						if (sym->paraId < 3) {
							regPool.bindSym2Reg(sym, regPool.a[sym->paraId + 1]);
							regPool.a[sym->paraId + 1]->protect = true;
							regPool.a[sym->paraId + 1]->idle = false;
						}
					} else {	// var
						sym->isVar = true;
						vars.push_back(sym);
					}
					sym->addr = offset;
					offset += sym->size;
					sym->isMapped = true;
				}
			}
		} else if (InterLex::reserve(line, line.at(0)) == InterSymbol::IDENFR) {
			string idfName = line.at(0).substr(1);
			if (is_arr(line.at(0))) {
				idfName = line.at(0).substr(1, line.at(0).find('[') - 1);
			}
			if (symTable.contain(idfName, level, false)) {
				SymbolC* sym = symTable.find(idfName, level, false);
				if (!sym->isMapped) {
					sym->addr = offset;
					offset += sym->size;
					sym->isMapped = true;
				}
			}
		} else if (InterLex::reserve(line, line.at(0)) == InterSymbol::INTERVAR) {
			if (!symTable.contain(line.at(0), level, false)) {
				Specifier spc;
				if (line.size() == 3 && InterLex::reserve(line, line.at(2)) == InterSymbol::RETVALTK) {	// 设置返回值类型. if is func's result, find func's type
					auto preLine = interLex->preGetLine(interLex->curLine - 2);
					spc.baseType = symTable.find(preLine.at(1).substr(1, preLine.at(1).find('(') - 1), level, true)->spec.baseType;
				} else {
					for (unsigned int i = 1; i < line.size(); i++) {
						if (i == 2 && (InterLex::reserve(line, line.at(i)) == InterSymbol::IDENFR || InterLex::reserve(line, line.at(i)) == InterSymbol::INTERVAR)) {
							spc.baseType = symTable.find(line.at(i)[0] == '_' ? line.at(i).substr(1, line.at(i).find('[') - 1) : line.at(i), level, true)->spec.baseType;
						} else if (i == 2 && InterLex::reserve(line, line.at(2)) == InterSymbol::CHARCON) {
							spc.baseType = BaseType::CHAR;
						} else {	// line size > 3
							spc.baseType = BaseType::INT;
						}
					}
				}
				auto* sym = new SymbolC(line.at(0), level, 0, spc);
				sym->level = level;
				sym->addr = offset;
				offset += sym->size;
				sym->isMapped = true;
				symTable.add(*sym, lvlMng.levelVec.at(level).outid);
			}
		} else if (InterLex::reserve(line, line.at(0)) == InterSymbol::PUSHTK) {
			if (InterLex::reserve(line, line.at(1)) == InterSymbol::IDENFR) {
				string idfName = line.at(1).substr(1);
				if (is_arr(line.at(1))) {
					idfName = line.at(1).substr(1, line.at(1).find('[') - 1);
				}
				if (symTable.contain(idfName, level, false)) {
					SymbolC* sym = symTable.find(idfName, level, false);
					if (!sym->isMapped) {
						sym->addr = offset;
						offset += sym->size;
						sym->isMapped = true;
					}
				}
			}
		}
		line = interLex->getLine();
	}
	map_var_id(vars, lineBak);
	interLex->curLine = lineBak - 1;
	line = interLex->getLine();
}



static void state_list() {
	while (line.at(0) != "#EOF") { // && !(line.size() > 1 && line.at(1).substr(line.at(1).length() - 1) == ")")) {
		statement();
		if (line.at(0) != "#EOF" && line.at(0)[0] == '#' && line.at(0)[1] == 'E' || (line.size() > 1 && line.at(1) == "_main()")) {
			statement();
			return;
		}
	}
}

static SymbolC* find_func_in_block() {
	int i = interLex->curLine - 2;
	if (i < 0) i = 0;
	while (i < interLex->maxLineIndex()) {
		auto aline = interLex->preGetLine(i);
		if (aline.size() > 1 && aline.at(1)[aline.at(1).size() - 1] == ')') { // find a func define
			return symTable.find(aline.at(1).substr(1, aline.at(1).size() - 3), level, true);
		}
		i++;
	}
	err("cannot find func in block!");
	exit(-10);
}

static void complex_stat() {
	// deal with AR
	thisFunc = find_func_in_block();
	mapLocalConst();
	mapLocalVar();
	if (!isFunc) {
		SymbolC* mainf = symTable.find("main", level, true);
		int mainLocalVarsCnt = symTable.lvl_var_cnt(mainf->inLevel, true);
		int off = 4 * 2 + 4 * 0 + 4 * mainLocalVarsCnt;
	}
	state_list();
}

static void process_func_def() {
	while (!(line.size() > 1 && line.at(1) == "_main()")) {
		genMips.prt("\n");
		plus_level();
		isFunc = true;
		complex_stat();
		isFunc = false;
		minu_level();
		regPool.release_all_save_gp();
		genMips.prt("jr\t$ra\n\n");
		for (auto& i : regPool.a) regPool.erase(i);
		for (auto& i : regPool.s) regPool.erase(i);
	}
}

static void process_main() {
	assert_type(line.at(0), InterSymbol::VOIDTK, true);
	if (line.at(1) != "_main()") { err("not main func"); }
	genMips.prt("Main:\n");
	plus_level();
	line = interLex->getLine();
	complex_stat();
	minu_level();
}

void start() {
	plus_level();
	genMips.prt(".data\n");
	interLex = new InterLex();
	line = interLex->getLine();
	global_const_def();        // 全部用立即数
	global_var_def();
	deal_string_con();
	genMips.prt("\n\n.text\n");
	init();
	process_func_def();
	process_main();
	genMips.mipsOut.close();
}
