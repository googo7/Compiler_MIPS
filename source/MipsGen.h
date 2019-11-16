#pragma once

#include <string>
#include "MidCode.h"
using namespace std;

class MipsGen {
public:
	void emit(string, string, string, string);
	void parse(MidCode mc);
	string SorT(string s);
	void sw(string s, string id);
	void lw(string s, string id);
	void sw(string s, string id, string reg);
	void lw(string s, string id, string reg);
};