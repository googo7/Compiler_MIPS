#include "MipsGen.h"
#include "FILEOperator.h"
#include "GMAnalysis.h"
#include <algorithm>
#include <sstream>
extern MemoryTable memory_table;
extern gm_analyse gm;
MidCode mc_now;
vector<string> func_name_stack = {};
string func_now = "";
int para_pt = 0;
int expr_temp_cnt = 0;
vector<string> inline_para;
extern gm_analyse gm;
int inline_flag = 0;
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
		/*else {
			sw(result_reg, result);
		}*/
	}
	else if (op == "mult" || op == "div") {
		if (is_digit(s1[0]) && is_digit(s2[0])) {
			string result_reg = lookup(result, "$t8");
			string res = "";
			if (op == "mult")
				res = to_string(toi(s1)* toi(s2));
			else
				res = to_string(toi(s1) / toi(s2));
			emit("li", result_reg, res, "");
			if (result_reg == "$t8")
				sw("$t8", result);
			return;
		}
		string s1_reg, s2_reg;
		if (is_digit(s1[0])) {
			s1_reg = "$t8";
			emit("li", "$t8", s1, "");
		}
		else {
			s1_reg = lookup(s1);
		}
		if (is_digit(s2[0])) {
			s2_reg = "$t9";
			emit("li", "$t9", s2, "");
		}
		else {
			s2_reg = lookup(s2, "$t9");
		}
		if (op == "mult") {
			emit("mult", s1_reg, s2_reg, "");
		}
		else if (op == "div") {
			emit("div", s1_reg, s2_reg, "");
		}
		string result_reg =lookup(result, "$t8");
		emit("mflo", result_reg, "", "");
		if(result_reg == "$t8")
			sw("$t8", result);
		/*else {
			sw(result_reg, result);
		}*/
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
				/*else {
					sw(result_reg, result);
				}*/
			}
			else {
				string result_reg = lookup(result);
				string s1_reg = lookup(s1, "$t9");
				emit("move", result_reg, s1_reg, "");
				if (result_reg == "$t8")
					sw(result_reg, result);
				/*else {
					sw(result_reg, result);
				}*/
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
				emit("li", "$t9", to_string(toi(s2) * 4), "");
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
			emit("li", "$t9", to_string(toi(s2) * 4), "");
		}
		else {
			string s2_reg = lookup(s2);
			emit("sll", "$t9", s2_reg, "2");
		}
		
		lw(result_reg, s1, "$t9");
		if (result_reg == "$t8")
			sw(result_reg, result);
		/*else {
			sw(result_reg, result);
		}*/
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
	else if (op == "BGE" || op == "BGT" || op == "BLE" || op == "BLT" || op == "BNE" || op == "BEQ") {
	string s1_reg = "", s2_reg = "";
	if (is_digit(s1[0])) {
		if (is_digit(s2[0])) {
			int res = 0;
			if (op == "BGE")
				res = (toi(s1) >= toi(s2));
			else if (op == "BGT")
				res = (toi(s1) > toi(s2));
			else if (op == "BLE")
				res = (toi(s1) <= toi(s2));
			else if (op == "BLT")
				res = (toi(s1) < toi(s2));
			else if (op == "BNE")
				res = (toi(s1) != toi(s2));
			else if (op == "BEQ")
				res = (toi(s1) == toi(s2));
			if (res)
				emit("j", result, "", "");

		}
		else {
			s2_reg = lookup(s2, "$t9");
			if (op == "BGE")  
				emit("ble",s2_reg, to_string(toi(s1)), result);
			else if (op == "BGT")
				emit("blt", s2_reg, to_string(toi(s1)), result);
			else if (op == "BLE")
				emit("bge", s2_reg, to_string(toi(s1)), result);
			else if (op == "BLT")
				emit("bgt", s2_reg, to_string(toi(s1)), result);
			else if (op == "BNE")
				emit("bne", s2_reg, to_string(toi(s1)), result);
			else if (op == "BEQ")
				emit("beq", s2_reg, to_string(toi(s1)), result);
		}
	}
	else {
		if (is_digit(s2[0])) {
			s1_reg = lookup(s1, "$t9");
			if (op == "BGE")   
				emit("bge", s1_reg, to_string(toi(s2)), result);
			else if (op == "BGT")
				emit("bgt", s1_reg, to_string(toi(s2)), result);
			else if (op == "BLE")
				emit("ble", s1_reg, to_string(toi(s2)), result);
			else if (op == "BLT")
				emit("blt", s1_reg, to_string(toi(s2)), result);
			else if (op == "BNE")
				emit("bne", s1_reg, to_string(toi(s2)), result);
			else if (op == "BEQ")
				emit("beq", s1_reg, to_string(toi(s2)), result);
		}
		else {
			s1_reg = lookup(s1);
			s2_reg = lookup(s2, "$t9");
			if (op == "BGE")   // num < iden
				emit("bge", s1_reg, s2_reg, result);
			else if (op == "BGT")
				emit("bgt", s1_reg, s2_reg, result);
			else if (op == "BLE")
				emit("ble", s1_reg, s2_reg, result);
			else if (op == "BLT")
				emit("blt", s1_reg, s2_reg, result);
			else if (op == "BNE")
				emit("bne", s1_reg, s2_reg, result);
			else if (op == "BEQ")
				emit("beq", s1_reg, s2_reg, result);
		}
	}

	/*else {
		sw(result_reg, result);
	}*/

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
		/*else {
			sw(result_reg, result);
		}*/

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
			if (isdigit(s2[0]) || s2[0] == '+' || s2[0] == '-') {
				if (mc.res_type == INT) {
					emit("li", "$a0", to_string(toi(s2)), "");
					emit("li", "$v0", "1", "");
					emit("syscall", "", "", "");
				}
				else if (mc.res_type == CHAR) {
					emit("li", "$a0", to_string(toi(s2)), "");
					emit("li", "$v0", "11", "");
					emit("syscall", "", "", "");
				}
			}
			/*if (isdigit(s2[0]) || s2[0] == '+' || s2[0] == '-' ) {
				emit("li", "$a0", to_string(toi(s2)), "");
				emit("li", "$v0", "1", "");
				emit("syscall", "", "", "");
			}
			else if (s2[0] == '\'') {
				emit("li", "$a0", to_string((int)s2[1]), "");
				emit("li", "$v0", "11", "");
				emit("syscall", "", "", "");
			}*/
			else {
				string s2_reg = lookup(s2);
				emit("move", "$a0", s2_reg, "");
				//int type = -1;
				//type = memory_table.lookup(func_now, s2)._type;
				//if (type == -1)
				//	type = memory_table.lookup("", s2)._type;
				//if (type == INT || type == TEMP)//数组查找类型
				//	emit("li", "$v0", "1", "");
				//else if (type == CHAR)
				//	emit("li", "$v0", "11", "");
				if (mc.res_type == INT)
					emit("li", "$v0", "1", "");
				else
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
		emit("sw", "$ra", "4", "$sp");

		func_now = s2;
	}
	else if (op == "para") {
		int num = (int)gm.symtab.func_lookup(func_now).v_table.size();
		string s2_reg = lookup(s2);
		emit("lw", s2_reg, to_string(8 + 4 * num - toi(s1)), "$sp");
		
		sw(s2_reg, s2);
	}
	else if (op == "func_push_para") {
	para_pt += 1;
	string s1_reg = "";
	if (is_digit(s1[0])) {
		s1_reg = "$t8";
		emit("li", "$t8", s1, "");
		}
	else {
		s1_reg = lookup(s1);
	}
		emit("sw", s1_reg, to_string(-((memory_table.top_addr(func_now) + para_pt - 1) << 2)), "$sp");
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
		emit("sw", "$sp", to_string(-((memory_table.top_addr(func_now) + para_pt) << 2)), "$sp");
		emit("addiu", "$sp", "$sp", to_string(-((memory_table.top_addr(func_now) + para_pt + 2) << 2)));
		emit("jal", s1, "", "");
		para_pt -= (int)func_lookup(s1).v_table.size();
		for (int i = 0; i < clr_s.size(); i++) {
			lw(clr_s[i].reg, clr_s[i].var.iden);
		}
		for (int i = 0; i < clr_t.size(); i++) {
			lw(clr_t[i].reg, clr_t[i].var.iden);
		}
		if (!result.size())
			return;
		string result_reg = lookup(result);
		emit("move", result_reg, "$v0", "");
		if (result_reg == "$t8") {
			sw(result_reg, result);
		}
		/*else {
			sw(result_reg, result);
		}*/
		
	}
	else if (op == "end_func") {
		clear_reg();
		emit("lw", "$ra", "4", "$sp");
		emit("lw", "$sp", "8", "$sp");
		emit("jr", "$ra", "", "");
	}
	else if (op == "ret") {
		if (!s1.size()) {
			if (func_now == "main") {
				emit("li", "$v0", "10", "");
				emit("syscall", "", "", "");
				return;
			}
			//clear_reg();
			emit("lw", "$ra", "4", "$sp");
			emit("lw", "$sp", "8", "$sp");
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
		//clear_reg();
		emit("lw", "$ra", "4", "$sp");
		emit("lw", "$sp", "8", "$sp");
		emit("jr", "$ra", "", "");
	}
	else if (op == "exit") {
		emit("li", "$v0", "10", "");
		emit("syscall", "", "", "");
	}
	else if (op == "clear_temp") {
		string remain = "";
		
		for (int i = 0; i < this->reg_table.size(); i++) {
			if (find(this->t_temp.begin(), this->t_temp.end(), this->reg_table[i].reg) != t_temp.end() && this->reg_table[i].var.iden != s1) {
				this->reg_table.erase(reg_table.begin() + i);
				i--;
			}
			else if (find(this->t_temp.begin(), this->t_temp.end(), this->reg_table[i].reg) != t_temp.end() && this->reg_table[i].var.iden == s1) {
				remain = this->reg_table[i].reg;
			}
		}
		for (int i = 0; i < this->use_t_reg.size(); i++) {
			if (find(this->t_temp.begin(), this->t_temp.end(), this->use_t_reg[i]) != t_temp.end()) {
				this->use_t_reg.erase(use_t_reg.begin() + i);
				i--;
			}
		}
		this->free_t_reg = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7" };
		if (remain!= "")
			this->use_t_reg.push_back(remain);
		for (int i = 0; i < free_t_reg.size(); i++) {
			if (find(this->use_t_reg.begin(), this->use_t_reg.end(), free_t_reg[i]) != this->use_t_reg.end()) {
				this->free_t_reg.erase(free_t_reg.begin() + i);
				i--;
			}
		}
		expr_temp_cnt--;
		t_temp = this->t_temp_stack[t_temp_stack.size() - 1];
		t_temp_stack.pop_back();
	}
	else if (op == "save_temp") {
		expr_temp_cnt++;
		this->t_temp_stack.push_back(t_temp);
		t_temp = {};
	}

	else if (op == "begin_inline_func") {
		inline_flag = 1;
		inline_para = {};
	}
	else if (op == "end_inline_func") {
	inline_flag = 0;
	for (int i = 0; i < inline_para.size(); i++) {
		
		for (int j = 0; j < reg_table.size(); j++) {
			if (reg_table[j].var.iden == inline_para[i] && inline_para[i] != s1) {
				if (reg_table[j].reg[1] == 't') {
					free_t_reg.push_back(reg_table[j].reg);
					for (int k = 0; k < use_t_reg.size(); k++) {
						if (use_t_reg[k] == reg_table[j].reg) {
							use_t_reg.erase(use_t_reg.begin() + k);
							k--;
						}
					}
				}
				else if (reg_table[j].reg[1] == 's') {
					free_s_reg.push_back(reg_table[j].reg);
					for (int k = 0; k < use_s_reg.size(); k++) {
						if (use_s_reg[k] == reg_table[j].reg) {
							use_s_reg.erase(use_s_reg.begin() + k);
							k--;
						}
					}
				}
				reg_table.erase(reg_table.begin() + j);
				j--;
				
			}
		}
		if(inline_para[i] != s1)
			memory_table.setflag(func_now, inline_para[i], 0);
		
	}
	for (int j = 0; j < reg_table.size(); j++) {
		if (reg_table[j].var.iden.size() > 5 && reg_table[j].var.iden[4] == 'i' && reg_table[j].var.iden[5] == 'n') {
			memory_table.setflag(func_now, reg_table[j].var.iden, 0);
			string ss = reg_table[j].reg;
			for (int k = 0; k < use_s_reg.size(); k++) {
				if (use_s_reg[k] == ss) {
					use_s_reg.erase(use_s_reg.begin() + k);
					k--;
				}
			}
			free_s_reg.push_back(ss);
			reg_table.erase(reg_table.begin() + j);
			j--;
		}
	}
	inline_para = {};
	}
	else if (op == "clear_for_block") {
	for (int i = 0; i < reg_table.size(); i++) {
		string reg = reg_table[i].reg;
		string iden = reg_table[i].var.iden;
		if (reg[1] == 's') {
			int res = gm.mc_gen.check_use(iden, s1);
			if (!res) {
				for (int k = 0; k < use_s_reg.size(); k++) {
					if (use_s_reg[k] == reg) {
						use_s_reg.erase(use_s_reg.begin() + k);
						k--;
						}
					}
					free_s_reg.push_back(reg);
					reg_table.erase(reg_table.begin() + i);
					i--;
			}
		}
	}
}
	
}

void MipsGen::emit(string op, string s1, string s2, string s3) {
	gm.mc_gen.mips_code.push_back(vector<string>({ op, s1, s2, s3 }));
}

void MipsGen::emit(vector<string> vs) {
	string op = vs[0];
	string s1 = vs[1];
	string s2 = vs[2];
	string s3 = vs[3];
	static int text_flag = 1;
	if (text_flag && op == "LABEL") {
		write_into_mfile(".text:");
		write_into_mfile("j main");
		text_flag = 0;
	}
	string res;
	res = "\t" + op;
	if (s1 == "$s8")
		s1 = "$a1";
	else if (s1 == "$s9")
		s1 = "$a2";
	else if (s1 == "$s10")
		s1 = "$a3";
	else if (s1 == "$s11")
		s1 = "$fp";
	if (s2 == "$s8")
		s2 = "$a1";
	else if (s2 == "$s9")
		s2 = "$a2";
	else if (s2 == "$s10")
		s2 = "$a3";
	else if (s2 == "$s11")
		s2 = "$fp";
	if (s3 == "$s8")
		s3 = "$a1";
	else if (s3 == "$s9")
		s3 = "$a2";
	else if (s3 == "$s10")
		s3 = "$a3";
	else if (s3 == "$s11")
		s3 = "$fp";

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
	else if (op == "jal" || op == "jr") {
		res += "\t" + s1;
	}
	else if (op == "bge" || op == "bgt" || op == "ble" || op == "blt" || op == "bne" || op == "beq") {
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
	memory_table.setflag(func_now, id);
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("sw", s, to_string(addr), "$gp");
		return;
	}
	emit("sw", s, to_string(-addr), "$sp");
}

void MipsGen::sw(string s, string id, string reg) {
	memory_table.setflag(func_now, id);
	int addr = memory_table.lookup_addr(func_now, id);
	if (addr < 0) {
		addr = memory_table.lookup_addr("", id);
		emit("addu", reg, "$gp", reg);
		emit("sw", s, to_string(addr), reg);
		return;
	}
	emit("subu", reg, "$sp", reg);
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
		emit("addu", reg, "$gp", reg);
		emit("lw", s, to_string(addr), reg);
		return;
	}
	emit("subu", reg, "$sp", reg);
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
			if (this->reg_table[i].var.iden == iden) {
				string r = this->reg_table[i].reg;
				if (this->reg_table[i].var.iden[0] == 'x' && this->reg_table[i].var.iden[1] == 'x' && this->reg_table[i].var.iden[2] == 'j' && this->reg_table[i].var.iden[3] == '_' && this->reg_table[i].var.iden[4] == 't')
				{
					memory_table.setflag(func_now, iden, 0);
					for (int j = 0; j < use_t_reg.size(); j++) {
						if (use_t_reg[j] == reg_table[i].reg){
							use_t_reg.erase(use_t_reg.begin() + j);
							j--;
							}
					}
					free_t_reg.push_back(reg_table[i].reg);
					this->reg_table.erase(this->reg_table.begin() + i);
					i--;
				}
				else {
					for (int j = 0; j < use_s_reg.size(); j++) {
						if (this->reg_table[i].reg == use_s_reg[j]) {
							this->use_s_reg.erase(use_s_reg.begin() + j);
							j--;
						}
					}
					this->use_s_reg.push_back(r);
				}
				return r;
			}
		}
	}
	return alloc(v, reg);
}

