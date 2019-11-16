#pragma once

#include <string>
#include "MidCode.h"
#include <unordered_map>
#include "SymbolTable.h"
using namespace std;

class MipsGen {
public:
	vector<MidCode> mc;
	unordered_map<string, string> string_map = {};
	void emit(string, string, string, string);
	void parse(MidCode mc);
	string SorT(string s);
	void sw(string s, string id);
	void lw(string s, string id);
	void sw(string s, string id, string reg);
	void lw(string s, string id, string reg);
	int is_digit(char);
	void predeal(vector<MidCode>);
	void push(string);
	void pop(string);
};