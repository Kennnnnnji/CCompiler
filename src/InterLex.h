//
// Created by kenwo on 2019/11/17.
//

#ifndef COMPILER_DEMO_INTERLEX_H
#define COMPILER_DEMO_INTERLEX_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "header.h"

using namespace std;

enum class InterSymbol {
    UNKNOWN = 0, IDENFR = 1, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK,
    SCANTK, PRINTTK, RETTK, PLUS, INTERVAR, BZTK, BNZTK, GOTOTK, PUSHTK, CALLTK,
    MINU, MULT, DIV, LESS, LEQ, GRT, GEQ, EQL, NEQ, ASSIGN, COLONTK, LABELTK, VARTK, VOIDTK, STRVAR,
	PARATK, RETVALTK, MINUIDNF, MINUINTV
};

class InterLex {
private:
    vector<string> lines;
    vector<vector<string>> lineWord;
public:
	int curLine = 0;
	InterLex();
    bool hasNextLine();
	vector<string> getLine();
    vector<string> preGetLine();
    vector<string> preGetLine(int lineNum);
    static InterSymbol reserve(vector<string> ln, const string& token);
	static bool isRelatOp(InterSymbol op);
	int maxLineIndex();
    int getAppear(SymbolC *sym, int startL);
    
};


#endif //COMPILER_DEMO_INTERLEX_H
