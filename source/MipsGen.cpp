#include "MipsGen.h"
#include "FILEOperator.h"
#include "GMAnalysis.h"
#include <sstream>
extern MemoryTable memory_table;
extern gm_analyse gm;
MidCode mc_now;
vector<string> func_name_stack = {};
string func_now = "";
int divide = 0;
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
	//if (mc.s1[0] == '\'')
	//	s1 = to_string((int)s1[1]);
	//if (mc.s2[0] == '\'')
	//	s2 = to_string((int)s2[1]);
	if (op == "add" || op == "sub") {
		string result_reg = lookup(result), s1_reg, s2_reg;
		if (is_digit(s1[0])) {
			if (is_digit(s2[0])) {
				int res = (op == "add") ? (toi(s1) + toi(s2)) : (toi(s1) - toi(s2));
				emit("li", result_reg, to_string(res), "");
			}
			else {
				s2_reg = lookup(s2, "$t9");
				if (op == "add")
					emit("addiu", result_reg, s2_reg, s1);
				else {
					emit("subu", "$t9", "$0", s2_reg);
					emit("addiu", result_reg, "$t9", s1);
				}
			}
		}
		else {
			if (is_digit(s2[0])) {
				s1_reg = lookup(s1);
				if (op == "add")
					emit("addiu", result_reg, s1_reg, s2);
				else
					emit("addiu", result_reg, s1_reg, to_string(0 - toi(s2)));
			}
			else {
				s1_reg = lookup(s1); 
				s2_reg = lookup(s2, "$t9");
				if(op == "add")
					emit("addu", result_reg, s1_reg, s2_reg);
				else
					emit("subu", result_reg, s1_reg, s2_reg);
			}
		}
		if (result_reg == "$t8") {
			sw(result_reg, result);
		}
	}
	else if (op == "mult" || op == "div") {
		if (is_digit(s1[0])) {
			emit("li", "$t8", s1, "");
		}
		else {
			string s1_reg = lookup(s1);
			emit("move", "$t8", s1_reg, "");
		}
		
		if (is_digit(s2[0])) {
			emit("li", "$t9", s2, "");
		}
		else {
			string s2_reg = lookup(s2, "$t9");
			emit("move", "$t9", s2_reg, "");
		}
		if (op == "mult") {
			emit("mult", "$t8", "$t9", "");
		}
		else if (op == "div") {
			emit("div", "$t8", "$t9", "");
		}
		string result_reg =lookup(result, "$t8");
		emit("mflo", result_reg, "", "");
		if(result_reg == "$t8")
			sw("$t8", result);
	}
	else if (op == "=") {
		if(s1 == result)
			return;
		if (s2 == "") {
			if (is_digit(s1[0])) {
				string result_reg = lookup(result);
				emit("li", result_reg, to_string(toi(s1)), "");
				if (result_reg == "$t8")
					sw(result_reg, result);
			}
			else {
				string result_reg = lookup(result);
				string s1_reg = lookup(s1, "$t9");
				emit("move", result_reg, s1_reg, "");
				if (result_reg == "$t8")
					sw(result_reg, result);
			}
		}
		else {
			string s1_reg = "", s2_reg = "";
			//result[s2] = s1
			if (is_digit(s1[0])) {
				emit("li", "$t8", s1, "");
				s1_reg = "$t8";
			}
			else {
				s1_reg = lookup(s1);
			}
			if (is_digit(s2[0])) {
				emit("li", "$t9", s2, "");
				emit("sll", "$t9", "$t9", "2");
			}
			else {
				s2_reg = lookup(s2, "$t9");
				emit("sll", "$t9", s2_reg, "2");
			}
			
			sw(s1_reg, result, "$t9");
		}
	}
	else if (op == "[]") {
		//result = s1[s2]
		string result_reg = lookup(result);
		if (is_digit(s2[0])) {
			emit("li", "$t9", s2, "");
			emit("sll", "$t9", "$t9", "2");
		}
		else {
			string s2_reg = lookup(s2);
			emit("sll", "$t9", s2_reg, "2");
		}
		
		lw(result_reg, s1, "$t9");
		if (result_reg == "$t8")
			sw(result_reg, result);
	}
	else if (op == "BZ") {
		//BZ s1 s2
		//beq s1, $0, s2
		string s1_reg = lookup(s1);
		emit("beq", s1_reg, "$0", s2);
	}
	else if (op == "BNZ") {
		string s1_reg = lookup(s1);
		emit("bne", s1_reg, "$0", s2);

	}
	else if (op == "GOTO") {
		emit("j", s1, "", "");
	}
	else if (op == "LABEL") {
	emit("LABEL", s1, s2, "");
	}
	else if (op == "<" || op == "<=" || op == ">=" || op == ">" || op == "==" || op == "!=") {
		string result_reg = lookup(result), s1_reg = "", s2_reg = "";
		if (is_digit(s1[0])) {
			if (is_digit(s2[0])) {
				int res = 0;
				if (op == "<")
					res = (toi(s1) < toi(s2));
				else if(op == "<=")
					res = (toi(s1) <= toi(s2));
				else if (op == ">=")
					res = (toi(s1) >= toi(s2));
				else if (op == ">")
					res = (toi(s1) > toi(s2));
				else if (op == "==")
					res = (toi(s1) == toi(s2));
				else if (op == "!=")
					res = (toi(s1) != toi(s2));
				emit("li", result_reg, to_string(res), "");

			}
			else {
				s2_reg = lookup(s2, "$t9");
				if (op == "<")   // num < iden
					emit("sgt", result_reg, s2_reg, to_string(toi(s1)));
				else if (op == "<=")
					emit("sge", result_reg, s2_reg, to_string(toi(s1)));
				else if (op == ">=")
					emit("sle", result_reg, s2_reg, to_string(toi(s1)));
				else if (op == ">")
					emit("slti", result_reg, s2_reg, to_string(toi(s1)));
				else if (op == "==")
					emit("seq", result_reg, s2_reg, to_string(toi(s1)));
				else if (op == "!=")
					emit("sne", result_reg, s2_reg, to_string(toi(s1)));
			}
		}
		else {
			if (is_digit(s2[0])) {
				s1_reg = lookup(s1, "$t9");
				if (op == "<")   // num < iden
					emit("slti", result_reg, s1_reg, to_string(toi(s2)));
				else if (op == "<=")
					emit("sle", result_reg, s1_reg, to_string(toi(s2)));
				else if (op == ">=")
					emit("sge", result_reg, s1_reg, to_string(toi(s2)));
				else if (op == ">")
					emit("sgt", result_reg, s1_reg, to_string(toi(s2)));
				else if (op == "==")
					emit("seq", result_reg, s1_reg, to_string(toi(s2)));
				else if (op == "!=")
					emit("sne", result_reg, s1_reg, to_string(toi(s2)));
			}
			else {
				s1_reg = lookup(s1);
				s2_reg = lookup(s2, "$t9");
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
		if (result_reg == "$t8") {
			sw(result_reg, result);
		}

}
	else if (op == "printf") {
		//printf s1(string) s2(expr)
		if (s1.size()) {
			string string_no = this->string_map[s1];
			emit("la", "$a0", string_no, "");
			emit("li", "$v0", "4", "");
			emit("syscall", "", "", "");
		}
		if (s2.size()) {
			if (isdigit(s2[0]) || s2[0] == '+' || s2[0] == '-' ) {
				emit("li", "$a0", to_string(toi(s2)), "");
				emit("li", "$v0", "1", "");
				emit("syscall", "", "", "");
			}
			else if (s2[0] == '\'') {
				emit("li", "$a0", to_string((int)s2[1]), "");
				emit("li", "$v0", "11", "");
				emit("syscall", "", "", "");
			}
			else {
				string s2_reg = lookup(s2);
				emit("move", "$a0", s2_reg, "");
				int type = -1;
				type = memory_table.lookup(func_now, s2)._type;
				if (type == -1)
					type = memory_table.lookup("", s2)._type;
				if (type == INT || type == TEMP)//数组查找类型
					emit("li", "$v0", "1", "");
				else if (type == CHAR)
					emit("li", "$v0", "11", "");
				emit("syscall", "", "", "");
			}
		}
		emit("la", "$a0", "enter", "");
		emit("li", "$v0", "4", "");
		emit("syscall", "", "", "");
	}
	else if (op == "scanf") {
		int type = -1;
		type = memory_table.lookup(func_now, s1)._type;
		if (type == -1)
			type = memory_table.lookup("", s1)._type;
		if (type == INT) {
			emit("li", "$v0", "5", "");
		}
		else {
			emit("li", "$v0", "12", "");
		}
		emit("syscall", "", "", "");
		string s1_reg = lookup(s1);
		emit("move", s1_reg, "$v0", "");
		if (s1_reg == "$t8") {
			sw(s1_reg, s1);
		}
	}
	else if (op == "func") {
		//func int func_name
		emit("LABEL", s2, "", "");		
		push("$ra");

		func_now = s2;
	}
	else if (op == "para") {
		int num = gm.symtab.func_lookup(func_now).v_table.size();
		emit("lw", "$t8", to_string(8 + 4 * num - toi(s1)), "$sp");
		sw("$t8", s2);
	}
	else if (op == "func_push_para") {
	string s1_reg = "";
	if (is_digit(s1[0])) {
		s1_reg = "$t8";
		emit("li", "$t8", s1, "");
		}
	else {
		s1_reg = lookup(s1);
	}
		emit("sw", s1_reg, to_string(-((memory_table.top_addr(func_now) + toi(s2)) << 2)), "$sp");
		divide = ((memory_table.top_addr(func_now) + toi(s2)) << 2) + 4;
	}
	else if (op == "func_call") {
		vector<RegTableItem> clr_s = clear_s();
		vector<RegTableItem> clr_t = clear_t();
		for (int i = 0; i < clr_s.size(); i++) {
			sw(clr_s[i].reg, clr_s[i].var.iden);
		}
		for (int i = 0; i < clr_t.size(); i++) {
			sw(clr_t[i].reg, clr_t[i].var.iden);
		}//清理寄存器
		emit("sw", "$sp", to_string(-divide), "$sp");
		divide += 4;
		emit("addiu", "$sp", "$sp", to_string(-divide));
		emit("jal", s1, "", "");
		string result_reg = lookup(result);
		emit("move", result_reg, "$v0", "");
		if (result_reg == "$t8") {
			sw(result_reg, result);
		}
	}
	else if (op == "end_func") {
		vector<RegTableItem> clr_s = clear_s();
		vector<RegTableItem> clr_t = clear_t();
		pop("$ra");
		pop("$sp");
		emit("jr", "$ra", "", "");
	}
	else if (op == "ret") {
		if (!s1.size()) {
			if (func_now == "main")
				return;
			vector<RegTableItem> clr_s = clear_s();
			vector<RegTableItem> clr_t = clear_t();
			pop("$ra");
			pop("$sp");
			emit("jr", "$ra", "", "");
			return;
		}
	if (is_digit(s1[0])) {
		emit("li", "$v0", to_string(toi(s1)), "");
	}
	else {
		string s1_reg = lookup(s1);
		emit("move", "$v0", s1_reg, "");

	}
	vector<RegTableItem> clr_s = clear_s();
	vector<RegTableItem> clr_t = clear_t();
	pop("$ra");
	pop("$sp");
	emit("jr", "$ra", "", "");
}
	else if (op == "exit") {
	emit("li", "$v0", "10", "");
	emit("syscall", "", "", "");
}
}

void MipsGen::emit(string op, string s1, string s2, string s3) {
	string res;
	res = "\t" + op;
	if (op == "lw" || op == "sw") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += "\t(" + s3 + ")";
	}
	else if (op == "addu" || op == "subu") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;
	}
	else if (op == "addiu") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;
	}
	else if (op == "li") {
		res += "\t" + s1;
		res += ",\t" + s2;
	}
	else if (op == "move") {
		res += "\t" + s1;
		res += ",\t" + s2;
	}
	else if (op == "sll") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;
	}
	else if (op == "mult" || op == "div") {
		res += "\t" + s1;
		res += ",\t" + s2;
	}
	else if (op == "mflo") {
		res += "\t" + s1;
	}
	else if (op == "bne" || op == "beq") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;
	}
	else if (op == "j") {
		res += "\t" + s1;
	}
	else if (op == "LABEL") {
		res = s1 + s2 + ":";
	}
	else if (op == "slt" || op == "slti" || op == "sge" || op == "sle" || op == "sgt" || op == "sne" || op == "seq") {
		res += "\t" + s1;
		res += ",\t" + s2;
		res += ",\t" + s3;

	} 
	else if (op == "string") {
		res = op + s2 + ":";
		res += "\t.asciiz";
		for (int i = 0; i < s1.size(); i++) {
			if (s1[i] == '\\')
				res += '\\';
			res += s1[i];
		}
	}
	else if (op == "la") {
		res += "\t" + s1;
		res += ",\t" + s2;
	}
	else if(op == "jal" || op == "jr"){
		res += "\t" + s1;
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
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("sw", s, to_string(addr), "$gp");
		return;
	}
	emit("sw", s, to_string(-addr), "$sp");
}

