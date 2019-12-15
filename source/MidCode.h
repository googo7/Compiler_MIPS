#pragma once
#include <string>
#include <vector>
#include "TkAnalysis.h"


using namespace std;
class MidCode {
public:
	MidCode();
	MidCode(string, string, string, string, int i = 1);
	string op = "";
	string s1 = "";
	string s2 = "";
	string result = "";
	int res_type = 1;
	string out();
};

class inline_func {
public:
	inline_func();
	inline_func(string, vector<string>, string, vector<MidCode>);
	string func_name;
	vector<string> para;
	string ret;
	vector<MidCode> func_mc;
};


class MidCodeGen {
public:
	vector<MidCode> mc;
	vector<inline_func> inline_func_info;
	vector<vector<string>> mips_code;
	MidCodeGen();
	void push(string, string, string, string, int i = 1);
	void parse(string, vector<token_info>, int i = 0);
	string gen_temp(int type = 1);
	string get_last_result(int flag = 0);
	int get_last_type(void);
	void out();
	void op_inline();
	void op_compare();
	void op_block();
	void op_kui();
	int check_use(string iden, string begin);
};