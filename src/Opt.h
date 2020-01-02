#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "BasicBlock.h"
#include "MipsBlock.h"

class Opt {
private:
	vector<string> lines;
	vector<vector<string>> lineWord;
	fstream infile, outfile;

	unordered_map<string, int> nob_times;
	BasicBlock * firstBlk{};

	void output();
	void deal_para();
	void delete_nob();
	void delete_nob_right();
	void setup_nob_times();
	bool contain_nob(string ss);
	void adjust_op_num_order();
	void replace_error();
	void create_blocks();
	bool is_jb_stat(vector<string> line);
	void connect_blks(BasicBlock* pre, BasicBlock* mid);
	void show_blks();
    void concat_blks();
    void con_zip();

	vector<string> mipsLines;
	vector<vector<string>> mipsLineWord;
	fstream mipsInfile, mipsOutfile;
	MipsBlock* fstMipsBlk{};
	void mipsCreateBlks();
	void show_mips_blks();
    void concat_mips_blks();
	void mips_output();
	
	void delete_nob_var();
public:
	int curLine = 0;

	Opt();
	void set_up_opt();
	vector<string> getLine();
    void con_spead();
    void work();
	void optMips();
	int find_var_in_n_blocks(string name);
};

