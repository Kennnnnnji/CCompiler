#include "MipsBlock.h"

void MipsBlock::set_lines(vector<vector<string>> lines) {
    this->lines = lines;
}

void MipsBlock::show_lines() {
    for (auto& line : lines) {
        for (auto& word : line) {
            cout << word << " ";
        }
        cout << endl;
    }
}

void MipsBlock::delete_nob_sw() {
    auto it = lines.begin();
    int i = 0;
    while (it != lines.end()) {
        auto line = it;
        if (line->size() == 3 && line->at(0) == "sw") {
            auto it2 = lines.begin();
            bool isNob = false;
            while (it2 != it) {
                if (it2->size() == 3 && it2->at(0) == "lw" && it2->at(1) == it->at(1) && it2->at(2) == it->at(2) ) {
                    if (line->at(2) == "-528($sp)") {
                        cout << "";
                    }
                    isNob = true;
                    string regName = it2->at(1);
                    auto it3 = it2 + 1;
                    while (it3 != it) {
                        set<string> exceptSet = { "div", "sw", };   // doesnt change reg
                        if (it3->size() > 1 
                            && it3->at(1) == regName 
                            && exceptSet.find(it3->front()) == exceptSet.end()) {
                            isNob = false;
                            goto GETNOB;
                        }
                        it3++;
                    }
                    break;
                }
                it2++;
            }
            GETNOB:
            if (isNob) {
                it = lines.erase(it);
                continue;
            }
        }
        it++;
    }
}
