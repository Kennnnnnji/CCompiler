#include "GenInter.h"

GenInter::GenInter() {
	interfile.open("inter.txt");
}

void GenInter::prt(string s) {
	interfile << s;
}

void GenInter::prt_inter_sym(InterSym is) {
	interfile << "*" << is.name;
}

void GenInter::find_func_def(SymbolC func) {
	interfile << types[static_cast<int>(func.spec.baseType)] << " " << func.name << "()" << endl;
	for (int i = 0; i < (func.args).size(); i++) {
		SymbolC param = *func.args[i];
		interfile << "para " << types[static_cast<int>(param.spec.baseType)] << " " << param.name << endl;
	}
}

void GenInter::find_const_var_def(SymbolC constVar) {
	interfile << "const " << types[static_cast<int>(constVar.spec.baseType)] << " " << constVar.name << " = ";
	if (constVar.spec.baseType == BaseType::INT) {
		interfile << constVar.value;
	} else {
		interfile << "\'" << char(constVar.value) << "\'";
	}
	interfile << endl;
}

void GenInter::find_normal_var_def(SymbolC constVar) {
	interfile << "var " << types[static_cast<int>(constVar.spec.baseType)] << " " << constVar.name << endl;
}

string GenInter::gen_label() {
	static int cnt = 0;
	string label = "Label_" + to_string(cnt++);
	//interfile << label + " :" << endl;
	return label;
}
