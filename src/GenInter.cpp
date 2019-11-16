#include "GenInter.h"

GenInter::GenInter() {
	interfile.open("inter.txt");
}

void GenInter::find_func_def(SymbolC func) {
	interfile << types[static_cast<int>(func.spec.baseType)] << " " << func.name << "()" << endl;
	for (int i = 0; i < (func.args).size(); i++) {
		SymbolC param = *func.args[i];
		interfile << "para " << types[static_cast<int>(param.spec.baseType)] << " " << param.name << endl;

	}
}

template<class T>
int length_of_array(T& arr) {
	return sizeof(arr) / sizeof(arr[0]);
}

void GenInter::find_func_use(SymbolC func, SymbolC *paras, SymbolC ret) {
	for (int i = 0; i < length_of_array(paras); i++) {
		interfile << "push " << paras[i].name << endl;
	}
	interfile << "call " << func.name << endl;
	interfile << ret.name << " = RET" << endl;
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
	interfile << types[static_cast<int>(constVar.spec.baseType)] << " " << constVar.name << endl;
}
