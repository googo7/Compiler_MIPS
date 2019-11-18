#pragma once

#include <string>
#include "MidCode.h"
#include <unordered_map>
#include "SymbolTable.h"
#include "MemoryManage.h"
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

	vector<RegTableItem> reg_table;
	vector<string> free_s_reg = { "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6","$s7" };
	vector<string> free_t_reg = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6","$t7" };
	vector<string> use_s_reg = {};
	vector<string> use_t_reg = {};

	string lookup(string iden, string reg = "$t8");
	string alloc(string i, string reg);
	vector<RegTableItem> clear_t();
	vector<RegTableItem> clear_s();
	int toi(string);
};