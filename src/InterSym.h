#pragma once
#include "table.h"
#include <string>

enum class InterType{
	INTER, INTCONST, CHARCONST, STRCONST, SYMC	// maybe array
};

class InterSym {
private:
public:
	int intcon;
	char charcon;
	string strcon;
	InterType interType;   // created inter sym by compiler
	SymbolC* symc;
	string name;
    BaseType baseType;

	InterSym();
	void set_inter_type(InterType it);
	void set_tmp_name();
};

