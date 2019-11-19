#pragma once
#include "table.h"
#include <string>

enum class InterType{
	DEFAULT, INTER, INTCONST, CHARCONST, STRCONST, SYMC	// maybe array
};

class InterSym {
private:
public:
	int intcon;
	char charcon;
	string strcon;
	InterType interType = InterType::DEFAULT;   // created inter sym by compiler
	SymbolC* symc;
	string name;
    BaseType baseType;

	bool singleChar = false;

	InterSym();
	void set_inter_type(InterType it);
	void set_tmp_name();
};

