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
	string iden;
	int isHit = 0;
	string reg;
	int isLocal = 0;
};


class RegTable {
	//用于完成变量对寄存器的映射关系的数据结构
public:
	vector<vector<RegTableItem>> reg_table;
	RegTable();
	void push(vector<RegTableItem>);
	vector<RegTableItem> pop(void);
	vector<RegTableItem> top(void);
	string lookup(string, int, string);
	int alloc(string i, string type);
};
