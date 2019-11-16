#pragma once

#include <vector>
#include <string>
using namespace std;



class MemoryTableItem {
public:
	MemoryTableItem();
	MemoryTableItem(string, string, int, int);
	string iden;
	string func;
	int isLocal;
	int arr;
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
	vector<string> free_s_reg = { "s0", "s1", "s2", "s3", "s4", "s5", "s6","s7" };
	vector<string> free_t_reg = { "t0", "t1", "t2", "t3", "t4", "t5", "t6","t7" };
	vector<string> use_s_reg = {};
	vector<string> use_t_reg = {};

	RegTable();

	string lookup(string, int, string);
	int alloc(string i, string type);
};