string MipsGen::alloc(var_info v, string reg) {
	string type = SorT(v.iden);
	if (type == "s") {
		if (inline_flag)
			inline_para.push_back(v.iden);
		if (free_s_reg.size()) {
			string reg = free_s_reg[0];
			free_s_reg.erase(free_s_reg.begin());
			use_s_reg.push_back(reg);
			if (v.value != 12306) {
				this->reg_table.push_back(RegTableItem(reg, 1, var_info(v.iden, 0, 1, 0, -10)));
				memory_table.setflag(func_now, v.iden);
			}
			else {
				this->reg_table.push_back(RegTableItem(reg, 1, var_info(v.iden, 0, 1, 0, -10)));
				lw(reg, v.iden);
			}
			return reg;
		}
		else {
			/*string reg_replace = use_s_reg[0], iden_replace;
			use_s_reg.erase(use_s_reg.begin());
			use_s_reg.push_back(reg_replace);
			for (int i = 0; i < reg_table.size(); i++) {
				if (reg_table[i].reg == reg_replace) {
					iden_replace = reg_table[i].var.iden;
					reg_table.erase(reg_table.begin() + i);
					i--;
				}
			}
			sw(reg_replace, iden_replace);
			if (v.value != 12306) {
				this->reg_table.push_back(RegTableItem(reg_replace, 1, var_info(v.iden, 0, 1, 0, -10)));
				memory_table.setflag(func_now, v.iden);
			}
			else {
				this->reg_table.push_back(RegTableItem(reg_replace, 1, var_info(v.iden, 0, 1, 0, -10)));
				lw(reg_replace, v.iden);
			}
			return reg_replace;*/
			if (v.value != 12306) {
				memory_table.setflag(func_now, v.iden);
			}
			else {
				lw(reg, v.iden);
			}
			return reg;
		}
	}
	if (type == "t") {
		if (inline_flag)
			inline_para.push_back(v.iden);
		if (free_t_reg.size()) {
			string reg = free_t_reg[0];
			free_t_reg.erase(free_t_reg.begin());
			use_t_reg.push_back(reg);
			if(expr_temp_cnt)
				t_temp.push_back(reg);
			if (v.value != 12306) {
				this->reg_table.push_back(RegTableItem(reg, 1, var_info(v.iden, 0, 1, 0, -10)));
				memory_table.setflag(func_now, v.iden);
			}
			else {
				this->reg_table.push_back(RegTableItem(reg, 1, var_info(v.iden, 0, 1, 0, -10)));
				lw(reg, v.iden);
			}
			//lw(reg, iden);
			return reg;
		}
		else {
			if (v.value != 12306) {
				memory_table.setflag(func_now, v.iden);
			}
			else {
				lw(reg, v.iden);
			}
			return reg;
		}
	}
	return "";
}

vector<RegTableItem> MipsGen::clear_t() {
	vector<RegTableItem> clr = {};
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[1] == 't') {
			clr.push_back(this->reg_table[i]);
		}
	}
	return clr;
}

vector<RegTableItem> MipsGen::clear_s() {
	vector<RegTableItem> clr = {};

	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[1] == 's') {
			clr.push_back(this->reg_table[i]);
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

void MipsGen::push_reg_table() {
	reg_table_stack.push_back(reg_table);
}

void MipsGen::pop_reg_table() {
	this->reg_table = reg_table_stack[reg_table_stack.size() - 1];
	reg_table_stack.pop_back();
}

void MipsGen::clear_reg() {
	this->reg_table = {};
	this->use_s_reg = {};
	this->free_s_reg = { "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6","$s7", "$s8", "$s9", "$s10", "$s11" };
	this->use_t_reg = {};
	this->free_t_reg = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6","$t7" };
}

func_info MipsGen::func_lookup(string iden) {
	func_info a(string(""), vector<int>{}, -1);
	for (int i = 0; i < this->func_table.size(); i++) {
		func_info fi = this->func_table[i];
		if (fi.iden == iden)
			return fi;
	}
	return a;
}
