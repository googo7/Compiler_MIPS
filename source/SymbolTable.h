#pragma once
/*
	func(global)
	------------------------------------------------
		iden	|	variable_table	|	return_type
	------------------------------------------------

	global var
	---------------------------------------------------------
		iden	|		isConst		|	type	|	value
	---------------------------------------------------------

	local var
	---------------------------------------------------------
		iden	|		isConst		|	type	|	value
	---------------------------------------------------------
*/

#include <string>
#include <unordered_map>
#include <vector>
#define VOID 0
#define INT 1
#define CHAR 2
#define TEMP 3

using namespace std;
class func_info {
public:
	string iden;
	vector<int> v_table;
	int return_type;
	func_info();
	func_info(string, vector<int>, int);
};

class var_info {
public:
	string iden;
	int isConst;
	int _type = -1;
	int value;
	int addr;
	int size = 1;
	var_info();
	var_info(string, int, int, int, int);
};
class Symtab {
public:
	vector<func_info> func_table;
	vector<var_info> global_var_table;
	vector<var_info> local_var_table;
	bool global_lookup(string);
	bool local_lookup(string);
	bool lookup(string);
	func_info func_lookup(string);
	var_info var_lookup(string);
	bool func_push(string, vector<int>, int);
	bool global_push(string, int, int, int v, int a);
	bool local_push(string, int, int, int v, int a);
	void adjust_size(string, int);
	void clear();
};
