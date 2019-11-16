#include "MipsGen.h"
#include "MemoryManage.h"
#include "FILEOperator.h"

RegTable reg_t;
extern vector<MemoryTableItem> memory_table;
MidCode mc_now;
string func_now = "main";
/*
	func
	para
	func_push_para
	func_call

	mult
	div
	BZ
	BNZ
	GOTO
	LABEL
	
	!=
	==
	<
	<=
	>=
	>
	scanf
	printf
	ret
	---------------
	add
	sub
	=
	[]
*/
void MipsGen::parse(MidCode mc) {
	mc_now = mc;
	string op = mc.op;
	string s1 = mc.s1;
	string s2 = mc.s2;
	string result = mc.result;
	if (mc.s1[0] == '\'')
		s1 = to_string((int)s1[1]);
	if (mc.s2[0] == '\'')
		s2 = to_string((int)s2[1]);
	if (op == "add" || op == "sub") {
		string result_reg = reg_t.lookup(result, 1, SorT(result)), s1_reg, s2_reg;
		if (!result_reg.size()) {
			result_reg = "t8";
		}
		if (is_digit(s1[0])) {
			if (is_digit(s2[0])) {
				int res = (op == "add") ? (stoi(s1) + stoi(s2)) : (stoi(s1) - stoi(s2));
				emit("li", result_reg, to_string(res), "");

			}
			else {
				s2_reg = reg_t.lookup(s2, 1, SorT(s2));
				if (!s2_reg.size()) {
					s2_reg = "t9";
					lw(s2_reg, s2);
				}
				if (op == "add")
					emit("addiu", result_reg, s2_reg, s1);
				else
					emit("addiu", result_reg, s2_reg, to_string(0 - stoi(s1)));
			}
		}
		else {
			if (is_digit(s2[0])) {
				s1_reg = reg_t.lookup(s1, 1, SorT(s1));
				if (!s1_reg.size()) {
					s1_reg = "t8";
					lw(s1_reg, s1);
				}
				if (op == "add")
					emit("addiu", result_reg, s1_reg, s2);
				else
					emit("addiu", result_reg, s1_reg, to_string(0 - stoi(s2)));
			}
			else {
				s1_reg = reg_t.lookup(s1, 1, SorT(s1)); 
				if (!s1_reg.size()) {
					s1_reg = "t8";
					lw(s1_reg, s1);
				}
				s2_reg = reg_t.lookup(s2, 1, SorT(s2));
				if (!s2_reg.size()) {
					s2_reg = "t9";
					lw(s2_reg, s2);
				}
				if(op == "add")
					emit("addu", result_reg, s1_reg, s2_reg);
				else
					emit("subu", result_reg, s1_reg, s2_reg);
			}
		}
		if (result_reg == "t8") {
			sw(result_reg, result);
		}
	}
	else if (op == "mult" || op == "div") {
		if (is_digit(s1[0])) {

		}
	}
	else if (op == "=") {
		if(s1 == result)
			return;
		if (s2 == "") {
			if (is_digit(s1[0])) {
				string result_reg = reg_t.lookup(result, 1, SorT(result));
				if (result_reg == "")
					result_reg = "t8";
				emit("li", result_reg, to_string(stoi(s1)), "");
				if (result_reg == "t8")
					sw(result_reg, result);
			}
			else {
				string result_reg = reg_t.lookup(result, 1, SorT(result));
				string s1_reg = reg_t.lookup(s1, 1, SorT(s1));
				if (result_reg == "")
					result_reg = "t8";
				if (s1_reg == "") {
					s1_reg = "t9";
					lw(s1_reg, s1);
				}
				emit("move", result_reg, s1_reg, "");
				if (result_reg == "t8")
					sw(result_reg, result);
			}
		}
		else {
			//result[s2] = s1
			if (is_digit(s1[0]))
				emit("li", "t8", s1, "");
			else {
				lw("t8", s1);
			}
			if (is_digit(s2[0]))
				emit("li", "t9", s2, "");
			else
				lw("t9", s2);
			emit("sll", "t9", "t9", "2");
			sw("t8", result, "t9");
		}
	}
	else if (op == "[]") {
		//result = s1[s2]
		string result_reg = reg_t.lookup(result, 1, SorT(result));
		if (!result_reg.size()) {
			result_reg = "t8";
		}
		if (is_digit(s2[0])) {
			emit("li", "t9", s2, "");
		}
		else {
			lw("t9", s2);
		}
		emit("sll", "t9", "t9", "2");
		lw(result_reg, s1, "t9");
		if (result_reg == "t8")
			sw(result_reg, result);
	}
}

