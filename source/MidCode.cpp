#include "MidCode.h"
#include "FILEOperator.h"
#include "MipsGen.h"
#include "MemoryManage.h"


MipsGen mips_gen;
extern int label_cnt;
extern MemoryTable memory_table;
extern string func_name_now;
MidCode::MidCode() { ; }
MidCode::MidCode(string o, string s, string ss, string r) {
	this->op = o;
	this->s1 = s;
	this->s2 = ss;
	this->result = r;
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



void MidCodeGen::push(string op, string s1, string s2, string result) {
	this->mc.push_back(MidCode(op, s1, s2, result));

}

string MidCodeGen::get_last_result(void) {
	return this->mc[mc.size() - 1].result;
}

string MidCodeGen::gen_temp(void) {
	static int cnt = 0;
	string s = string("xxj_temp") + to_string(cnt++);
	memory_table.push(s);
	return s;
}

void MidCodeGen::parse(string type, vector<token_info> tk_set) {
	if (type == "FUNC") {
		if (tk_set.size() == 4)
			return;
		//int\char func_name(int a, int b, int c)
		string s1 = tk_set[0].token;
		string s2 = tk_set[1].token;
		push(string("func"), s1, s2, string(""));
		for (int i = 3; i < tk_set.size(); i += 3) {
			s1 = tk_set[i].token;
			s2 = tk_set[i + 1].token;
			push(string("para"), s1, s2, string(""));
		}
	}
	else if (type == "FUNCCALL") {
		//func_name(expr1, expr2, expr3)
		string s1;
		string s2;
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size(); i++) {
			if (tk_set[i].type == COMMA) {
				parse("EXPR", vt);
				string res = get_last_result();
				push(string("func_push_para"), res, string(""), string(""));
				vt.clear();
			}
			else if (i != tk_set.size() - 1) {
				vt.push_back(tk_set[i]);
			}
		}
		parse("EXPR", vt);
		string res = get_last_result();
		push(string("func_push_para"), res, string(""), string(""));
		vt.clear();
		s1 = tk_set[0].token;
		push(string("func_call"), s1, string(""), gen_temp());
	}
	else if (type == "EXPR") {
		// a + b * c - d / (5 + -1) - func() + a[1] + b()
		//运算符栈的方法解析表达式

		int cnt_pa = 0;
		vector<token_info> expr;
		int i = 0;
		int begin, end;
		string replace;
		while (i < tk_set.size()) {
			for (; i < tk_set.size(); i++) {
				if (tk_set[i].type == LPARENT && ((!i) || tk_set[i - 1].type != IDENFR)) {
					cnt_pa++;
					begin = i;
					break;
				}
			}
			for (i++; i < tk_set.size(); i++) {
				if (tk_set[i].type == LPARENT) {
					cnt_pa++;
				}
				if (tk_set[i].type == RPARENT) {
					cnt_pa--;
				}
				if (!cnt_pa) {
					end = i;
					parse("EXPR", expr);
					expr.clear();
					replace = get_last_result();
					//replace
					tk_set.erase(tk_set.begin() + begin, tk_set.begin() + end + 1);
					tk_set.insert(tk_set.begin() + begin, token_info(IDENFR, replace));
					i = begin;
					break;
				}
				else {
					expr.push_back(tk_set[i]);
				}
			}
		}
		vector<string> stack;
		vector<string> polish;

		int isPos = 1;
		i = 0;

		if (tk_set[0].type == PLUS) {
			isPos = 1;
			i++;
		}
		else if (tk_set[0].type == MINU) {
			isPos = 0;
			string tk = string("0");
			tk_set.insert(tk_set.begin(), token_info(INTCON, tk));
			//i++;
		}
		for (int j = i; j < tk_set.size(); j++) {
			if ((tk_set[j].type == PLUS || tk_set[j].type == MINU) && (tk_set[j - 1].type == PLUS || tk_set[j - 1].type == MINU || tk_set[j - 1].type == MULT || tk_set[j - 1].type == DIV)) {
				tk_set[j + 1].token = tk_set[j].token + tk_set[j + 1].token;
				tk_set.erase(tk_set.begin() + j); j--;
			}
		}
		for (; i < tk_set.size(); i++) {
			if ((tk_set[i].type == PLUS || tk_set[i].type == MINU)) {
				while (stack.size()) {
					polish.push_back(stack[stack.size() - 1]);
					stack.pop_back();
				}
				stack.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == MULT || tk_set[i].type == DIV) {
				while (stack.size() && (stack[stack.size() - 1] != "+" && stack[stack.size() - 1] != "-")) {
					polish.push_back(stack[stack.size() - 1]);
					stack.pop_back();
				}
				stack.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == IDENFR) {
				if (i != tk_set.size() - 1 && tk_set[i + 1].type == LPARENT) {
					int count_parent = 0;
					vector<token_info> vt;
					vt.push_back(tk_set[i]);
					do {
						i++;
						if (tk_set[i].type == LPARENT)
							count_parent++;
						else if (tk_set[i].type == RPARENT)
							count_parent--;
						vt.push_back(tk_set[i]);
					} while (count_parent);
					parse(string("FUNCCALL"), vt);
					polish.push_back(get_last_result());
				}
				else if (i != tk_set.size() - 1 && tk_set[i + 1].type == LBRACK) {
					int count_parent = 0;
					vector<token_info> vt;
					vt.push_back(tk_set[i]);
					do {
						i++;
						if (tk_set[i].type == LBRACK)
							count_parent++;
						else if (tk_set[i].type == RBRACK)
							count_parent--;
						vt.push_back(tk_set[i]);
					} while (count_parent);
					parse(string("ARRAY"), vt);
					polish.push_back(get_last_result());
				}
				else {
					polish.push_back(tk_set[i].token);
				}
			}
			else if (tk_set[i].type == INTCON) {
				polish.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == CHARCON) {
				polish.push_back("\'" + tk_set[i].token + "\'");
			}
		}
		while (stack.size()) {
			polish.push_back(stack[stack.size() - 1]);
			stack.pop_back();
		}
		//输出四元式
		string s1 = "";
		string s2 = "";
		for (int i = 0; i < polish.size(); i++) {
			if (polish[i] == "+" || polish[i] == "-" || polish[i] == "*" || polish[i] == "/") {
				s1 = stack[stack.size() - 1]; stack.pop_back();
				s2 = stack[stack.size() - 1]; stack.pop_back();
				if (polish[i] == "+") {
					push(string("add"), s2, s1, gen_temp());
					stack.push_back(get_last_result());
				}
				else if (polish[i] == "-") {
					push(string("sub"), s2, s1, gen_temp());
					stack.push_back(get_last_result());
				}
				else if (polish[i] == "*") {
					push(string("mult"), s2, s1, gen_temp());
					stack.push_back(get_last_result());
				}
				else if (polish[i] == "/") {
					push(string("div"), s2, s1, gen_temp());
					stack.push_back(get_last_result());
				}
			}
			else {
				stack.push_back(polish[i]);
			}
		}
		if (stack.size() == 1) {
			s1 = stack[stack.size() - 1]; stack.pop_back();
			push("=", s1, "", s1);
		}
		else {
			exit(0);
		}
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
		string s2 = "label_if_head" + to_string(label_cnt);
		push(string("BZ"), s1, s2, string(""));
	}
	else if (type == "CONDITION") {
		//exp1 <= exp2
		string s1;
		string s2;
		string op = "=";
		vector<token_info> vt1, vt2;
		int i;
		for (i = 0; i < tk_set.size(); i++) {
			if (tk_set[i].type == LEQ || tk_set[i].type == LSS || tk_set[i].type == EQL || tk_set[i].type == NEQ || tk_set[i].type == GEQ || tk_set[i].type == GRE) {
				op = tk_set[i].token;
				
				break;
			}
			vt1.push_back(tk_set[i]);
		}
		parse(string("EXPR"), vt1);
		s1 = get_last_result();
		for (i++; i < tk_set.size(); i++) {
			vt2.push_back(tk_set[i]);
		}
		if (vt2.size()) {
			parse(string("EXPR"), vt2);
			s2 = get_last_result();
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
		push(string("LABEL"), string("label_while_begin"), to_string(label_cnt), string(""));
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size() - 1; i++) {
			vt.push_back(tk_set[i]);
		}
		parse(string("CONDITION"), vt);
		string s1 = get_last_result();
		string s2 = string("label_while_end") + to_string(label_cnt);
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
		string s2 = string("label_while_end") + to_string(label_cnt++);
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

		*/
		vector<token_info> expr, con;
		int i;
		for (i = 4; i < tk_set.size(); i++) {
			if (tk_set[i].type == SEMICN) {
				break;
			}
			expr.push_back(tk_set[i]);
		}
		parse("EXPR", expr);
		push("=", get_last_result(), "", tk_set[2].token);
		push("LABEL", "label_for_begin", to_string(label_cnt), "");
		for (i++; i < tk_set.size(); i++) {
			if (tk_set[i].type == SEMICN) {
				break;
			}
			con.push_back(tk_set[i]);
		}
		parse("CONDITION", con);
		push(string("BNZ"), get_last_result(), "label_for_end" + to_string(label_cnt), string(""));

	}
	else if (type == "ARRAY") {
		//iden [expr]
		//[] , iden, expr, t0
		string s1 = tk_set[0].token;
		vector<token_info> vt;
		for (int i = 2; i < tk_set.size() - 1; i++) {
			vt.push_back(tk_set[i]);
		}
		parse("EXPR", vt);
		string s2 = get_last_result();
		push("[]", s1, s2, gen_temp());
	}
}

