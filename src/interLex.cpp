#include "InterLex.h"

InterLex::InterLex(fstream &file) {
    file.seekg(0, ios::beg);
    string s;
    while (getline(file, s)) {
        vector<string> words;
        lines.push_back(s);
        stringstream input(s);
        string word;
        while (input >> word) {
            words.push_back(word);
        }
        lineWord.push_back(words);
    }
}

vector<string> InterLex::getLine() {
    if (curLine >= lineWord.size()) {
        cerr << "getLine(): MAX LINES!" << endl;
        exit(-3);
    } else {
        auto ret = lineWord.at(curLine);
        curLine++;
        return ret;
    }
}

InterSymbol InterLex::reserve(const string& token) {
    if (token == "const") {
        return InterSymbol::CONSTTK;
    } else if (token == "int") {
        return InterSymbol::INTTK;
    } else if (token == "char") {
        return InterSymbol::CHARTK;
    } else if (token == "SCAN") {
        return InterSymbol::SCANTK;
    } else if (token == "PRINT") {
        return InterSymbol::PRINTTK;
    } else if (token == "ret") {
        return InterSymbol::RETTK;
    } else if (token == "+") {
        return InterSymbol::PLUS;
    } else if (token == "-") {
        return InterSymbol::MINU;
    } else if (token == "*") {
        return InterSymbol::MULT;
    } else if (token == "/") {
        return InterSymbol::DIV;
    } else if (token == "<") {
        return InterSymbol::LESS;
    } else if (token == "<=") {
        return InterSymbol::LEQ;
    } else if (token == "<=") {
        return InterSymbol::LEQ;
    } else if (token == ">") {
        return InterSymbol::GRT;
    } else if (token == ">=") {
        return InterSymbol::GEQ;
    } else if (token == "==") {
        return InterSymbol::EQL;
    } else if (token == "!=") {
        return InterSymbol::NEQ;
    } else if (token == "=") {
        return InterSymbol::ASSIGN;
    } else if (token == "BZ") {
        return InterSymbol::BZTK;
    } else if (token == "BNZ") {
        return InterSymbol::BNZTK;
    } else if (token == "GOTO") {
        return InterSymbol::GOTOTK;
    } else if (token == "push") {
        return InterSymbol::PUSHTK;
    } else if (token == "call") {
        return InterSymbol::CALLTK;
    } else if (token[0] == '@') {
        return InterSymbol::INTERVAR;
    } else if (isdigit(token[0]) || token[0] == '+' || token[0] == '-') {
        return InterSymbol::INTCON;
    } else if (token[0] == '\'') {
        return InterSymbol::CHARCON;
    } else if (token[0] == '\"') {
        return InterSymbol::STRCON;
    } else if (token == ":") {
        return InterSymbol::COLONTK;
    } else if (token[0] == '#') {
        return InterSymbol::LABELTK;
    } else if (token[0] == '_') {
        return  InterSymbol::IDENFR;
    } else if (token == "var") {
        return InterSymbol::VARTK;
	} else if (token == "void") {
		return InterSymbol::VOIDTK;
	} else if (token[0] == '$') {
		return InterSymbol::STRVAR;
	}
    cerr << "InterLex.cpp: UNKNOWN TOKEN IN INTER.TXT:" + token << endl;
    exit(-6);
}

bool InterLex::hasNextLine() {
    return curLine < lineWord.size() - 1;
}

