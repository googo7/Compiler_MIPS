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


using namespace std;
class er_func_info {
public:
	string iden;
	vector<int> v_table;
	int return_type;
	er_func_info();
	er_func_info(string, vector<int>, int);
};

class er_var_info {
public:
	string iden;
	int isConst;
	int _type;
	int value;
	int isArray; //Êý×éflag
	er_var_info();
	er_var_info(string, int, int, int);
};
class er_Symtab {
public:
	vector<er_func_info> func_table;
	vector<er_var_info> global_var_table;
	vector<er_var_info> local_var_table;
	bool global_lookup(string);
	bool local_lookup(string);
	bool lookup(string);
	er_func_info func_lookup(string);
	er_var_info var_lookup(string);
	bool func_push(string, vector<int>, int);
	bool global_push(string, int, int, int v = 0);
	bool local_push(string, int, int, int v = 0);
	void clear();
};
