#include "BasicBlock.h"
#include "header.h"

void BasicBlock::set_lines(vector<vector<string>> lines) {
    this->lines = lines;
}

void BasicBlock::show_lines() {
    for (auto &line : lines) {
        for (auto &word : line) {
            cout << word << " ";
        }
        cout << endl;
    }
}


/*
 *  a =	0
    b = a
    -->
    b = 0
 */
void BasicBlock::const_spead() {
    auto it = lines.begin();
    int i = 0;
    while (it != lines.end()) {
        auto line = it;
        if (line->size() == 3 && line->at(1) == "=" && line->at(2)[0] == '@') {
            auto it2 = lines.begin();
            while (it2 != it) {
                set<InterSymbol> conSet = {InterSymbol::INTCON, InterSymbol::CHARCON};
                if (it2->size() == 3 && it2->at(1) == "=" && it2->at(0)[0] == '@'
                        && conSet.find(InterLex::reserve(*it, it2->at(2)))
                            != conSet.end()) {
                    line->at(2) = it2->at(2); // b=@a -> b=0
                    if (find_var_times(line->at(2)) == 2) { // remove @a=0
                        lines.erase(it2);
                    }
                    break;
                }
                it2++;
            }

        }
        it++;
    }
}

int BasicBlock::find_var_times(const string &var) {
    int cnt = 0;
    for (const auto &line : lines) {
        for (const auto &s : line) {
            if (s == var) {
                cnt++;
            }
        }
    }
    return cnt;
}
