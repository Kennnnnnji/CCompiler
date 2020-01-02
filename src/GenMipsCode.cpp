//
// Created by kenwo on 2019/11/17.
//

#include "GenMipsCode.h"

GenMipsCode::GenMipsCode() {
    mipsOut.open("mips_mid.txt");
    if (!mipsOut.is_open()) {
        cerr << "GenMipsCode::GenMipsCode(): CANNOT OPEN mips_mid.TXT!" << endl;
        //exit(-4);
    }
	pushCnt = 0;
}

void GenMipsCode::prt(const string& s) {
	if (s == "sw\t$t1\t-536($sp)\n") {
		cout << "";
	}
    mipsOut << s;
	//cout << s;
}

void GenMipsCode::read_int(string regName) {
	//push("$v0");
	prt("li\t$v0\t5\n");
	mipsOut << "syscall\n";
	prt("move\t" + regName + "\t$v0\n");
	//pop("$v0");
}
void GenMipsCode::read_char(string regName) {
	//push("$v0");
	prt("li\t$v0\t12\n");
	mipsOut << "syscall\n";
	prt("move\t" + regName + "\t$v0\n");
	//pop("$v0");
}
void GenMipsCode::print_enter() {
	//push("$a0");
	prt("addi\t$a0\t$zero\t10\n");
	prt("li\t$v0\t11\n");
	mipsOut << "syscall\n";
	//pop("$a0");
}
void GenMipsCode::print_char(string regName) {
	//push("$a0");
	prt("add\t$a0\t$zero\t" + regName + "\n");
	prt("li\t$v0\t11\n");
	mipsOut << "syscall\n";
	//pop("$a0");
}

void GenMipsCode::print_int(string regName) {
	//push("$a0");
	prt("add\t$a0\t$zero\t" + regName + "\n");
	prt("li\t$v0\t1\n");
	mipsOut << "syscall\n";
	//pop("$a0");
}

void GenMipsCode::exit_prt() {
	mipsOut << "li\t$v0\t10\n";
	mipsOut << "syscall\n";
}

void GenMipsCode::push(string regName) {
	//mipsOut << "sw\t" + regName + "\t0($sp)\n";
	//mipsOut << "subi\t$sp\t$sp\t4\n";
	prt("sw\t" + regName + "\t-" + to_string(pushCnt * 4) + "($sp)\n");
	pushCnt++;
}

void GenMipsCode::pop(string regName) {
	mipsOut << "addi\t$sp\t$sp\t4\n";
	mipsOut << "lw\t" + regName + "\t0($sp)\n";
}

void GenMipsCode::print_str(string label) {
	//push("$a0");
	mipsOut << "la\t$a0\t" + label + "\n";
	mipsOut << "li\t$v0\t4\n";
	mipsOut << "syscall\n";
	//pop("$a0");
}

void GenMipsCode::jal(string target) {
	mipsOut << "jal\t" + target + "\n";
}

void GenMipsCode::prt_exit() {
	mipsOut << "li\t$v0\t10\n";
	mipsOut << "syscall\n";
}
