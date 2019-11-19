#include "header.h"
#include "InterLex.h"
#include <unordered_map>
InterLex* interLex;
vector<string> line;
static int level = 0;
static int levelCnt = -1;


static void err(const string& str) {
	cerr << "ERROR in MIPS.CPP! " + str + " line:" << interLex->curLine << endl;
}

static bool assert_type(const string& s, InterSymbol type, bool warn) {
	if (InterLex::reserve(s) != type && warn) {
		err("assert failed, " + s);
	}
	return InterLex::reserve(s) == type;
}

static void into_func() {
	level = ++levelCnt;
}

static void init() {
	genMips.prt("addi\t$fp\t$sp\t4\n");
}

static void out_func() {
	level = lvlMng.levelVec.at(level).outid;
}

static bool is_arr(string s) {
	return s.back() == ']';
}

static int get_index(string s) {
	int left = s.find('[');
	int right = s.find(']');
	string ss = s.substr(left + 1, right - 1);
	if (isdigit(ss[0]) || ss[0] == '+') { return stoi(ss); }
	return -1;
}

static Register* get_arr_addr(string s) {
	Register * reg;
	int left = s.find('[');
	int right = s.find(']');
	string ss = s.substr(left + 1);
	ss.pop_back();
	SymbolC* symc = symTable.find(ss, level, true);
	if (!regPool.isSymInReg(symc)) {
		regPool.bindSym2Reg(symc, regPool.apply(RegType::T));
		reg = regPool.findReg(symc);
		genMips.prt("lw\t$t" + to_string(reg->id) + "\t" + to_string(symc->addr) + (symc->level < level ? "($gp)" : "($fp)") + "\n");
	} else {
		reg = regPool.findReg(symc);
	}
	genMips.prt("sll\t$t" + to_string(reg->id) + "\t$t" + to_string(reg->id) + "\t2\n");
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
		//genMips.prt(line.at(2).substr(1) + ":\t.word\n");
		SymbolC* sym = symTable.find(line.at(2).substr(1), level, false);
		sym->addr = offset;
		offset += sym->size;
		if (interLex->hasNextLine()) {
			line = interLex->getLine();
		} else { break; }
	}
}

static void string_con() {
	auto symMap = symTable.sym_map;
	for (unordered_multimap<string, SymbolC>::iterator it = symMap.begin(); it != symMap.end(); it++) {
		if (it->second.spec.baseType == BaseType::STRING) {
			genMips.prt(it->first + ":\t.asciiz\t\"" + it->second.strVal + "\"\n");
		}
	}
}

static void process_func_def() {

}

