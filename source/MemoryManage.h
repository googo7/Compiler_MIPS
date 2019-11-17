#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "SymbolTable.h"
using namespace std;



class MemoryTable {
public:
	MemoryTable();
	unordered_map<string, vector<var_info>> map;
	vector<string> temp;
	void push(string func_name, var_info vi);
	void push(string);
	void push(string func_name, string temp_name);
	var_info lookup(string func_name, string iden);
	int lookup_addr(string func_name, string iden);
};

class RegTableItem {
public:
	RegTableItem();
	RegTableItem(string, int, string, int);
	string iden;
	int isHit = 0;
	string reg;
	int isLocal = 0;
};


class RegTable {
	//用于完成变量对寄存器的映射关系的数据结构
public:
	vector<RegTableItem> reg_table;
	vector<string> free_s_reg = { "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6","$s7" };
	vector<string> free_t_reg = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6","$t7" };
	vector<string> use_s_reg = {};
	vector<string> use_t_reg = {};

	RegTable();

	string lookup(string, int, string);
	int alloc(string i, string type);
	void clear_t();
	void clear_s();
};
