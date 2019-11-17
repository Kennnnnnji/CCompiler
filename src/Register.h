#pragma once
class Register {
public:
	static int status[32];
	int zero;
	int at;
	int v[2];
	int a[4];
	int t[8];
	int s[8];
	int tt[2];	// t8, t9
	int gp, sp, fp, ra;
};

