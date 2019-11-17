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
	var_info lookup(string func_name, string iden = "$t8");
	int lookup_addr(string func_name, string iden);
	int top_addr(string func_name);
};

class RegTableItem {
public:
	RegTableItem();
	RegTableItem(string, int, var_info);
	string reg;
	int isHit = 0;
	var_info var;
};