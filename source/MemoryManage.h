#pragma once

#include <vector>
#include <string>
using namespace std;



class MemoryTableItem {
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
