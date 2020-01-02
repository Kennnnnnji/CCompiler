#include <iostream>
#include <string>
#include "header.h"
#include <random>
using namespace std;

int curLineNum = 1;
char* src;
int cur, fileSize;
char curChar;
int number;
char character;
string token;
Symbol symbol;
SymTable symTable;
CError globalErr;
GenInter genInter;
//ofstream outfile("output.txt");
GenMipsCode genMips;
RegPool regPool;
Opt opt;

int main() {
	ifstream infile("testfile.txt");
	if (!infile) {
		cout << "Error opening testfile.txt";
		exit(1);
	}
	string file_str;
	char c;
	while (infile.get(c)) {
		file_str += c;
	}
	infile.close();
	src = (char*)file_str.data();
	fileSize = file_str.length();

	// start
	/*while (getsym() == 0) {
		outfile << symbolNameArr[symbol] << " " << token << endl;
	}*/
	getsym();
	program();	// lex, syntax, error, mid
	genInter.interfile.close();
	/*if (globalErr.ERROR) {
		return 0;
	}*/
	opt.set_up_opt();
	opt.work();
	start();
	opt.optMips();
	//outfile.close();
	return 0;
}
