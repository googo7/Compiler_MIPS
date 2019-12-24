#include "MidCode.h"
#include "FILEOperator.h"
#include "MipsGen.h"
#include "MemoryManage.h"
#include <algorithm>
#include <unordered_map>

extern MipsGen mips_gen;
extern int label_cnt;
extern MemoryTable memory_table;
extern string func_name_now;
extern int array_type;
extern int func_return_type;
MidCode::MidCode() { ; }
extern ofstream mipsfile;
MidCode::MidCode(string o, string s, string ss, string r, int i) {
	this->op = o;
	this->s1 = s;
	this->s2 = ss;
	this->result = r;
	this->res_type = i;
}

string MidCode::out() {
	string s = this->op;
	s += "\t\t|";
	s += this->s1;
	if ((s1[0] == 'x' && s1[1] == 'x' && s1[2] == 'j') || (s1[0] == 'l' && s1[1] == 'a' && s1[2] == 'b' && s1[3] == 'e' && s1[4] == 'l'))
		s += "\t|";
	else
		s += "\t\t|";
	s += this->s2;
	if ((s2[0] == 'x' && s2[1] == 'x' && s2[2] == 'j') || (s2[0] == 'l' && s2[1] == 'a' && s2[2] == 'b' && s2[3] == 'e' && s2[4] == 'l'))
		s += "\t|";
	else
		s += "\t\t|";
	s += this->result;
	return s;
}


MidCodeGen::MidCodeGen() {
	this->mc = {};
}



void MidCodeGen::push(string op, string s1, string s2, string result, int i) {
	this->mc.push_back(MidCode(op, s1, s2, result, i));

}

int MidCodeGen::get_last_type(void) {
	if (mc.size())
		for (int i = mc.size() - 1; i >= 0; i--) {
			if (this->mc[i].op != "clear_temp" && this->mc[i].op != "save_temp")
				return this->mc[i].res_type;
		}
	else
		return 1;
	return 1;
}
string MidCodeGen::get_last_result(int flag) {
	/*if (flag) {
		if (mc.size() - 1)
			return this->mc[mc.size() - 2].result;
		else
			return "";
	}*/
	if (mc.size())
		for (int i = mc.size() - 1; i >= 0 ; i--) {
			if(this->mc[i].op != "clear_temp" && this->mc[i].op != "save_temp")
				return this->mc[i].result;
		}
	else
		return "";
	return "";
}

string MidCodeGen::gen_temp(int type) {
	static int cnt = 0;
	string s = string("xxj_temp") + to_string(cnt++);
	memory_table.push(s, type);
	return s;
}