static void plus(bool useImmed, SymbolC* left, int immed, Register* rightReg) {
	if (useImmed) {
		genMips.prt("addiu\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(immed) + "\n");
	} else {
		genMips.prt("addu\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(rightReg->id) + "\n");
	}
}

static void minus(bool useImmed, SymbolC* left, int immed, Register* rightReg) {
	if (useImmed) {
		genMips.prt("subiu\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(immed) + "\n");
	} else {
		genMips.prt("subu\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(rightReg->id) + "\n");
	}
}

static void mult(bool useImmed, SymbolC* left, int immed, Register* rightReg) {
	if (useImmed) {
		rightReg = regPool.apply(RegType::T);
		genMips.prt("li\t$t" + to_string(rightReg->id) + "\t" + to_string(immed) + "\n");
	}
	genMips.prt("mult\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(rightReg->id) + "\n");
	genMips.prt("mflo\t$t" + to_string(regPool.findReg(left)->id) + "\n");
}

static void div(bool useImmed, SymbolC* left, int immed, Register* rightReg) {
	if (useImmed) {
		rightReg = regPool.apply(RegType::T);
		genMips.prt("li\t$t" + to_string(rightReg->id) + "\t" + to_string(immed) + "\n");
	}
	genMips.prt("divu\t$t" + to_string(regPool.findReg(left)->id) + "\t$t" + to_string(rightReg->id) + "\n");
	genMips.prt("mflo\t$t" + to_string(regPool.findReg(left)->id) + "\n");
}

static void val_stat() {
	Register* rightReg;
	SymbolC* left, * right;
	if (is_arr(line.at(0))) {
		left = symTable.find(line.at(0).substr(1, line.at(0).find('[') - 1), level, true);
	} else {
		left = symTable.find(line.at(0)[0] == '_' ? line.at(0).substr(1) : line.at(0), level, true);
	}
	if (!regPool.isSymInReg(left)) {
		regPool.bindSym2Reg(left, regPool.apply(RegType::T));
	}
	switch (InterLex::reserve(line.at(2))) {
		case InterSymbol::INTCON:
			genMips.prt("li\t$t" + to_string(regPool.findReg(left)->id) + "\t" + line[2] + "\n");	 // TODO: split li	?? mars automatically split in need
			break;
		case InterSymbol::CHARCON:
			genMips.prt("ori\t$t" + to_string(regPool.findReg(left)->id) + "\t$zero\t" + to_string((int)line.at(2).at(1)) + "\n");
			break;
		case InterSymbol::IDENFR:
		case InterSymbol::INTERVAR:
			// TODO: array
			if (is_arr(line.at(2)) == true) {
				int index = get_index(line.at(2));
				right = symTable.find(line.at(2).substr(1, line.at(2).find('[') - 1), level, true);
				if (index != -1) {	// a intcon
					genMips.prt("lw\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(right->addr + index * 4) + (right->level < level ? "($gp)" : "($fp)") + "\n");
				} else {
					Register* add = get_arr_addr(line.at(2));
					genMips.prt("addu\t$t" + to_string(add->id) + "\t$t" + to_string(add->id) + "\t" + (right->level < level ? "$gp" : "$fp") + "\n");
					genMips.prt("lw\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(right->addr) + "($t" + to_string(add->id) + ")\n");
					regPool.release(add);
				}
			} else {
				right = symTable.find(line.at(2)[0] == '_' ? line.at(2).substr(1) : line.at(2), level, true);
				if (right->spec.isConst) {	// const, replace to an immediate
					genMips.prt("li\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(right->value) + "\n");
				} else {
					genMips.prt("lw\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(right->addr) + (right->level < level ? "($gp)" : "($fp)") + "\n");
				}
			}
			break;
		default:
			err("in val_stat(): switch 3");
	}
	// operation
	if (line.size() == 5) {
		int immed = 0;
		bool useImmed = false;
		rightReg = regPool.apply(RegType::T);
		switch (InterLex::reserve(line.at(4))) {
			case InterSymbol::INTCON:
				immed = stoi(line[2]);
				if (0 <= immed && immed <= 0xffff) {
					useImmed = true;
				} else {
					genMips.prt("li\t$t" + to_string(rightReg->id) + "\t" + line[2] + "\n");
				}
				break;
			case InterSymbol::CHARCON:
				immed = stoi(line[2]);
				if (0 <= immed && immed <= 0xffff) {
					useImmed = true;
				} else {
					genMips.prt("ori\t$t" + to_string(rightReg->id) + "\t" + to_string((int)line.at(2).at(1)) + "\n");
				}
				break;
			case InterSymbol::IDENFR:
			case InterSymbol::INTERVAR:
				if (is_arr(line.at(4)) == true) {
					int index = get_index(line.at(4));
					right = symTable.find(line.at(4).substr(1, line.at(4).find('[') - 1), level, true);
					regPool.bindSym2Reg(right, rightReg);
					if (index != -1) {	// a intcon
						genMips.prt("lw\t$t" + to_string(rightReg->id) + "\t" + to_string(right->addr + index * 4) + (right->level < level ? "($gp)" : "($fp)") + "\n");
					} else {
						Register* add = get_arr_addr(line.at(2));
						genMips.prt("addu\t$t" + to_string(add->id) + "\t$t" + to_string(add->id) + "\t" + (right->level < level ? "$gp" : "$fp") + "\n");
						genMips.prt("lw\t$t" + to_string(rightReg->id) + "\t" + to_string(right->addr) + "($t" + to_string(add->id) + ")\n");
						regPool.release(add);
					}
				} else {
					right = symTable.find(line.at(4)[0] == '_' ? line.at(4).substr(1) : line.at(4), level, true);
					regPool.bindSym2Reg(right, rightReg);
					if (right->spec.isConst) {
						immed = right->value;
						if (0 <= immed && immed <= 0xffff) {
							useImmed = true;
						} else {
							genMips.prt("li\t$t" + to_string(rightReg->id) + "\t" + to_string(right->value) + "\n");
						}
					} else {
						genMips.prt("lw\t$t" + to_string(rightReg->id) + "\t" + to_string(right->addr) + (right->level < level ? "($gp)" : "($fp)") + "\n");
					}
				}
				break;
			default:
				err("in val_stat(): switch 3");
		}
		switch (InterLex::reserve(line.at(3))) {
			case InterSymbol::PLUS:
				::plus(useImmed, left, immed, rightReg);
				break;
			case InterSymbol::MINU:
				::minus(useImmed, left, immed, rightReg);
				break;
			case InterSymbol::MULT:
				::mult(useImmed, left, immed, rightReg);
				break;
			case InterSymbol::DIV:
				break;
			default:
				break;
		}
		regPool.release(rightReg);
	} else if (line.size() != 3) {
		err("in val_stat(): wrong length");
	}
	// save to mem
	// TODO: array
	if (left->arr) {
		Register* add = get_arr_addr(line.at(0));
		genMips.prt("addu\t$t" + to_string(add->id) + "\t$t" + to_string(add->id) + "\t" + (left->level < level ? "$gp" : "$fp") + "\n");
		genMips.prt("sw\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(left->addr) + "($t" + to_string(add->id) + ")\n");
		regPool.release(add);
	} else {
		genMips.prt("sw\t$t" + to_string(regPool.findReg(left)->id) + "\t" + to_string(left->addr) + (left->level < level ? "($gp)" : "($fp)") + "\n");
	}
	regPool.release(left);
}

static void scan_stat(vector<SymbolC*> vec) {
	Register* tmp = regPool.apply(RegType::T);
	for (auto i : vec) {
		if (i->spec.baseType == BaseType::CHAR) {
			genMips.read_char(tmp->name);
		} else {
			genMips.read_int(tmp->name);
		}
		genMips.prt("sw\t" + tmp->name + "\t" + to_string(i->addr) + (i->level < level ? "($gp)" : ("$fp")) + "\n");
	}
	regPool.release(tmp);
}
	
static void prt_stat(vector<SymbolC *> vec) {
	for (auto i : vec) {
		if (i->spec.baseType == BaseType::STRING) {
			genMips.print_str(i->name);
		} else {
			if (regPool.isSymInReg(i)) {
				if (i->spec.baseType == BaseType::CHAR) {
					genMips.print_char(regPool.findReg(i)->name);
				} else {
					genMips.print_int(regPool.findReg(i)->name);
				}
			} else {
				Register* tmp = regPool.apply(RegType::T);
				genMips.prt("lw\t$t" + to_string(tmp->id) + "\t" + to_string(i->addr) + (i->level < level ? "($gp)" : "($fp)") + "\n");
				if (i->spec.baseType == BaseType::CHAR) {
					genMips.print_char(tmp->name);
				} else {
					genMips.print_int(tmp->name);
				}
				regPool.release(tmp);
			}
		}
	}
	genMips.print_enter();
}

static void func_stat() {
	vector<SymbolC*> paras;
	while (interLex->reserve(line.at(0)) == InterSymbol::PUSHTK) {
		SymbolC* sym;
		if (is_arr(line.at(1))) {
			sym = symTable.find(line.at(1).substr(1, line.at(1).find('[') - 1), level, true);
		} else {
			sym = symTable.find(line.at(1)[0] == '_' ? line.at(1).substr(1) : line.at(1), level, true);
		}
		paras.push_back(sym);
		line = interLex->getLine();
	}
	switch (interLex->reserve(line.at(0))) {
		case InterSymbol::SCANTK:
			scan_stat(paras);
			break;
		case InterSymbol::PRINTTK:
			prt_stat(paras);
			break;
		default:
			break;
	}
}

static void statement() {
	switch (InterLex::reserve(line.at(0))) {
		case InterSymbol::IDENFR:
		case InterSymbol::INTERVAR: // val stat
			val_stat();
			break;
		case InterSymbol::PUSHTK:	// func stat
			func_stat();
		default:
			break;
	}
	line = interLex->getLine();
}

static void mapLocalConst() {

}

static void mapLocalVar() {
	int lineBak = interLex->curLine;
	int offset = 0;
	while (true) {
		if (line.at(0) == "#EOF" || (line.size() > 1 && line.at(1).substr(line.at(1).length() - 1) == ")")) {
			break;
		}
		if (InterLex::reserve(line.at(0)) == InterSymbol::IDENFR) {
			if (symTable.contain(line.at(0).substr(1), level, false)) {
				SymbolC* sym = symTable.find(line.at(0).substr(1), level, false);
				sym->addr = offset;
				offset += sym->size;
			}
		} else if (InterLex::reserve(line.at(0)) == InterSymbol::INTERVAR) {
			if (!symTable.contain(line.at(0), level, false)) {
				Specifier spc;
				for (int i = 1; i < line.size(); i++) {
					if (i == 2 && InterLex::reserve(line.at(i)) == InterSymbol::IDENFR || InterLex::reserve(line.at(i)) == InterSymbol::INTERVAR) {
						spc.baseType = symTable.find(line.at(i)[0] == '_'? line.at(i).substr(1, line.at(i).find('[') - 1) : line.at(i), level, true)->spec.baseType;
					} else if (i == 2 && InterLex::reserve(line.at(2)) == InterSymbol::CHARCON) {
						spc.baseType = BaseType::CHAR;
					} else {
						spc.baseType = BaseType::INT;
					}
				}
				auto* sym = new SymbolC(line.at(0), level, 0, spc);
				sym->addr = offset;
				offset += sym->size;
				symTable.add(*sym, lvlMng.levelVec.at(level).outid);
			}
		}
		line = interLex->getLine();
	}
	interLex->curLine = lineBak - 1;
	line = interLex->getLine();
}

static void state_list() {
	while (line.at(0) != "#EOF" && !(line.size() > 1 && line.at(1).substr(line.at(1).length() - 1) == ")")) {
		statement();
	}
}

static void complex_stat() {
	// deal with AR
	mapLocalConst();
	mapLocalVar();
	state_list();
}

static void process_main() {
	assert_type(line.at(0), InterSymbol::VOIDTK, true);
	if (line.at(1) != "_main()") { err("not main func"); }
	genMips.prt("Main :\n");
	into_func();
	line = interLex->getLine();
	complex_stat();
	out_func();
}

void start() {
	into_func();
	genMips.prt(".data\n");
	interLex = new InterLex(genInter.interfile);
	line = interLex->getLine();
	global_const_def();	// 全部用立即数
	global_var_def();
	string_con();
	genMips.prt(".text\n");
	init();
	process_func_def();
	process_main();
}
