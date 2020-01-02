#include "Register.h"

Register::Register(RegType t, int id) {
    type = t;
    this->id = id;
    idle = true;
	name = "$";
	switch (t) {
		case RegType::ZERO:
			name += "zero";
			break;
		case RegType::AT:
			name += "at";
			break;
		case RegType::V:
			name += "v" + std::to_string(id);
			break;
		case RegType::A:
			name += "a" + std::to_string(id);
			break;
		case RegType::T:
			name += "t" + std::to_string(id);
			break;
		case RegType::S:
			name += "s" + std::to_string(id);
			break;
		case RegType::GP:
			name += "gp";
			break;
		case RegType::SP:
			name += "sp";
			break;
		case RegType::FP:
			name += "fp";
			break;
		case RegType::RA:
			name += "ra";
			break;
		default:
			break;
	}
}

std::string Register::sid() {
    return std::to_string(id);
}