void MidCodeGen::parse(string type, vector<token_info> tk_set, int cnt) {
	if (type == "FUNC") {
		
		//int\char func_name(int a, int b, int c)
		string s1 = tk_set[0].token;
		string s2 = tk_set[1].token;
		push(string("func"), s1, s2, string(""));
		if (tk_set.size() == 4)
			return;
		for (int i = 3; i < tk_set.size(); i += 3) {
			s1 = tk_set[i].token;
			s2 = tk_set[i + (long long)1].token;
			push(string("para"), to_string((i / 3 - 1) << 2), s2, string(""));
		}
	}
	else if (type == "FUNCCALL") {
		//func_name(expr1, expr2, expr3)
		string s1;
		string s2;
		vector<token_info> vt;
		int cnt_para = 0;
		int cnt_parent = 1;
		for (int i = 2; i < tk_set.size(); i++) {
			if (tk_set[i].type == COMMA && cnt_parent == 1) {
				parse("EXPR", vt);
				string res = get_last_result();
				push(string("func_push_para"), res, to_string(cnt_para++), string(""));
				vt.clear();
			}
			else if (i != tk_set.size() - 1) {
				vt.push_back(tk_set[i]);
				if (tk_set[i].type == LPARENT)
					cnt_parent++;
				else if (tk_set[i].type == RPARENT)
					cnt_parent--;
			}
		}
		parse("EXPR", vt);
		string res = get_last_result();
		push(string("func_push_para"), res, to_string(cnt_para++), string(""));
		vt.clear();
		s1 = tk_set[0].token;
		
		push(string("func_call"), s1, string(""), gen_temp(func_return_type));
	}
	
	else if (type == "IFHEAD") {
		//if(condition) statement1 else <label_begin> statement2 <label_end>
		//BZ condition label_begin
		//stat1
		//GOTO label_end
		//<label_begin> stat2 
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size() - 1; i++) {
			vt.push_back(tk_set[i]);
		}
		parse("CONDITION", vt);
		string s1 = get_last_result();
		string s2 = "label_if_head" + to_string(cnt);
		push(string("BZ"), s1, s2, string(""));
	}
	else if (type == "CONDITION") {
		//exp1 <= exp2
		string s1 = tk_set[0].token;
		string s2 = "";
		string op = "=";
		if (tk_set.size() > 1) {
			s2 = tk_set[2].token;
			op = tk_set[1].token;
		}
		push(op, s1, s2, gen_temp());
	}
	else if (type == "WHILE") {
		//whlie(con) stat1 
		
		//	label_while_begin:
		//	BZ con <label_while_end>  
		//	stat1
		//	GOTO <label_while_begin>
		//	label_while_end:
		
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size() - 1; i++) {
			vt.push_back(tk_set[i]);
		}
		parse(string("CONDITION"), vt);
		string s1 = get_last_result();
		string s2 = string("label_while_end") + to_string(cnt);
		push(string("BZ"), s1, s2, string(""));
		
	
	}
	else if (type == "DOWHILE") {
		//do stat1 while(con)

		/*
			label_dowhile:
			stat1
			BNZ con <label_dowhile>
		*/
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size() - 1; i++) {
			vt.push_back(tk_set[i]);
		}
		parse(string("CONDITION"), vt);
		string s1 = get_last_result();
		string s2 = string("label_dowhile") + to_string(cnt);
		push(string("BNZ"), s1, s2, string(""));
	}
	else if (type == "FOR") {
		//for(iden = expr; con; iden = iden + step) stat1
		/*
			iden = expr;
			label_for_begin:
			BZ con <label_for_end>
			stat1
			iden = iden + step;
			GOTO <label_for_begin>
			label_for_end:



			iden = expr;
			label_for_begin:
			BZ con <label_for_end>
			stat1
			iden = iden + step;
			BNZ con <label_for_begin>
			label_for_end:
		*/
		vector<token_info> con;
		int i;

		for (i = 6; i < tk_set.size(); i++) {
			if (tk_set[i].type == SEMICN) {
				break;
			}
			con.push_back(tk_set[i]);
		}
		parse("CONDITION", con);
		push(string("BZ"), get_last_result(), "label_for_end" + to_string(cnt), string(""));

	}
	else if (type == "FOR_TAIL") {
	//for(iden = expr; con; iden = iden + step) stat1
	/*
		iden = expr;
		label_for_begin:
		BZ con <label_for_end>
		stat1
		iden = iden + step;
		GOTO <label_for_begin>
		label_for_end:



		iden = expr;
		BZ con <label_for_end>
		label_for_begin:
		
		stat1
		iden = iden + step;
		BNZ con <label_for_begin>
		label_for_end:
	*/
	vector<token_info> con;
	int i;

	for (i = 6; i < tk_set.size(); i++) {
		if (tk_set[i].type == SEMICN) {
			break;
		}
		con.push_back(tk_set[i]);
	}

	push(string("BNZ"), get_last_result(), "label_for_begin" + to_string(cnt), string(""));


	}
	//else if (type == "ARRAY") {
	//	//iden [expr]
	//	//[] , iden, expr, t0
	//	string s1 = tk_set[0].token;
	//	
	//	vector<token_info> vt;
	//	for (int i = 2; i < tk_set.size() - 1; i++) {
	//		vt.push_back(tk_set[i]);
	//	}
	//	parse("EXPR", vt);
	//	string s2 = get_last_result();
	//	push("[]", s1, s2, gen_temp(array_type));
	//}
	else if (type == "ENDFUNC") {
		push("end_func", "", "", "");
	}
	else if (type == "EXIT") {
		push("exit", "", "", "");
	}

}

