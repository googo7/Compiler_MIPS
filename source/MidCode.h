#pragma once
#include <string>
#include <vector>
#include "TkAnalysis.h"


using namespace std;
class MidCode {
public:
	MidCode();
	MidCode(string, string, string, string);
	string op;
	string s1;
	string s2;
	string result;
	string out();
};

class MidCodeGen {
public:
	vector<MidCode> mc;
	MidCodeGen();
	void push(string, string, string, string);
	void parse(string, vector<token_info>, int i = 0);
	string gen_temp(int type = 1);
	string get_last_result(void);
	void out();
};