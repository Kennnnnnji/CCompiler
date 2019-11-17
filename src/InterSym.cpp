#include "InterSym.h"

InterSym::InterSym() {
	static int tempVarCnt = 0;
}

void InterSym::set_inter_type(InterType it) {
	interType = it;
}

void InterSym::set_tmp_name() {
	static int cnt = 0;
	name = "@tmp" + to_string(cnt++);
}