void MidCodeGen::out() {
	mips_gen.predeal(this->mc);
	op_dead();
	op_inline();
	op_compare();
	op_block();
	
	for (int i = 0; i < this->mc.size(); i++) {
		if(mc[i].op != "save_temp" && mc[i].op != "clear_temp" && mc[i].op != "begin_inline_func" && mc[i].op != "end_inline_func" && mc[i].op != "clear_for_block")
			write_into_file(mc[i]);
		mips_gen.parse(mc[i]);
	}

	op_kui();
	for (int i = 0; i < mips_code.size(); i++) {
		mips_gen.emit(mips_code[i]);
	}
}

void MidCodeGen::op_inline() {
	string func_pt = "";
	vector<string> para_pt = {};
	string ret = "";
	vector<MidCode> vM = {};
	for (int i = 0; i < this->mc.size(); i++) {
		func_pt = ""; para_pt = {}; ret = ""; vM = {};
		if (mc[i].op == "func" && mc[i].s2 != "main") {
			func_name_now = mc[i].s2;
			vM.push_back(mc[i]);
			func_pt = mc[i].s2;
			while (mc[i].op != "end_func") {
				
				i++;
				vM.push_back(mc[i]);
				if ((mc[i].s1 != "" && !isdigit(mc[i].s1[0]) && (memory_table.lookup(func_name_now, mc[i].s1).addr == -1))
					|| (mc[i].s2 != "" && !isdigit(mc[i].s2[0]) && memory_table.lookup(func_name_now, mc[i].s2).addr == -1)
					|| (mc[i].result != "" && !isdigit(mc[i].result[0]) && memory_table.lookup(func_name_now, mc[i].result).addr == -1))
					break;
				if (mc[i].op == "para") {
					para_pt.push_back(mc[i].s2);
				}
				else if (mc[i].op == "ret" && ret == "") {
					ret = mc[i].s1;
				}
				else if (mc[i].op == "BZ" || mc[i].op == "GOTO" || mc[i].op == "func_call" || mc[i].op == "BNZ") {
					break;
				}
				else if (mc[i].op == "end_func") {
					int cnt = 0;
					vector<var_info> vv = memory_table.map[func_pt];
					for (int i = 0; i < vv.size(); i++) {
						if (!(vv[i].iden[0] == 'x' && vv[i].iden[1] == 'x' && vv[i].iden[2] == 'j'))
							cnt++;
					}
					if (cnt == para_pt.size()) {
						inline_func_info.push_back(inline_func(func_pt, para_pt, ret, vM));
					}
				}
			}
		}
	}

	for (int i = 0; i < this->inline_func_info.size(); i++) {
		vector<MidCode> vm_temp = {};
		vector<MidCode> vm = inline_func_info[i].func_mc;
		vector<string> para_set = inline_func_info[i].para;
		unordered_map<string, string> temp_map = unordered_map<string, string>({});
		if(inline_func_info[i].ret != "" && !(isdigit(inline_func_info[i].ret[0]) || inline_func_info[i].ret[0] == '+' || inline_func_info[i].ret[0] == '-'))
			para_set.push_back(inline_func_info[i].ret);
		for (int j = 0; j < vm.size(); j++) {
			if (vm[j].op == "func" || vm[j].op == "ret" || vm[j].op == "para" || vm[j].op == "end_func") {
				vm.erase(vm.begin() + j);
				inline_func_info[i].func_mc.erase(inline_func_info[i].func_mc.begin() + j);
				j--;
				continue;
			}
			if (find(para_set.begin(), para_set.end(), vm[j].s1) != para_set.end()) {
				inline_func_info[i].func_mc[j].s1 = "xxj_inline_" + vm[j].s1;
			}
			if (find(para_set.begin(), para_set.end(), vm[j].s2) != para_set.end()) {
				inline_func_info[i].func_mc[j].s2 = "xxj_inline_" + vm[j].s2;
			}
			if (find(para_set.begin(), para_set.end(), vm[j].result) != para_set.end()) {
				inline_func_info[i].func_mc[j].result = "xxj_inline_" + vm[j].result;
			}
			if (vm[j].s1[0] == 'x' && vm[j].s1[1] == 'x' && vm[j].s1[2] == 'j' && vm[j].s1[3] == '_' && vm[j].s1[4] == 't') {
				inline_func_info[i].func_mc[j].s1 = temp_map[vm[j].s1];
			}
			if (vm[j].s2[0] == 'x' && vm[j].s2[1] == 'x' && vm[j].s2[2] == 'j' && vm[j].s2[3] == '_' && vm[j].s2[4] == 't') {
				inline_func_info[i].func_mc[j].s2 = temp_map[vm[j].s2];
			}
			if (vm[j].result[0] == 'x' && vm[j].result[1] == 'x' && vm[j].result[2] == 'j' && vm[j].result[3] == '_' && vm[j].result[4] == 't') {
				int var_type = memory_table.lookup(inline_func_info[i].func_name, vm[j].result)._type;
				
				string s11 = vm[j].result;
				string s22 = gen_temp(var_type);
				inline_func_info[i].func_mc[j].result = s22;
				temp_map[s11] = s22;
			}
		}
		if (inline_func_info[i].ret[0] == 'x' && inline_func_info[i].ret[1] == 'x' && inline_func_info[i].ret[2] == 'j' && inline_func_info[i].ret[3] == '_' && inline_func_info[i].ret[4] == 't' && inline_func_info[i].ret[5] == 'e')
		{
			inline_func_info[i].ret = temp_map[inline_func_info[i].ret];
		}
	}

	inline_func in_func = inline_func("", {}, "", {});
	for (int i = 0; i < this->mc.size(); i++) {
		if (mc[i].op == "func")
			func_name_now = mc[i].s2;
		in_func = inline_func("", {}, "", {});
		if (mc[i].op == "func_call") {
			for (int j = 0; j < inline_func_info.size(); j++) {
				if (inline_func_info[j].func_name == mc[i].s1) {
					in_func = inline_func_info[j];
					break;
				}
			}
			if (in_func.func_name == "")
				continue;
			else {
				for (int j = 0; j < in_func.para.size(); j++) {
					memory_table.push(func_name_now, var_info("xxj_inline_" + in_func.para[j], 0, 1, 0, memory_table.top_addr(func_name_now)));
				}
			}
			int j = i;
			int cnt = 0;
			while (cnt != in_func.para.size()) {
				if (mc[j].op == "func_push_para") {
					mc[j] = MidCode("=", mc[j].s1, "", "xxj_inline_" + in_func.para[in_func.para.size() - 1 - cnt]);
					cnt++;
				}
				j--;
			}
			mc.insert(mc.begin() + i + 1, MidCode("end_inline_func", mc[i].result, "", ""));
			if (mc[i].result != "") {
				if (in_func.ret != "" && in_func.ret[0] == 'x' && in_func.ret[1] == 'x' && in_func.ret[2] == 'j' && in_func.ret[3] == '_' && in_func.ret[4] == 't')
					mc.insert(mc.begin() + i + 1, MidCode("=", in_func.ret, "", mc[i].result, mc[i].res_type));
				else if (in_func.ret != "" && !(isdigit(in_func.ret[0]) || in_func.ret[0] == '+' || in_func.ret[0] == '-'))
					mc.insert(mc.begin() + i + 1, MidCode("=", "xxj_inline_" + in_func.ret, "", mc[i].result, mc[i].res_type));
				else if (in_func.ret != "")
					mc.insert(mc.begin() + i + 1, MidCode("=", in_func.ret, "", mc[i].result, mc[i].res_type));
			}
			for (int j = in_func.func_mc.size() - 1; j >= 0; j--) {
				if (in_func.func_mc[j].result[0] == 'x' && in_func.func_mc[j].result[1] == 'x' && in_func.func_mc[j].result[2] == 'j' && in_func.func_mc[j].result[3] == '_' && in_func.func_mc[j].result[4] == 't') {
					string s11 = in_func.func_mc[j].result;
					memory_table.push(func_name_now, var_info(s11, 0, 1, 0, memory_table.top_addr(func_name_now)));
				}
				mc.insert(mc.begin() + i + 1, in_func.func_mc[j]);
			}
			mc[i] = MidCode("begin_inline_func", "", "", "");
			//mc.erase(mc.begin() + i);
			//mc.insert(mc.begin() + j + 1, MidCode("begin_inline_func", "", "", ""));
			
		}
	}

}

