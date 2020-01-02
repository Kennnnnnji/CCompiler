#include "GenInter.h"

GenInter::GenInter() {
    locale::global(locale(""));
	interfile.open("17373052_王俊雄_优化前中间代码.txt", ios::out|ios::trunc);
	if (!interfile.is_open()) {
		cout << "GenInter.cpp: Error opening 17373052_王俊雄_优化前中间代码.txt";
		exit(1);
	}
}

void GenInter::prt(string s) {
	interfile << s;
}

void GenInter::find_func_def(const SymbolC& func) {
	interfile << types[static_cast<int>(func.spec.baseType)] << " _" << func.name << "()" << endl;
	for (auto & arg : (func.args)) {
		SymbolC param = *arg;
		interfile << "para " << types[static_cast<int>(param.spec.baseType)] << " _" << param.name << endl;
	}
}

void GenInter::find_const_var_def(SymbolC constVar) {
	interfile << "const " << types[static_cast<int>(constVar.spec.baseType)] << " _" << constVar.name << " = ";
	if (constVar.spec.baseType == BaseType::INT) {
		interfile << constVar.value;
	} else {
		interfile << "\'" << char(constVar.value) << "\'";
	}
	interfile << endl;
}

void GenInter::find_normal_var_def(SymbolC constVar) {
	interfile << "var " << types[static_cast<int>(constVar.spec.baseType)] << " _" << constVar.name << endl;
}

string GenInter::gen_label() {
	static int cnt = 0;
	string label = "#Label_" + to_string(cnt++);
	return label;
}

string GenInter::gen_label_loop_start() {
	string label = "#LOOP_STT_" + to_string(++loopcnt);
	return label;
}

string GenInter::gen_label_loop_end() {
	string label = "#LOOP_END_" + to_string(loopcnt);
	return label;
}

string GenInter::gen_func_label(string fname) {
	return "#Func_" + fname;
}

string GenInter::gen_func_end(string fname) {
	return "#End_" + fname;
}