void MipsGen::emit(string op, string s1, string s2, string s3) {
	string res;
	res = op;
	if (op == "lw" || op == "sw") {
		res += "\t$" + s1;
		res += ",\t" + s2;
		res += "\t($" + s3 + ")";
	}
	else if(op == "addu" || op == "subu"){
		res += "\t$" + s1;
		res += ",\t$" + s2;
		res += ",\t$" + s3;
	}
	else if (op == "addiu") {
		res += "\t$" + s1;
		res += ",\t$" + s2;
		res += ",\t" + s3;
	}
	else if (op == "li") {
		res += "\t$" + s1;
		res += ",\t" + s2;
	}
	else if (op == "move") {
		res += "\t$" + s1;
		res += ",\t$" + s2;
	}
	else if (op == "sll") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;
	}
	write_into_mfile(res);
}

string MipsGen::SorT(string s) {
	if (s[0] == 'x' && s[1] == 'x' && s[2] == 'j' && s[3] == '_' && s[4] == 't' && s[5] == 'e' && s[6] == 'm' && s[7] == 'p')
		return "t";
	else
		return "s";
}

void MipsGen::sw(string s, string id) {
	int addr = -1;
	int arr = 0;
	for (int i = 0; i < memory_table.size(); i++) {
		arr = memory_table[i].arr;
		if (memory_table[i].iden == id && memory_table[i].func == func_now) {
			addr = memory_table[i].arr << 2;
			break;
		}
		if (memory_table[i].iden == id && memory_table[i].isLocal == 0) {
			addr = memory_table[i].arr << 2;
			break;
		}
	}
	if (addr == -1) {
		memory_table.push_back(MemoryTableItem(id, "xxj_temp", 0, arr + 1));
		addr = (arr + 1) << 2;
	}
	emit("sw", s, to_string(addr), "0");
}

void MipsGen::sw(string s, string id, string reg) {
	int addr = -1;
	int arr;
	for (int i = 0; i < memory_table.size(); i++) {
		arr = memory_table[i].arr;
		if (memory_table[i].iden == id && memory_table[i].func == func_now) {
			addr = memory_table[i].arr << 2;
			break;
		}
		if (memory_table[i].iden == id && memory_table[i].isLocal == 0) {
			addr = memory_table[i].arr << 2;
			break;
		}
	}
	emit("sw", s, to_string(addr), reg);
}

void MipsGen::lw(string s, string id) {
	int addr = -1;
	int arr = 0;
	for (int i = 0; i < memory_table.size(); i++) {
		arr = memory_table[i].arr;
		if (memory_table[i].iden == id && memory_table[i].func == func_now) {
			addr = memory_table[i].arr << 2;
			break;
		}
		if (memory_table[i].iden == id && memory_table[i].isLocal == 0) {
			addr = memory_table[i].arr << 2;
			break;
		}
	}
	if (addr == -1) {
		memory_table.push_back(MemoryTableItem(id, "xxj_temp", 0, arr + 1));
		addr = (arr + 1) << 2;
	}
	//emit("lw", s, to_string(addr), "0");lw正常情况下不能够被其他变量调用
}

void MipsGen::lw(string s, string id, string reg) {
	int addr = -1;
	int arr = 0;
	for (int i = 0; i < memory_table.size(); i++) {
		arr = memory_table[i].arr;
		if (memory_table[i].iden == id && memory_table[i].func == func_now) {
			addr = memory_table[i].arr << 2;
			break;
		}
		if (memory_table[i].iden == id && memory_table[i].isLocal == 0) {
			addr = memory_table[i].arr << 2;
			break;
		}
	}
	emit("lw", s, to_string(addr), reg);
}

int MipsGen::is_digit(char ch) {
	return isdigit(ch) || ch == '+' || ch == '-';
}