void MipsGen::sw(string s, string id, string reg) {
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("sw", s, to_string(addr + 0x1800), reg);
		return;
	}
	emit("subu", "$t9", "$sp", "$t9");
	emit("sw", s, to_string(-addr), reg);
}

void MipsGen::lw(string s, string id) {
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("lw", s, to_string(addr), "$gp");
		return;
	}
	emit("lw", s, to_string(-addr), "$sp");//lw正常情况下不能够被中间临时变量调用
}

void MipsGen::lw(string s, string id, string reg) {
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("lw", s, to_string(addr + 0x1800), reg);
		return;
	}
	emit("subu", "$t9", "$sp", "$t9");
	emit("lw", s, to_string(-addr), reg);
}

int MipsGen::is_digit(char ch) {
	return isdigit(ch) || ch == '+' || ch == '-' || ch == '\'';
}

void MipsGen::predeal(vector<MidCode> mc) {
	write_into_mfile(".data:");
	write_into_mfile("\tenter: .asciiz \"\\n\"");
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
	write_into_mfile("j main");
}

void MipsGen::push(string reg) {
	emit("sw", reg, "0", "$sp");
	emit("addiu", "$sp", "$sp", "-4");
}

void MipsGen::pop(string reg) {
	emit("addiu", "$sp", "$sp", "4");
	emit("lw", reg, "0", "$sp");
}

