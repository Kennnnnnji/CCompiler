#pragma once
#include <string>

enum class RegType {
    ZERO, AT, V, A, T, S, GP, SP, FP, RA/*
    zero;
    Register at;
    Register v[2];
    Register a[4];
    Register t[10];	// t0-t7, s0-s7, t8-t9
    Register s[8];
    Register gp, sp, fp, ra;*/
};

class Register {
public:
    RegType type;
    int id;
	std::string name = "";
	int value{};
	bool idle{};
    bool protect = false;

	Register(RegType t, int id);
	std::string sid();
};

