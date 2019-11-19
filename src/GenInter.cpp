#include "GenInter.h"

GenInter::GenInter() {
	interfile.open("inter.txt", ios::in|ios::out|ios::trunc);
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
	//interfile << label + " :" << endl;
	return label;
}