string MipsGen::lookup(string iden, string reg) {
	var_info v;
	v = memory_table.lookup(func_now, iden);
	if (v._type == -1) {
		//global
		v = memory_table.lookup("", iden);
		emit("lw", reg, to_string(v.addr << 2), "$gp");
		return reg;
	}
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].isHit) {
			v = this->reg_table[i].var;
			if (v.iden == iden)
				return this->reg_table[i].reg;
		}
	}
	return alloc(iden, reg);
}

string MipsGen::alloc(string iden, string reg) {
	string type = SorT(iden);
	if (type == "s") {
		if (free_s_reg.size()) {
			string reg = free_s_reg[0];
			free_s_reg.erase(free_s_reg.begin(), free_s_reg.begin() + 1);
			use_s_reg.push_back(reg);
			this->reg_table.push_back(RegTableItem(reg, 1, var_info(iden, -1, -1, -1, -1)));
			lw(reg, iden);
			return reg;
		}
		else {
			lw(reg, iden);
			return reg;
		}
	}
	if (type == "t") {
		if (free_t_reg.size()) {
			string reg = free_t_reg[0];
			free_t_reg.erase(free_t_reg.begin(), free_t_reg.begin() + 1);
			use_t_reg.push_back(reg);
			this->reg_table.push_back(RegTableItem(reg, 1, var_info(iden, -1, -1, -1, -1)));
			lw(reg, iden);
			return reg;
		}
		else {
			lw(reg, iden);
			return reg;
		}
	}
	return "";
}

vector<RegTableItem> MipsGen::clear_t() {
	vector<RegTableItem> clr = {};
	this->use_t_reg = {};
	this->free_t_reg = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6","$t7" };
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[1] == 't') {
			clr.push_back(this->reg_table[i]);
			this->reg_table.erase(reg_table.begin() + i, reg_table.begin() + i + 1);
			i--;
		}
	}
	return clr;
}

vector<RegTableItem> MipsGen::clear_s() {
	vector<RegTableItem> clr = {};
	this->use_s_reg = {};
	this->free_s_reg = { "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6","$s7" };
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[1] == 's') {
			clr.push_back(this->reg_table[i]);
			this->reg_table.erase(reg_table.begin() + i, reg_table.begin() + i + 1);
			i--;
		}
	}
	return clr;
}


int MipsGen::toi(string a) {
	if (isdigit(a[0]) || a[0] == '+' || a[0] == '-') {
		int c;
		stringstream ss;
		ss << a;
		ss >> c;
		return c;
	}
	else
		return int(a[1]);
}