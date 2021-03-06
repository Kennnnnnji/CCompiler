#include "cError.h"

CError::CError() {
	errfile.open("error.txt");
	if (!errfile.is_open()) {
		cerr << "CANNOT OPEN ERROR.TXT" << endl;
	}
}

void CError::prt_error(int line_num, const string& message) {
	errfile << line_num << ' ' << message << '\n';
}

void CError::catch_e(int line_num, const string& errType) {
	ERROR = true;
	prt_error(line_num, errType);
}
