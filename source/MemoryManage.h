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
	//������ɱ����ԼĴ�����ӳ���ϵ�����ݽṹ
public:
	vector<vector<RegTableItem>> reg_table;
	RegTable();
	void push(vector<RegTableItem>);
	vector<RegTableItem> pop(void);
	vector<RegTableItem> top(void);
	string lookup(string, int, string);
	int alloc(string i, string type);
};
