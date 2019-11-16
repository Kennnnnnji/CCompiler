#pragma once
#include "table.h"

enum class interSymType{

};

class InterSym {
private:
public:
    bool isInter;   // created inter sym by compiler
	SymbolC* symc;
	string name;
    BaseType baseType;
};

