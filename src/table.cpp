#include "table.h"
#include "header.h"
#include <iostream>

Specifier::Specifier() = default;

Specifier::Specifier(bool iscon, BaseType t) {
	isConst = iscon;
	baseType = t;
}

bool SymTable::contain(string ss) {
	vector<f>::iterator it;
	for (it = fVec.begin(); it != fVec.end(); it++) {
		if ((*it).name == ss) {
			return true;
		}
	}
	return false;
}

// find an item in a given name and level, if(find && valid)return the item
SymbolC *SymTable::find(string nam, int level, bool outer) {
	SymbolC ret = * new SymbolC();
	auto pr = sym_map.equal_range(nam); 
	while (pr.first != pr.second) {
		if ((pr.first->second.level == level ||
			 (pr.first->second.level < level && outer)) &&
				pr.first->second.is_valid()) {
			return &(pr.first->second);
		}
		++pr.first; // Increment begin iterator
	}
	cerr << "CANNOT FIND SYMBOLC!" << nam << ", " << level << endl;
	return new SymbolC();
}

// find an item in a given name and level, return (find && valid)
bool SymTable::contain(string nam, int level, bool outer) {
	auto pr = sym_map.equal_range(nam);
	while (pr.first != pr.second) {
		if ((pr.first->second.level == level ||
			 (pr.first->second.level < level && outer)) &&
			pr.first->second.is_valid()) {
			return true;
		}
		++pr.first; // Increment begin iterator
	}
	return false;
}

SymbolC* SymTable::add(SymbolC s, int out_level) {
	int l = s.level;
	if (contain(s.name, l, false)) {
		// dup var
		globalErr.catch_e(curLineNum, "b");
		return new SymbolC();
	}
	sym_map.insert(pair<string, SymbolC>(s.name, s));
	if (entryVec.size() <= (unsigned int)l) {
		while (entryVec.size() < (unsigned int)l) {
			entryVec.push_back(*new SymbolEntry);
		}
		entryVec.push_back(*new SymbolEntry(s));
	} else {
		// new first
		SymbolEntry old = entryVec.at(l);
		old.setPrev(new SymbolEntry(s));
		(*old.getPrev()).setNext(&old);
		entryVec.at(l) = (*old.getPrev());
	}
	entryVec.at(l).outLevel = out_level;
	return find(s.name,s.level,false);
	// cout << "===" << s.name << ' ' << s.level << ' ' << out_level << endl;
}

void SymTable::devalid(int level) {
	SymbolEntry e = entryVec.at(level);
	while (e.getNext() != NULL) {
		SymbolC sbc = e.symbolc;// find(e.symbolc.name, e.symbolc.level, false);
		if (!sbc.param) {
			sbc.set_valid(false);
		}
		e = *e.getNext();
	}
}

SymbolEntry::SymbolEntry() {
	symbolc = *new SymbolC();
}

SymbolEntry::SymbolEntry(SymbolC symc) {
	symbolc = symc;
}

void SymbolEntry::setPrev(SymbolEntry* prev_) {
	prev = prev_;
}

void SymbolEntry::setNext(SymbolEntry* next_) {
	next = next_;
}

SymbolEntry* SymbolEntry::getPrev() {
	return prev;
}

SymbolEntry* SymbolEntry::getNext() {
	return next;
}

SymbolC::SymbolC() {
	valid = false;
	duplicate = true;
}

SymbolC::SymbolC(string nam, int lev, int lin, Specifier spc) {
	name = nam;
	level = lev;
	line = lin;
	spec = spc;
	valid = true;
}

void SymbolC::set_spec(Specifier spc) {
	spec = spc;
}

Specifier* SymbolC::get_spec() {
	return &spec;
}

bool SymbolC::is_ret_func() {
	return func && spec.baseType != BaseType::VOID;
}

void SymbolC::set_valid(bool b) {
	valid = b;
}

bool SymbolC::is_valid() { return valid; }

Level::Level(int id, int outid) {
	this->id = id;
	this->outid = outid;
}
