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
	string iden;
	int isHit = 0;
	string reg;
	int isLocal = 0;
};


class RegTable {
public:

};
