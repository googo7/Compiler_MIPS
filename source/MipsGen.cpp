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


	ret


	---------------
	add
	sub
	=
	[]
	mult
	div
	scanf
	printf	
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
			emit("li", "t8", s1, "");
		}
		else {
			string s1_reg = reg_t.lookup(s1, 1, SorT(s1));
			if (!s1_reg.size()) {
				s1_reg = "t8";
				lw(s1_reg, s1);
			}
		}
		if (is_digit(s2[0])) {
			emit("li", "t9", s2, "");
		}
		else {
			string s2_reg = reg_t.lookup(s2, 1, SorT(s2));
			if (!s2_reg.size()) {
				s2_reg = "t9";
				lw(s2_reg, s2);
			}
		}
		if (op == "mult") {
			emit("mult", "t8", "t9", "");
		}
		else if (op == "div") {
			emit("div", "t8", "t9", "");
		}
		string result_reg = reg_t.lookup(result, 1, SorT(result));
		if (result_reg.size())
			emit("mflo", result_reg, "", "");
		else {
			emit("mflo", "t8", "", "");
			sw("t8", result);
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
	else if (op == "BZ") {
		//BZ s1 s2
		//beq s1, $0, s2
		string s1_reg = reg_t.lookup(s1, 1, SorT(s1));
		if (s1_reg.size())
			emit("beq", s1_reg, "0", s2);
		else {
			lw("t8", s1);
			emit("beq", "t8", "0", s2);
		}
	}
	else if (op == "BNZ") {
		string s1_reg = reg_t.lookup(s1, 1, SorT(s1));
		if (s1_reg.size())
			emit("bne", s1_reg, "0", s2);
		else {
			lw("t8", s1);
			emit("bne", "t8", "0", s2);
		}
	}
	else if (op == "GOTO") {
		emit("j", s1, "", "");
	}
	else if (op == "LABEL") {
	emit("LABEL", s1, s2, "");
	}
	else if (op == "<" || op == "<=" || op == ">=" || op == ">" || op == "==" || op == "!=") {
		string result_reg = reg_t.lookup(result, 1, SorT(result)), s1_reg, s2_reg;
		if (!result_reg.size()) {
			result_reg = "t8";
		}
		if (is_digit(s1[0])) {
			if (is_digit(s2[0])) {
				int res = 0;
				if (op == "<")
					res = (stoi(s1) < stoi(s2));
				else if(op == "<=")
					res = (stoi(s1) <= stoi(s2));
				else if (op == ">=")
					res = (stoi(s1) >= stoi(s2));
				else if (op == ">")
					res = (stoi(s1) > stoi(s2));
				else if (op == "==")
					res = (stoi(s1) == stoi(s2));
				else if (op == "!=")
					res = (stoi(s1) != stoi(s2));
				emit("li", result_reg, to_string(res), "");

			}
			else {
				s2_reg = reg_t.lookup(s2, 1, SorT(s2));
				if (!s2_reg.size()) {
					s2_reg = "t9";
					lw(s2_reg, s2);
				}
				if (op == "<")   // num < iden
					emit("sgt", result_reg, s2_reg, to_string(stoi(s1)));
				else if (op == "<=")
					emit("sge", result_reg, s2_reg, to_string(stoi(s1)));
				else if (op == ">=")
					emit("sle", result_reg, s2_reg, to_string(stoi(s1)));
				else if (op == ">")
					emit("slti", result_reg, s2_reg, to_string(stoi(s1)));
				else if (op == "==")
					emit("seq", result_reg, s2_reg, to_string(stoi(s1)));
				else if (op == "!=")
					emit("sne", result_reg, s2_reg, to_string(stoi(s1)));
			}
		}
		else {
			if (is_digit(s2[0])) {
				s1_reg = reg_t.lookup(s1, 1, SorT(s1));
				if (!s1_reg.size()) {
					s1_reg = "t8";
					lw(s1_reg, s1);
				}
				if (op == "<")   // num < iden
					emit("slti", result_reg, s1_reg, to_string(stoi(s2)));
				else if (op == "<=")
					emit("sle", result_reg, s1_reg, to_string(stoi(s2)));
				else if (op == ">=")
					emit("sge", result_reg, s1_reg, to_string(stoi(s2)));
				else if (op == ">")
					emit("sgt", result_reg, s1_reg, to_string(stoi(s2)));
				else if (op == "==")
					emit("seq", result_reg, s1_reg, to_string(stoi(s2)));
				else if (op == "!=")
					emit("sne", result_reg, s1_reg, to_string(stoi(s2)));
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
				if (op == "<")   // num < iden
					emit("slt", result_reg, s1_reg, s2_reg);
				else if (op == "<=")
					emit("sle", result_reg, s1_reg, s2_reg);
				else if (op == ">=")
					emit("sge", result_reg, s1_reg, s2_reg);
				else if (op == ">")
					emit("sgt", result_reg, s1_reg, s2_reg);
				else if (op == "==")
					emit("seq", result_reg, s1_reg, s2_reg);
				else if (op == "!=")
					emit("sne", result_reg, s1_reg, s2_reg);
			}
		}
		if (result_reg == "t8") {
			sw(result_reg, result);
		}

}
	else if (op == "printf") {
		//printf s1(string) s2(expr)
		if (s1.size()) {
			push("a0");
			string string_no = this->string_map[s1];
			emit("la", "a0", string_no, "");
			emit("li", "v0", "4", "");
			emit("syscall", "", "", "");
			pop("a0");
		}
		if (s2.size()) {
			string s2_reg = reg_t.lookup(s2, 1, SorT(s2));
			push("a0");
			if (s2_reg.size()) {
				emit("move", "a0", s2_reg, "");
			}
			else {
				lw("a0", s2);
			}
			int type = -1;
			for (int i = 0; i < memory_table.size(); i++) {
				if (memory_table[i].iden == s2 && memory_table[i].func == func_now) {
					type = memory_table[i].var_type;
					break;
				}
				if (memory_table[i].iden == s2 && memory_table[i].isLocal == 0) {
					type = memory_table[i].var_type;
					break;
				}
			}
			if (type == INT)
				emit("li", "v0", "1", "");
			else if (type == CHAR)
				emit("li", "v0", "11", "");
			emit("syscall", "", "", "");
			pop("a0");
		}
	}
	else if (op == "scanf") {
		int type = -1;
		for (int i = 0; i < memory_table.size(); i++) {
			if (memory_table[i].iden == s1 && memory_table[i].func == func_now) {
				type = memory_table[i].var_type;
				break;
			}
			if (memory_table[i].iden == s1 && memory_table[i].isLocal == 0) {
				type = memory_table[i].var_type;
				break;
			}
		}
		if (type == INT) {
			emit("li", "v0", "5", "");
		}
		else {
			emit("li", "v0", "12", "");
		}
		emit("syscall", "", "", "");
		string s1_reg = reg_t.lookup(s1, 1, SorT(s1));
		if (!s1_reg.size()) {
			s1_reg = "t8";
		}
		emit("move", s1_reg, "v0", "");
		if (s1_reg == "t8") {
			sw(s1_reg, s1);
		}
	}
}

