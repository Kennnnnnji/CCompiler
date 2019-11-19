//
// Created by kenwo on 2019/11/17.
//

#ifndef COMPILER_DEMO_GENMIPSCODE_H
#define COMPILER_DEMO_GENMIPSCODE_H

#include <iostream>
#include <fstream>
#include "table.h"

class GenMipsCode {
private:
    fstream mipsOut;
public:
    GenMipsCode();
    void prt(const string& s);
	void read_int(string regName);
	void read_char(string regName);
	void print_enter();
	void print_char(string regName);
	void print_int(string regName);
	void exit_prt();
	void push(string x);
	void pop(string regName);
	void print_str(string x);
};


#endif //COMPILER_DEMO_GENMIPSCODE_H
