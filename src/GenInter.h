#pragma once
#include <iostream>
#include <fstream>
#include "table.h"
#include "InterSym.h"

using namespace std;

class GenInter {
public:
	fstream interfile;
	string types[3] = { "void", "int", "char" };
	GenInter();
	void prt(string s);
	void find_func_def(const SymbolC& func);
	void find_const_var_def(SymbolC constVar);
	void find_normal_var_def(SymbolC constVar);
	string gen_label();
};