void MipsGen::emit(string op, string s1, string s2, string s3) {
	string res;
	res = "\t" + op;
	if (op == "lw" || op == "sw") {
		res += "\t$" + s1;
		res += ",\t" + s2;
		res += "\t($" + s3 + ")";
	}
	else if (op == "addu" || op == "subu") {
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
		res += "\t$" + s1;
		res += ",\t$" + s2;
		res += ",\t" + s3;
	}
	else if (op == "mult" || op == "div") {
		res += "\t$" + s1;
		res += ",\t$" + s2;
	}
	else if (op == "mflo") {
		res += "\t$" + s1;
	}
	else if (op == "bne" || op == "beq") {
		res += "\t$" + s1;
		res += ",\t$" + s2;
		res += ",\t" + s3;
	}
	else if (op == "j") {
		res += "\t" + s1;
	}
	else if (op == "LABEL") {
		res = s1 + s2 + ":";
	}
	else if (op == "slt" || op == "slti" || op == "sge" || op == "sle" || op == "sgt" || op == "sne" || op == "seq") {
		res += "\t$" + s1;
		res += ",\t$" + s2;
		if (is_digit(s3[0]))
			res += ",\t" + s3;
		else
			res += ",\t$" + s3;
	} 
	else if (op == "string") {
		res = op + s2 + ":";
		res += "\t.asciiz";
		res += "\t" + s1;
	}
	else if (op == "la") {
		res += "\t$" + s1;
		res += ",\t" + s2;
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
		memory_table.push_back(MemoryTableItem(id, "xxj_temp", 0, arr + 1, INT));
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
		memory_table.push_back(MemoryTableItem(id, "xxj_temp", 0, arr + 1, INT));
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

void MipsGen::predeal(vector<MidCode> mc) {
	write_into_mfile(".data:");
	this->mc = mc;
	static int string_cnt = 0;
	for (int i = 0; i < mc.size(); i++) {
		if (mc[i].op == "printf") {
			if (mc[i].s1[0] == '\"') {
				emit("string", mc[i].s1, to_string(string_cnt), "");
				this->string_map[mc[i].s1] = ("string" + to_string(string_cnt));
				string_cnt++;
			}
		}
	}
	write_into_mfile(".text:");
}

void MipsGen::push(string reg) {
	emit("sw", reg, "0", "sp");
	emit("addiu", "sp", "sp", "-4");
}

void MipsGen::pop(string reg) {
	emit("addiu", "sp", "sp", "4");
	emit("lw", reg, "0", "sp");
}