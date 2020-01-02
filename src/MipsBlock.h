#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <unordered_map>
#include "table.h"

class MipsBlock {
public:
	set < MipsBlock*> prevs;
	set < MipsBlock*> backs;
	vector<vector<string>> lines;
	set<SymbolC*> vars;
	int startLineNum;
	void set_lines(vector<vector<string>> lines);
	void show_lines();
	void delete_nob_sw();
};

