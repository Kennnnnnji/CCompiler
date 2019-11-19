#ifndef HEADER_H
#define HEADER_H
#include <iostream>
#include <string>
#include <list>
#include "table.h"
#include "cError.h"
#include "GenInter.h"
#include "InterSym.h"
#include "Register.h"
#include "RegPool.h"
#include "InterLex.h"
#include "GenMipsCode.h"

using namespace std;

enum class Symbol {
    UNKNOWN = 0, IDENFR = 1, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK, VOIDTK,
    MAINTK, IFTK, ELSETK, DOTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK, PLUS,
    MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN, SEMICN, COMMA, LPARENT,
    RPARENT, LBRACK, RBRACK, LBRACE, RBRACE, /* mine */ EOFF
};

const char* const symbolNameArr[] = {
	"UNKNOWN", "IDENFR", "INTCON", "CHARCON", "STRCON", "CONSTTK", "INTTK", "CHARTK", "VOIDTK",
	"MAINTK", "IFTK", "ELSETK", "DOTK", "WHILETK", "FORTK", "SCANFTK", "PRINTFTK", "RETURNTK", "PLUS",
	"MINU", "MULT", "DIV", "LSS", "LEQ", "GRE", "GEQ", "EQL", "NEQ", "ASSIGN", "SEMICN", "COMMA", "LPARENT",
	"RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE", "EOFF"
};

extern char *src, curChar;
extern int curLineNum, cur, fileSize;
extern LevelMng lvlMng;
extern string token;
extern Symbol symbol;
extern ofstream outfile;
extern SymTable symTable;
extern CError globalErr;
extern GenInter genInter;
extern GenMipsCode genMips;
extern int number;
extern char character;
extern RegPool regPool;


int next();
int sub_lines();

int getsym();
void sym_retract();
void sym_output();
void program();
void start();

#endif
