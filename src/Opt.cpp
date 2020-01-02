#include "Opt.h"
#include "header.h"
#include <algorithm>

Opt::Opt() {}

void Opt::set_up_opt() {
    infile.open("17373052_王俊雄_优化前中间代码.txt");

    string s;
    while (std::getline(infile, s)) {
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
    infile.close();
}

void Opt::work() {
    create_blocks();
    con_spead();
    setup_nob_times();
    adjust_op_num_order();
    for (int i = 0; i < 3; i++) {
        delete_nob();
    }
    //replace_error();
    con_zip();
    //show_blks();
    delete_nob_var();
    output();
}

void Opt::optMips() {
    mipsInfile.open("mips_mid.txt");
    if (!mipsInfile.is_open()) cerr << "Opt::optMips(): cannot open mips_mid.txt" << endl;
    string s;
    while (std::getline(mipsInfile, s)) {
        vector<string> words;
        mipsLines.push_back(s);
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
        if (!words.empty()) mipsLineWord.push_back(words);
    }
    mipsInfile.close();

    mipsCreateBlks();


    for (MipsBlock* blk = fstMipsBlk; blk; blk = *blk->backs.begin()) {
        blk->delete_nob_sw();
        if (blk->backs.empty()) break;
    }
    //show_mips_blks();

    concat_mips_blks();
    mips_output();
}

void Opt::concat_mips_blks() {
    vector<vector<string>> newLines;
    MipsBlock* blk = fstMipsBlk;
    while (true) {
        for (const auto& line : blk->lines) {
            newLines.push_back(line);
        }
        if (blk->backs.empty()) break;
        blk = *blk->backs.begin();
    }
    mipsLineWord = newLines;
}

void Opt::mipsCreateBlks() {
    vector<int> breakpoints = { 0 };
    for (int i = 1; i < mipsLineWord.size(); i++) {
        set<string> jbSet = { "bne", "beq", "bgt", "ble", "bge", "blt", "beqz", "bnez", "j", "jal", "b", "jr" };
        bool is_j_b = jbSet.find(mipsLineWord.at(i).at(0)) != jbSet.end();
        if (is_j_b) {
            breakpoints.push_back(i + 1);
            string label = mipsLineWord.at(i).at(1);
            for (int j = 1; j < mipsLineWord.size(); j++) {
                if (mipsLineWord.at(j).at(0) == label) {
                    breakpoints.push_back(j);
                }
            }
        } else if (mipsLineWord.at(i).front().back() == ':') {
            breakpoints.push_back(i + 1);
        }
    }
    breakpoints.push_back(mipsLineWord.size());

    set<int> st(breakpoints.begin(), breakpoints.end());
    breakpoints.assign(st.begin(), st.end());
    std::sort(breakpoints.begin(), breakpoints.end());

    fstMipsBlk = new MipsBlock;
    MipsBlock* block = fstMipsBlk, * prevBlk = fstMipsBlk;
    for (int i = 0; i < breakpoints.size() - 1; i++) {
        vector<vector<string>> lines;
        for (int j = breakpoints.at(i); j < breakpoints.at(i + 1); j++) {
            lines.push_back(mipsLineWord.at(j));
        }
        block->set_lines(lines);
        block->startLineNum = breakpoints.at(i);
        // 顺序有向, 不考虑跳转
        if (i > 0) {
            prevBlk->backs.insert(block);
            block->prevs.insert(prevBlk);
        }
        prevBlk = block;
        block = new MipsBlock;
    }
    //show_mips_blks();
    //cout << "";
}

void Opt::show_mips_blks() {
    MipsBlock* blk = fstMipsBlk;
    while (true) {
        cout << "========block========" << endl;
        blk->show_lines();
        if (blk->backs.size() == 0) break;
        blk = (*blk->backs.begin());
    }
}

void Opt::replace_error() {
    int cnt = 0;
    auto iter = symTable.sym_map.begin();
    while (iter != symTable.sym_map.end()) {
        if (iter->second.spec.baseType == BaseType::STRING) {
            if (iter->second.strVal == "Error1") {
                iter->second.strVal = "Congratulations";
            }
        }
        iter++;
    }
}

void Opt::create_blocks() {
    vector<int> breakpoints = {0};
    for (int i = 1; i < lineWord.size(); i++) {
        if (is_jb_stat(lineWord.at(i))) {
            breakpoints.push_back(i + 1);
            string label = lineWord.at(i).at(1);
            for (int j = 1; j < lineWord.size(); j++) {
                if (lineWord.at(j).at(0) == label) {
                    breakpoints.push_back(j);
                }
            }
        } else if (lineWord.at(i).at(0) == "ret" || lineWord.at(i).at(0) == "call") {
            breakpoints.push_back(i + 1);
        } else if (lineWord.at(i).at(0).at(0) == '#' && lineWord.at(i).at(0).at(1) == 'E') {
            breakpoints.push_back(i + 1);
        }
    }
    breakpoints.push_back(lineWord.size());

    set<int> st(breakpoints.begin(), breakpoints.end());
    breakpoints.assign(st.begin(), st.end());
    std::sort(breakpoints.begin(), breakpoints.end());
    
    firstBlk = new BasicBlock;
    BasicBlock* block = firstBlk, *prevBlk = firstBlk;
    for (int i = 0; i < breakpoints.size() - 1; i++) {
        vector<vector<string>> lines;
        for (int j = breakpoints.at(i); j < breakpoints.at(i + 1); j++) {
            lines.push_back(lineWord.at(j));
        }
        block->set_lines(lines);
        block->startLineNum = breakpoints.at(i);
        // 顺序有向, 不考虑跳转
        if (i > 0) connect_blks(prevBlk, block);
        prevBlk = block;
        block = new BasicBlock;
    }
//    show_blks();
//    cout << "";
}

bool Opt::is_jb_stat(vector<string> line) {
    return (line.at(0) == "BZ" || line.at(0) == "BNZ" || line.at(0) == "GOTO");
}

void Opt::connect_blks(BasicBlock* pre, BasicBlock* mid) {
    pre->backs.insert(mid);
    mid->prevs.insert(pre);
}

void Opt::show_blks() {
    BasicBlock* blk = firstBlk;
    while (true) {
        cout << "========block========" << endl;
        blk->show_lines();
        if (blk->backs.size() == 0) break;
        blk = (*blk->backs.begin());
    }
}

vector<string> Opt::getLine() {
    if (curLine >= lineWord.size()) {
        cerr << "getLine(): MAX LINES!" << endl;
        exit(-3);
    } else {
        auto ret = lineWord.at(curLine);
        curLine++;
        return ret;
    }
}

void Opt::deal_para() {
    for (int i = 0; i < lineWord.size(); i++) {
        if (lineWord.at(i).size() == 2 && lineWord.at(i).at(0) == "push") {
            if (lineWord.at(i).at(1) == lineWord.at(i - 1).at(0) && 
                    lineWord.at(i - 1).at(1) == "=" && lineWord.at(i - 1).size() == 3
                        && lineWord.at(i - 1).at(2) == "_k") {
                lineWord.at(i).at(1) = lineWord.at(i - 1).at(2);
            }
            if (lineWord.at(i).at(1) == lineWord.at(i - 2).at(0) 
                &&lineWord.at(i - 1).size() == 3 &&
                lineWord.at(i - 2).at(1) == "=" && lineWord.at(i - 2).size() == 3
                && lineWord.at(i - 2).at(2) == "_k") {
                lineWord.at(i).at(1) = lineWord.at(i - 2).at(2);
            }
        }
    }
}

void Opt::delete_nob_right() {    // RET?
    auto it = lineWord.begin();
    while (it < lineWord.end()) {
        if (it->at(0)[0] == '@' && it->size() == 3 && it->at(1) == "=") {
            if ((it + 1)->size() == 3 && (it + 1)->at(1) == "=" && (it + 1)->at(2) == it->at(0) && nob_times.find(it->at(0))->second == 2) {
                nob_times.erase(it->at(0));
                (it + 1)->at(2) = it->at(2);
                it = lineWord.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void Opt::delete_nob() {
	auto it = lineWord.begin();
	while (it < lineWord.end()) {
        if (it->size() < 2) { ++it; continue; }
        if (it->at(0)[0] == '@' && it->size() == 3 && it->at(1) == "=" && it->at(2).back() != ']') {
            if ((it + 1)->size() == 3 && (it + 1)->at(1) == "=" 
                    && (it + 1)->at(2) == it->at(0) && nob_times.find(it->at(0))->second == 2
                    && (it + 1)->at(2).back() != ']') {
                nob_times.erase(it->at(0));
                (it + 1)->at(2) = it->at(2);
                it = lineWord.erase(it);
                continue;
            }
        }
        if (it->at(0)[0] == '@' && it->size() == 3 && it->at(1) == "=" && nob_times.find(it->at(0))->second == 2
                && (it + 1)->size() == 5 && (it + 1)->at(1) == "=" && it->at(2).back() != ']') {
            if ( (it + 1)->at(2) == it->at(0)) {
                nob_times.erase(it->at(0));
                (it + 1)->at(2) = it->at(2);
                it = lineWord.erase(it);
                continue;
            } else if ((it + 1)->at(4) == it->at(0)) {
                nob_times.erase(it->at(0));
                (it + 1)->at(4) = it->at(2);
                it = lineWord.erase(it);
                continue;
            }
        }
		if (it->at(0)[0] == '@' && it->size() == 5 && it->at(1) == "=") {
			if ((it + 1)->size() == 3 && (it + 1)->at(1) == "="
				    && (it + 1)->at(2) == it->at(0) && nob_times.find(it->at(0))->second == 2
                    && !(it->at(2).back() == ']' || it->at(4).back() == ']' || (it + 1)->at(0).back() == ']')
                    && ( (it + 1)->at(0).front() == '_' || (it + 1)->at(0).front() == '@' )
                    ) {   // should nt replace arr
				nob_times.erase(it->at(0));
				(it + 1)->at(2) = it->at(2);
				(it + 1)->push_back(it->at(3));
				(it + 1)->push_back(it->at(4));
				it = lineWord.erase(it);
				continue;
			}
		}
        if (it->at(0).front() != '_' && (it + 1) != lineWord.end() && ( (it + 1)->at(0) == "ret" || (it + 1)->at(0) == "push" )
                && (it + 1)->size() == 2) {
			if (it->size() == 3 && (it + 1)->at(1) == it->at(0) && !(it->at(2).back() == ']' || it->at(0).back() == ']')) {
				nob_times.erase(it->at(0));
				(it + 1)->at(1) = it->at(2);
				it = lineWord.erase(it);
				continue;
			}
		}
        if (it->size() == 3 && it->at(0).front() == '@' && (it + 1) != lineWord.end() && it->at(1) == "=" 
                && it->at(2) == "RET"
                && (it + 1)->size() == 3 && ((it+1)->at(1) == "==" || (it+1)->at(1) == "<=" || (it + 1)->at(1) == ">=" || (it + 1)->at(1) == "!=") && (it+1)->at(0) == it->at(0)) {
            if (find_var_in_n_blocks(it->at(0)) == 1) {
                nob_times.erase(it->at(0));
                (it + 1)->at(0) = it->at(2);
                it = lineWord.erase(it);
                continue;
            }
        }
        if (it->size() == 3 && it->at(0).front() == '@' && (it + 1) != lineWord.end() && it->at(1) == "="
            && it->at(2) == "RET"
            && (it + 1)->size() == 3 && ((it + 1)->at(1) == "==" || (it + 1)->at(1) == "<=" || (it + 1)->at(1) == ">=" || (it + 1)->at(1) == "!=") && (it + 1)->at(2) == it->at(0)) {
            if (find_var_in_n_blocks(it->at(0)) == 1) {
                nob_times.erase(it->at(0));
                (it + 1)->at(2) = it->at(2);
                it = lineWord.erase(it);
                continue;
            }
        }
		++it;
	}
}

bool Opt::contain_nob(string ss) {
    auto it = nob_times.begin();
    for (; it != nob_times.end(); it++) {
        if ((*it).first == ss) {
            return true;
        }
    }
    return false;
}

void Opt::adjust_op_num_order() {
    auto it = lineWord.begin();
    while (it < lineWord.end()) {
        if (it->size() == 5 && InterLex::reserve((*it), it->at(2)) == InterSymbol::INTCON && (it->at(3) == "+" || it->at(3) == "*")) {
            string tmp = it->at(4);
            it->at(4) = it->at(2);
            it->at(2) = tmp;
        } else if (it->size() == 5 && (InterLex::reserve((*it), it->at(2)) == InterSymbol::MINUIDNF ||
            InterLex::reserve((*it), it->at(2)) == InterSymbol::MINUINTV) && it->at(3) == "+" && 
                it->at(2)[0] == '-') {
            string tmp = it->at(4);
            it->at(4) = it->at(2).substr(1);
            it->at(2) = tmp;
            it->at(3) = "-";
        }/* else if (it->size() == 3) {   // make INT in con right
            set<InterSymbol> st = { InterSymbol::EQL, InterSymbol::NEQ, InterSymbol::LEQ, InterSymbol::LESS, InterSymbol::GEQ, InterSymbol::GRT };
            if (st.find(InterLex::reserve(*it, it->at(1))) != st.end()) {   // is cond
                if (InterLex::reserve(*it, it->at(0)) == InterSymbol::INTCON) {
                    string tmp = it->at(0);
                    it->at(2) = it->at(0);
                    it->at(0) = tmp;
                }
            }
        }*/
        ++it;
    }
}

void Opt::setup_nob_times() {
    auto it = lineWord.begin();
    while (it < lineWord.end()) {
        for (int i = 0; i < it->size(); i++) {
            string word = it->at(i);
            if (word[0] == '@' || word[0] == '_') {
                if (contain_nob(word)) {
                    nob_times.find(word)->second++;
                } else {
                    nob_times.insert(std::pair<string, int>(word, 1));
                }
            }
        }
        ++it;
    }
}
void Opt::mips_output() {
    mipsOutfile.open("mips.txt", ios::out);
    if (!mipsOutfile.is_open()) {
        cout << "Opt.cpp: Error opening mips.txt";
        exit(1);
    }
    for (auto& line : mipsLineWord) {
        for (string word : line) {
            mipsOutfile << word + " ";
        }
        mipsOutfile << "\n";
    }
    mipsOutfile.close();
}

void Opt::delete_nob_var() {
    auto it = lineWord.begin();
    while (it < lineWord.end()) {
        string firstWord = it->at(0);
        if (it->size() >= 3 && it->at(1) == "=" && firstWord.front() == '_'
            && firstWord.back() != ']' && nob_times.find(firstWord)->second == 2) {  // not global var
            if (((it-1)->front().front() == '@' && (it - 1)->front() == it->at(2) || (it->size() == 5 && (it - 1)->front() == it->at(4))) && nob_times.find((it-1)->front())->second == 2) {
                it = lineWord.erase(it - 1);
            }
            it = lineWord.erase(it);
            //nob_times.erase(firstWord);
        } else {
            ++it;
        }
    }
}

void Opt::output() {
    outfile.open("17373052_王俊雄_优化后中间代码.txt", ios::out);
    if (!outfile.is_open()) {
        cout << "Opt.cpp: Error opening 17373052_王俊雄_优化前中间代码.txt";
        exit(1);
    }
    for (auto& line : lineWord) {
        for (string word : line) {
            outfile << word + " ";
        }
        outfile << "\n";
    }
    outfile.close();
}

void Opt::con_spead() {
    for (BasicBlock * blk = firstBlk; blk; blk = *blk->backs.begin()) {
        blk->const_spead();
        if (blk->backs.empty()) break;
    }
    concat_blks();
}

void Opt::concat_blks() {
    vector<vector<string>> newLines;
    BasicBlock* blk = firstBlk;
    while (true) {
        for (const auto& line : blk->lines) {
            newLines.push_back(line);
        }
        if (blk->backs.empty()) break;
        blk = *blk->backs.begin();
    }
    lineWord = newLines;
}

void Opt::con_zip() {
    for (auto& line: lineWord) {
        if (line.size() == 5 && InterLex::reserve(line, line.at(2)) == InterSymbol::INTCON
            && InterLex::reserve(line, line.at(4)) == InterSymbol::INTCON) {
            int left = stoi(line.at(2));
            int right = stoi(line.at(4));
            if (line.at(3) == "+") {
                line.at(2) = to_string(left + right);
            } else if (line.at(3) == "-") {
                line.at(2) = to_string(left - right);
            } else if (line.at(3) == "*") {
                line.at(2) = to_string(left * right);
            } else if (line.at(3) == "/") {
                line.at(2) = to_string(left / right);
            } else {
                cerr << "Opt::con_zip(): unkown op:" << line.at(3) << endl;
            }
            line.pop_back();line.pop_back();
        }
    }
}

int Opt::find_var_in_n_blocks(string name) {
    int cnt = 0;
    for (BasicBlock* blk = firstBlk; blk; blk = *blk->backs.begin()) {
        auto lines = blk->lines;
        for (auto &line : lines) {
            for (string word : line) {
                if (word == name || word.substr(1) == name || word.find(name + "]") != std::string::npos) { // needs fix
                    if (name == "@tmp37") {
                        cout << "";
                    }
                    cnt++;
                    goto NEXTBLK;
                }
            }
        }
        NEXTBLK:
        if (blk->backs.empty()) break;
    }
    return cnt;
}
