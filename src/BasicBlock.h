#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <unordered_map>
#include "table.h"

using namespace std;

class BasicBlock {
public:
	set < BasicBlock*> prevs;
	set < BasicBlock*> backs;
	vector<vector<string>> lines;
	set<SymbolC*> vars;
	int startLineNum;
	void set_lines(vector<vector<string>> lines);
	void show_lines();
	void const_spead();
    int find_var_times(const string& var);

};

