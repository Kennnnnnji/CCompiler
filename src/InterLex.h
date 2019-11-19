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

using namespace std;

enum class InterSymbol {
    UNKNOWN = 0, IDENFR = 1, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK,
    SCANTK, PRINTTK, RETTK, PLUS, INTERVAR, BZTK, BNZTK, GOTOTK, PUSHTK, CALLTK,
    MINU, MULT, DIV, LESS, LEQ, GRT, GEQ, EQL, NEQ, ASSIGN, COLONTK, LABELTK, VARTK, VOIDTK, STRVAR
};

class InterLex {
private:
    vector<string> lines;
    vector<vector<string>> lineWord;
    int curWord = 0;
public:
	int curLine = 0;
    explicit InterLex(fstream &file);
    bool hasNextLine();
    vector<string> getLine();
    static InterSymbol reserve(const string& token);
};


#endif //COMPILER_DEMO_INTERLEX_H
