#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

class CError {
private:
	ofstream errfile;
public:
	bool ERROR = false;
	CError();
	void prt_error(int line_num, const string& message);
	void catch_e(int line_num, const string& errType);
};
