#include "InterLex.h"

InterLex::InterLex() {
	fstream file("17373052_王俊雄_优化后中间代码.txt", ios::in);
    if (!file.is_open()) {
        cout << "interLex.cpp: Error opening 17373052_王俊雄_优化后中间代码.txt";
        exit(1);
    }
    string s;
    while (getline(file, s)) {
        vector<string> words;
        lines.push_back(s);
        stringstream input(s);
        string word;
        while (input >> word) {
			if (word[0] == '-' && word[1] == '-') {
				word = word.substr(2);
			}
			if ((word[0] == '-' && word[1] == '+') ||
					(word[0] == '+' && word[1] == '-')) {
				word = "-" + word.substr(2);
			}
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

InterSymbol InterLex::reserve(vector<string> ln, const string& token) {
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
		if (token[0] == '+' || token[0] == '-') {
			if (isdigit(token[1])) return InterSymbol::INTCON;
			else if (token[1] == '_') return InterSymbol::MINUIDNF;
			else if (token[1] == '@') return InterSymbol::MINUINTV;
		}
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
	} else if (token == "para") {
		return InterSymbol::PARATK;
	} else if (token == "RET") {
		return InterSymbol::RETVALTK;
	}
	cerr << "InterLex TOKEN:" + token;
	for (string& s : ln) {
		cerr << s << " ";
	}
    exit(-6);
}

bool InterLex::isRelatOp(InterSymbol op) {
	return op == InterSymbol::EQL || op == InterSymbol::LEQ || op == InterSymbol::LESS ||
		op == InterSymbol::GEQ || op == InterSymbol::GRT || op == InterSymbol::NEQ;
}

int InterLex::maxLineIndex() {
	return lineWord.size();
}



int InterLex::getAppear(SymbolC* sym, int startL) {
    int cnt = 0;
    if (sym->name == "j") {
        cout << "";
    }
    int loopDep = 0;
    for (int start = startL; start < lineWord.size(); start++) {
        vector<string> line = lineWord.at(start);
        if (line.front().find("#End") != std::string::npos) break;
        if (line.front().find("#LOOP_STT") != std::string::npos) loopDep++;
        else if (line.front().find("#LOOP_END") != std::string::npos) loopDep--;
        for (string s : line) {
            string ss = s.substr(1);
            if (ss.find(sym->name + "]") != std::string::npos ||
                 ss == sym->name || ((ss.size()) > 1 && ss.substr(1) == sym->name)) {
                cnt += 1 + loopDep * 100;
            }
        }
    }
	return cnt;
}

bool InterLex::hasNextLine() {
    return curLine < lineWord.size() - 1;
}

vector<string> InterLex::preGetLine() {
    if (curLine >= lineWord.size()) {
        cerr << "preGetLine(): MAX LINES!" << endl;
        exit(-3);
    } else {
        return lineWord.at(curLine);
    }
}

vector<string> InterLex::preGetLine(int lineNum) {
	return lineWord.at(lineNum);
}