void MidCodeGen::out() {
	mips_gen.predeal(this->mc);
	for (int i = 0; i < this->mc.size(); i++) {
		write_into_file(mc[i]);
		mips_gen.parse(mc[i]);
	}
}



/*
		// a + b * c - d / (5 + -1) - func() + a[1] + b()
		//运算符栈的方法解析表达式
		vector<string> stack;
		vector<string> polish;
		int isPos = 1;
		int i = 0;
		if (tk_set[0].type == PLUS) {
			isPos = 1;
			i++;
		}
		else if (tk_set[0].type == MINU){
			isPos = 0;
			i++;
		}
		for (; i < tk_set.size(); i++) {
			if ((tk_set[i].type == PLUS || tk_set[i].type == MINU) && tk_set[i+1].type != INTCON ) {
				while (stack.size() && stack[stack.size() - 1] != "(") {
					polish.push_back(stack[stack.size() - 1]);
					stack.pop_back();
				}
				stack.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == MULT || tk_set[i].type == DIV) {
				while (stack.size() && (stack[stack.size() - 1] != "(" || stack[stack.size() - 1] != "+" || stack[stack.size() - 1] != "-")) {
					polish.push_back(stack[stack.size() - 1]);
					stack.pop_back();
				}
				stack.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == LPARENT && tk_set[i - 1].type != IDENFR) {
				stack.push_back(tk_set[i].token);
			}
			else if (tk_set[i].type == RPARENT && tk_set[i - 2].type != IDENFR) {
				while (stack.size() && stack[stack.size() - 1] != "(") {
					polish.push_back(stack[stack.size() - 1]);
					stack.pop_back();
				}
				stack.pop_back();
			}
			else if (tk_set[i].type == IDENFR) {
				if (tk_set[i + 1].type == LPARENT) {
					int count_parent = 1;
					vector<token_info> vt;
					vt.push_back(tk_set[i]);
					do {
						i++;
						if (tk_set[i].type == LPARENT)
							count_parent++;
						else if (tk_set[i].type == RPARENT)
							count_parent--;
						vt.push_back(tk_set[i]);
					} while (count_parent);
					parse(string("FUNCCALL"), vt);
					polish.push_back(get_last_result());
				}
				else if (tk_set[i + 1].type == LBRACK) {
					int count_parent = 1;
					vector<token_info> vt;
					vt.push_back(tk_set[i]);
					do {
						i++;
						if (tk_set[i].type == LBRACK)
							count_parent++;
						else if (tk_set[i].type == RBRACK)
							count_parent--;
						vt.push_back(tk_set[i]);
					} while (count_parent);
					parse(string("ARRAY"), vt);
					polish.push_back(get_last_result());
				}
				else {
					polish.push_back(tk_set[i].token);
				}
			}
			else if(tk.set[i].type == )
		}
*/