void MidCodeGen::op_compare() {
	string s1, s2;
	for (unsigned int i = 0; i < mc.size(); i++) {
		if (mc[i].op == "BZ") {
			s1 = mc[i - 1].s1;
			s2 = mc[i - 1].s2;
			if (mc[i - 1].op == "<") {
				mc[i] = MidCode("BGE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "<=") {
				mc[i] = MidCode("BGT", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == ">") {
				mc[i] = MidCode("BLE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == ">=") {
				mc[i] = MidCode("BLT", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "==") {
				mc[i] = MidCode("BNE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "!=") {
				mc[i] = MidCode("BEQ", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else {
				continue;
			}
			mc.erase(mc.begin() + i - 1);
			i--;
		}
		else if (mc[i].op == "BNZ") {
			s1 = mc[i - 1].s1;
			s2 = mc[i - 1].s2;
			if (mc[i - 1].op == "<") {
				mc[i] = MidCode("BLT", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "<=") {
				mc[i] = MidCode("BLE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == ">") {
				mc[i] = MidCode("BGT", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == ">=") {
				mc[i] = MidCode("BGE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "==") {
				mc[i] = MidCode("BEQ", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else if (mc[i - 1].op == "!=") {
				mc[i] = MidCode("BNE", s1, s2, mc[i].s2, mc[i].res_type);
			}
			else {
				continue;
			}
			mc.erase(mc.begin() + i - 1);
			i--;
		}
	}

}

void MidCodeGen::op_block() {
	for (int i = 0; i < mc.size(); i++) {
		if (mc[i].op == "func")
			func_name_now = mc[i].s2;
		if (mc[i].op == "LABEL" && mc[i].s1 == "label_for_begin") {
			string num = mc[i].s2;
			while (!(mc[i].op == "LABEL" && mc[i].s1 == "label_for_end" && mc[i].s2 == num)) {
				i++;
			}
			mc.insert(mc.begin() + i + 1, MidCode("clear_for_block", mc[i].s1+mc[i].s2, "", ""));
		}
	}
}


void MidCodeGen::op_dead() {
	vector<string> vs;
	int cnt = 0;
	int cnt_temp = mc.size();
	while (cnt != cnt_temp) {
		vs = {};
		for (int i = 0; i < mc.size(); i++) {
			if (mc[i].result != "" && !isdigit(mc[i].result[0]) && (mc[i].result[0]!='-') && !(mc[i].result[0] == 'l' && mc[i].result[1] == 'a' && mc[i].result[5] == '_')) {
				int res = check_use(mc[i].result, i);
				if (!res && mc[i].op != "func_call") {
					int flag = 0;
					for (int j = 0; j < vs.size(); j++) {
						if (vs[j] == mc[i].result) {
							flag = 1;
							break;
						}
					}
					if (flag)
						continue;
					mc.erase(mc.begin() + i);
					i--;
				}
				else if(mc[i].op != "func_call") {
					vs.push_back(mc[i].result);
				}
				else if(!res){
					mc[i].result = "";
				}
			}
			if (mc[i].op == "scanf") {
				vs.push_back(mc[i].s1);
			}
			vs.push_back(mc[i].s1);
			vs.push_back(mc[i].s2);
		}
		cnt = cnt_temp;
		cnt_temp = mc.size();
	}
}

void MidCodeGen::op_kui() {
	for (int i = 0; i < mips_code.size(); i++) {
		if (mips_code[i][0] == "move" && mips_code[i][2][1] == 't') {
			//if (mips_code[i - 1][0] == "addu" || mips_code[i - 1][0] == "addiu" || mips_code[i - 1][0] == "subu" || mips_code[i - 1][0] == "subiu" || mips_code[i - 1][0] == "mflo" || mips_code[i - 1][0] == "move" || mips_code[i - 1][0] == "li") {
				if (mips_code[i - 1][1] == mips_code[i][2]) {
					mips_code[i - 1][1] = mips_code[i][1];
					mips_code.erase(mips_code.begin() + i);
					i--;
				}
			//}
		}
		//if (mips_code[i][0] == "syscall" && mips_code[i - 1][0] == "li") {
		//	if (mips_code[i - 3][0] == "syscall" && mips_code[i - 4][0] == "li") {
		//		if (mips_code[i - 1][2] == mips_code[i - 4][2]) {
		//			mips_code.erase(mips_code.begin() + i - 1);
		//			i--;
		//		}
		//	}
		//}
		if (mips_code[i][0] == "bne" || mips_code[i][0] == "beq" || mips_code[i][0] == "bgt" || mips_code[i][0] == "bge" || mips_code[i][0] == "blt" || mips_code[i][0] == "ble") {
			if (mips_code[i][2] == "0")
				mips_code[i][2] = "$0";
		}
		if (mips_code[i][0] == "j" && mips_code[i + 1][0] == "LABEL" && mips_code[i + 2][0] == "LABEL") {
			if (mips_code[i][1] == mips_code[i + 2][1] + mips_code[i + 2][2]) {
				mips_code.erase(mips_code.begin() + i);
			}
		}
	}
}

int MidCodeGen::check_use(string iden, string begin) {
	for (int i = 0; i < mc.size(); i++) {
		if (mc[i].op == "LABEL" && mc[i].s1 + mc[i].s2 == begin) {
			for (int j = i; j < mc.size(); j++) {
				if (mc[j].s1 == iden || mc[j].s2 == iden || mc[j].result == iden) {
					return 1;
				}
				else if (mc[j].op == "end_func") {
					break;
				}
			}
		}
	}

	return 0;
}

int MidCodeGen::check_use(string iden, int i) {
	for (i++; i < mc.size(); i++) {
		if ((mc[i].s1 == iden || mc[i].s2 == iden || mc[i].result == iden) && mc[i].op != "clear_temp"&& mc[i].op != "end_inline_func") {
			return 1;
		}
		else if (mc[i].op == "exit") {
			break;
		}
	}
	return 0;
}

inline_func::inline_func() { ; }

inline_func::inline_func(string n, vector<string> p, string r, vector<MidCode> vM) {
	this->func_name = n;
	this->para = p;
	this->ret = r;
	this->func_mc = vM;
}

//else if (type == "EXPR") {
//// a + b * c - d / (5 + -1) - func() + a[1] + b()
////运算符栈的方法解析表达式
//if (!tk_set.size())
//return;
//int cnt_pa = 0;
//vector<token_info> expr;
//int i = 0;
//int begin, end;
//string replace;
//while (i < tk_set.size()) {
//	for (; i < tk_set.size(); i++) {
//		if (tk_set[i].type == LPARENT && ((!i) || tk_set[i - 1].type != IDENFR)) {
//			cnt_pa++;
//			begin = i;
//			break;
//		}
//	}
//	for (i++; i < tk_set.size(); i++) {
//		if (tk_set[i].type == LPARENT) {
//			cnt_pa++;
//		}
//		if (tk_set[i].type == RPARENT) {
//			cnt_pa--;
//		}
//		if (!cnt_pa) {
//			end = i;
//			parse("EXPR", expr);
//			expr.clear();
//			replace = get_last_result();
//			//replace
//			tk_set.erase(tk_set.begin() + begin, tk_set.begin() + end + 1);
//			tk_set.insert(tk_set.begin() + begin, token_info(IDENFR, replace));
//			i = begin;
//			break;
//		}
//		else {
//			expr.push_back(tk_set[i]);
//		}
//	}
//}
//vector<string> stack;
//vector<string> polish;
//
//int isPos = 1;
//i = 0;
//
//if (tk_set[0].type == PLUS) {
//	isPos = 1;
//	i++;
//}
//else if (tk_set[0].type == MINU) {
//	isPos = 0;
//	string tk = string("0");
//	tk_set.insert(tk_set.begin(), token_info(INTCON, tk));
//	//i++;
//}
//for (int j = i; j < tk_set.size(); j++) {
//	if ((tk_set[j].type == PLUS || tk_set[j].type == MINU) && (tk_set[j - 1].type == PLUS || tk_set[j - 1].type == MINU || tk_set[j - 1].type == MULT || tk_set[j - 1].type == DIV)) {
//		tk_set[j + 1].token = tk_set[j].token + tk_set[j + 1].token;
//		tk_set.erase(tk_set.begin() + j); j--;
//	}
//}
//for (; i < tk_set.size(); i++) {
//	if ((tk_set[i].type == PLUS || tk_set[i].type == MINU)) {
//		while (stack.size()) {
//			polish.push_back(stack[stack.size() - 1]);
//			stack.pop_back();
//		}
//		stack.push_back(tk_set[i].token);
//	}
//	else if (tk_set[i].type == MULT || tk_set[i].type == DIV) {
//		while (stack.size() && (stack[stack.size() - 1] != "+" && stack[stack.size() - 1] != "-")) {
//			polish.push_back(stack[stack.size() - 1]);
//			stack.pop_back();
//		}
//		stack.push_back(tk_set[i].token);
//	}
//	else if (tk_set[i].type == IDENFR) {
//		if (i != tk_set.size() - 1 && tk_set[i + 1].type == LPARENT) {
//			int count_parent = 0;
//			vector<token_info> vt;
//			vt.push_back(tk_set[i]);
//			do {
//				i++;
//				if (tk_set[i].type == LPARENT)
//					count_parent++;
//				else if (tk_set[i].type == RPARENT)
//					count_parent--;
//				vt.push_back(tk_set[i]);
//			} while (count_parent);
//			parse(string("FUNCCALL"), vt);
//			polish.push_back(get_last_result());
//		}
//		else if (i != tk_set.size() - 1 && tk_set[i + 1].type == LBRACK) {
//			int count_parent = 0;
//			vector<token_info> vt;
//			vt.push_back(tk_set[i]);
//			do {
//				i++;
//				if (tk_set[i].type == LBRACK)
//					count_parent++;
//				else if (tk_set[i].type == RBRACK)
//					count_parent--;
//				vt.push_back(tk_set[i]);
//			} while (count_parent);
//			parse(string("ARRAY"), vt);
//			polish.push_back(get_last_result());
//		}
//		else {
//			polish.push_back(tk_set[i].token);
//		}
//	}
//	else if (tk_set[i].type == INTCON) {
//		polish.push_back(tk_set[i].token);
//	}
//	else if (tk_set[i].type == CHARCON) {
//		polish.push_back("\'" + tk_set[i].token + "\'");
//	}
//}
//while (stack.size()) {
//	polish.push_back(stack[stack.size() - 1]);
//	stack.pop_back();
//}
////输出四元式
//string s1 = "";
//string s2 = "";
//for (int i = 0; i < polish.size(); i++) {
//	if (polish[i] == "+" || polish[i] == "-" || polish[i] == "*" || polish[i] == "/") {
//		s1 = stack[stack.size() - 1]; stack.pop_back();
//		s2 = stack[stack.size() - 1]; stack.pop_back();
//		if (polish[i] == "+") {
//			push(string("add"), s2, s1, gen_temp());
//			stack.push_back(get_last_result());
//		}
//		else if (polish[i] == "-") {
//			push(string("sub"), s2, s1, gen_temp());
//			stack.push_back(get_last_result());
//		}
//		else if (polish[i] == "*") {
//			push(string("mult"), s2, s1, gen_temp());
//			stack.push_back(get_last_result());
//		}
//		else if (polish[i] == "/") {
//			push(string("div"), s2, s1, gen_temp());
//			stack.push_back(get_last_result());
//		}
//	}
//	else {
//		stack.push_back(polish[i]);
//	}
//}
//if (stack.size() == 1) {
//	s1 = stack[stack.size() - 1]; stack.pop_back();
//	push("=", s1, "", s1);
//}
//	}