#pragma once
#include <iostream>
#include <fstream>
#include "table.h"
#include "InterSym.h"

using namespace std;

class GenInter {
private:
	int loopcnt = 0;
public:
	ofstream interfile;
	string types[3] = { "void", "int", "char" };
	GenInter();
	void prt(string s);
	void find_func_def(const SymbolC& func);
	void find_const_var_def(SymbolC constVar);
	void find_normal_var_def(SymbolC constVar);
	string gen_label();
	string gen_label_loop_start();
	string gen_label_loop_end();
	string gen_func_label(string fname);
	string gen_func_end(string fname);
};

