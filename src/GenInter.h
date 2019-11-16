#pragma once
#include <iostream>
#include <fstream>
#include "table.h"
using namespace std;

class GenInter {
public:
	ofstream interfile;
	string types[3] = { "void", "int", "char" };
	GenInter();
	void find_func_def(SymbolC func);
	void find_func_use(SymbolC func, SymbolC* paras, SymbolC ret);
	void find_const_var_def(SymbolC constVar);
	void find_normal_var_def(SymbolC constVar);
};

