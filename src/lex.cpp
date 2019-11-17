#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <list>
#include <set>
#include "header.h"
using namespace std;

int last, last2, last3, last4;	// for sym_retract
list<string> outQue;

void char_getchar() {
	curChar = src[cur];
	cur++;
}

void clear_token() {
	token.clear();
}

void catToken() {
	token += curChar;
}

static void retract() {
	cur--;
}

Symbol reserver() {
	if (token == "const") {
		return Symbol::CONSTTK;
	} else if (token == "int") {
		return Symbol::INTTK;
	} else if (token == "char") {
		return Symbol::CHARTK;
	} else if (token == "void") {
		return Symbol::VOIDTK;
	} else if (token == "main") {
		return Symbol::MAINTK;
	} else if (token == "if") {
		return Symbol::IFTK;
	} else if (token == "else") {
		return Symbol::ELSETK;
	} else if (token == "do") {
		return Symbol::DOTK;
	} else if (token == "while") {
		return Symbol::WHILETK;
	} else if (token == "for") {
		return Symbol::FORTK;
	} else if (token == "scanf") {
		return Symbol::SCANFTK;
	} else if (token == "printf") {
		return Symbol::PRINTFTK;
	} else if (token == "return") {
		return Symbol::RETURNTK;
	}
	return Symbol::IDENFR;   // identifier
}

/*
static void error() {
	cerr << "err: in getsym(), cur = " << cur << ", curChar = " << curChar << endl;
	//exit(-1);
}
*/
int next() {
    last4 = last3;
	last3 = last2;
	last2 = last;
	last = cur;
	if (cur >= fileSize) {
		// cout << "end";
		symbol = Symbol::EOFF;
		return 1;
	}
	clear_token();
	char_getchar();
	while (isspace(curChar)) {	// ' ', '\n', '\v', '\t', '\f', '\r'
		if (curChar == '\n') { line++; }
		char_getchar();
	}
	if (curChar == '\0') {	// end of source string
		symbol = Symbol::EOFF;
		return 1;
	}
	token = curChar;
	if (isalpha(curChar) || curChar == '_') {	// IDENFR or reserver
		clear_token();
		while (isalpha(curChar) || isdigit(curChar) || curChar == '_') {
			catToken();
			char_getchar();
		}
		retract();
		symbol = (Symbol)reserver();
	} else if (isdigit(curChar)) {	// int
		symbol = Symbol::INTCON;
		clear_token();
		catToken();
		char_getchar();
		while (isdigit(curChar)) {
			catToken();
			char_getchar();
		}
		number = stoi(token);
		retract();
	} else if (curChar == '=') {
		symbol = Symbol::ASSIGN;
		char_getchar();
		if (curChar == '=') {
			symbol = Symbol::EQL;
			token.assign("==");
		} else {
			retract();
		}
	} else if (curChar == '+') {
		symbol = Symbol::PLUS;
	} else if (curChar == '-') {
		symbol = Symbol::MINU;
	} else if (curChar == '*') {
		symbol = Symbol::MULT;
	} else if (curChar == '/') {
		symbol = Symbol::DIV;
	} else if (curChar == '(') {
		symbol = Symbol::LPARENT;
	} else if (curChar == ')') {
		symbol = Symbol::RPARENT;
	} else if (curChar == ',') {
		symbol = Symbol::COMMA;
	} else if (curChar == '\'') {	// char
		// =  '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'
		symbol = Symbol::CHARCON;
		char_getchar();
		token = curChar;	// Escape characters incomplete!!!
		set<char> addmul({ '+','-','*','/' });
		if (addmul.find(curChar) == addmul.end() && !isalnum(curChar)) {
			globalErr.catch_e(line, "a");
		}
		character = curChar;
		char_getchar();
		if (curChar != '\'') {
			// error();
			globalErr.catch_e(line, "a");
		}
	} else if (curChar == '\"') {	// string
		symbol = Symbol::STRCON;
		char_getchar();
		token = curChar;	// Escape characters incomplete!!!
		char_getchar();
		while (curChar && curChar != '\"') {
			// 十进制编码为32,33,35-126的ASCII字符
			if (curChar != 32 && curChar != 33 && curChar < 35 && curChar > 126) {
				globalErr.catch_e(line, "a");
			}
			token += curChar;
			char_getchar();
		}
		if (curChar != '\"') {
			// error();
			globalErr.catch_e(line, "a");
		}
	} else if (curChar == ';') {
		symbol = Symbol::SEMICN;
	} else if (curChar == '<') {
		symbol = Symbol::LSS;
		char_getchar();
		if (curChar == '=') {
			symbol = Symbol::LEQ;
			token.assign("<=");
		} else {
			retract();
		}
	} else if (curChar == '>') {
		symbol = Symbol::GRE;
		char_getchar();
		if (curChar == '=') {
			symbol = Symbol::GEQ;
			token.assign(">=");
		} else {
			retract();
		}
	} else if (curChar == '!') {	// what about only one '!' ???
		char_getchar();
		if (curChar == '=') {
			symbol = Symbol::NEQ;
			token.assign("!=");
		} else {
			retract();
		}
	} else if (curChar == '[') {
		symbol = Symbol::LBRACK;
	} else if (curChar == ']') {
		symbol = Symbol::RBRACK;
	} else if (curChar == '{') {
		symbol = Symbol::LBRACE;
	} else if (curChar == '}') {
		symbol = Symbol::RBRACE;
	} else {
		// error();
		globalErr.catch_e(line, "a");
		return next();
	}
	return 0;
}

int getsym() {
	int ret = next();
	if (ret == 1) {
		return 1;
	}
	string s(symbolNameArr[static_cast<int>(symbol)]);
	s.append(" ");
	s.append(token);
	s.append("\n");
		outQue.push_back(s);
	return ret;
}

void sym_output() {
	while (!outQue.empty()) {
		outfile << (string)(outQue.front());
		outQue.pop_front();
	}
}

int sub_lines() {
	int n = 0;
	for (int i = last; i < cur; i++) {
		if (src[i] == '\n') { n++; }
	}
	return n;
}

void sym_retract() {
	line -= sub_lines();
	cur = last;
	last = last2;
	last2 = last3;
    last3 = last4;
	if (!outQue.empty()) {
		outQue.pop_back();
	}
}
