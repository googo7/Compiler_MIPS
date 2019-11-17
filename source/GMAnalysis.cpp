﻿#include <iostream>
#include <string>
#include <stack>
#include "GMAnalysis.h"
#include "FILEOperator.h"
#include "MemoryManage.h"
#include <set>
#include <iostream>
#include <sstream>

//函数宏简介
//WA用来debug,做代码静态检查
//type && token 用来表征词法分析器中单词寄存器的type和token
//getsym 调用词法分析器读入下一个单词
//！！！PUSH表示预读开始前保存现场，通常是现行单词分析(但没有解析输出)完成后使用PUSH记录栈帧
//！！！POP回到PUSH现场，通常是确认进入分支，进入之前使用
//OUT 用于输出到文件

#define WA do{cout << "----------WARNING-------" << __LINE__ << "---------" << endl;}while(0)
#define type (tk.now_token.type)
#define token (tk.now_token.token)
#define getsym do{(tk.get_token());}while(0)
#define PUSH do{(sp.push(tk.ch_pt - token.size())); line_reg = tk.line;}while(0)
#define POP do{tk.ch_pt = sp.top(); sp.pop(); tk.get_token(); tk.line = line_reg;}while(0)
#define OUT(A) do{write_into_file(A); if (midcode_flag) {midcode_vt.push_back(replace(tk.now_token));}}while(0)
#define OUT_DEF(A) do{write_into_file(A); if (midcode_flag) {midcode_vt.push_back((tk.now_token));}}while(0)
#define OUT_LABEL(A) do{write_into_file(A);}while(0)
#define push_into_symtab(A, B, C, D) do{if(!D && isLocal) symtab.local_push(iden, A, B, C, -1);else if(!D && !isLocal) symtab.global_push(iden, A, B, C, -1); else if (D && isLocal) symtab.local_push(iden, A, B, C, local_cnt++); else symtab.global_push(iden, A, B, C, global_cnt++);} while(0)  //添加到符号表，分别为isConst,type,value,addr,默认具有iden
int isLocal = 0;    //表示是否处在local内或在global作用域内
int line_reg = 0;	//行号寄存器，需要在预读保留现场的成员之一

int global_cnt = 0;	//保留global变量相对地址计数器
int local_cnt = 0;	//保留local变量的相对地址计数器
int label_cnt = 0;
vector<token_info> midcode_vt{}; //用于保存用于解析出四元式的容器，注意及时清空
int midcode_flag = 0;//控制是否进入midcode_vt的开关
//WARNING!
string func_name_now = "";
int func_var_pt = 0;
MemoryTable memory_table;



extern string file_string;



gm_analyse::gm_analyse() {
	this->tk = tk_analyse();
	this->er = error_analyse();
	this->symtab = Symtab();
	this->mc_gen = MidCodeGen();
}

void gm_analyse::isProgram() {
	if (type == CONSTTK) {
		isConstDeclaration();
	}
	if (type == INTTK || type == CHARTK) {
		//pre-read
		PUSH; getsym; 
		if (type == IDENFR) {
			getsym;
			if (type != LPARENT) {
				POP;
				isVariDeclaration();
			}
			else {
				POP;
			}
		}
	}
	while (type == INTTK || type == CHARTK || type == VOIDTK) {
		if (type == INTTK || type == CHARTK) {
			into_func();
			isReturnFunction();
			out_func();
		}
		else if (type == VOIDTK) {
			PUSH; getsym;
			if (type == MAINTK) {
				POP;  break;
			}
			else {
				POP;
				into_func();
				isVoidFunction();
				out_func();
			}
		}
	}

	//-------------main function ----------
	into_func();
	isMainFunction();
	out_func();

	vector<var_info> vv = symtab.global_var_table;
	for (int i = 0; i < vv.size(); i++) {
		memory_table.push("", vv[i]);
	}
	//-------------OUT--------------------
	string s("<程序>");
	OUT_LABEL(s);
}
void gm_analyse::isMainFunction(void){
	//--------------main function-----------------------
	begin_midcode();
	if (type == VOIDTK) {
		OUT(tk.now_token); getsym;
	} //else error
	if (type == MAINTK) {
		func_name_now = "main";
		OUT(tk.now_token); getsym;
	} //else error
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	} //else error
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	} //else error
	end_midcode("FUNC");
	if (type == LBRACE) {
		OUT(tk.now_token); getsym;
	}
	isComStatement();

	if (type == RBRACE) {
		OUT(tk.now_token); getsym;
	}
	begin_midcode();
	end_midcode("EXIT");
	//---------------------pass-----------------
	string s("<主函数>");
	OUT_LABEL(s);
}
void gm_analyse::isConstDeclaration(void){
	if (type == CONSTTK) {
		OUT(tk.now_token); getsym;
	} //else error
	isConstDefinition();
	if (type == SEMICN) {
		OUT(tk.now_token); getsym;
	} //else 
	while (type == CONSTTK) {
		OUT(tk.now_token); getsym;
		isConstDefinition();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
	}
	//----------------pass--------------------
	string s("<常量说明>");
	OUT_LABEL(s);
}
void gm_analyse::isConstDefinition(void){
	string iden;
	if (type == INTTK) {
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			iden = token;
			OUT_DEF(tk.now_token); getsym;
		}//else
		if (type == ASSIGN) {
			OUT(tk.now_token); getsym;
		}//else
		int num = isInteger();
		push_into_symtab(1, INT, num, 0);
		while (type == COMMA) {
			OUT(tk.now_token); getsym;
			if (type == IDENFR) {
				iden = token;
				OUT_DEF(tk.now_token); getsym;
			}//else
			if (type == ASSIGN) {
				OUT(tk.now_token); getsym;
			}//else
			int num = isInteger();
			push_into_symtab(1, INT, num, 0);
		}
	}
	else if (type == CHARTK) {
		char c = 0;
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			iden = token;
			OUT_DEF(tk.now_token); getsym;
		}//else
		if (type == ASSIGN) {
			OUT(tk.now_token); getsym;
		} //else
		if (type == CHARCON) {
			
			stringstream ss;
			ss << token;
			ss >> c;
			OUT(tk.now_token); getsym;
		}//else
		else if (type == CHARERROR) {
			string code("a");
			er.push(tk.line, code);
			getsym;
		}
		push_into_symtab(1, CHAR, c, 0);

		while (type == COMMA) {
			OUT(tk.now_token); getsym;
			if (type == IDENFR) {
				iden = token;
				OUT_DEF(tk.now_token); getsym;
			}//else
			if (type == ASSIGN) {
				OUT(tk.now_token); getsym;
			}//else
			if (type == CHARCON) {
				stringstream ss;
				ss << token;
				ss >> c;
				OUT(tk.now_token); getsym;
			}
			else if (type == CHARERROR) {
				string code("a");
				er.push(tk.line, code);
				getsym;
			}
			push_into_symtab(1, CHAR, c, 0);
			//else
		}
	}//else
	//---------------pass------------
	string s("<常量定义>");
	OUT_LABEL(s);
}
void gm_analyse::isVariDeclaration(void){
	isVariDefinition();
	if (type == SEMICN) {
		OUT(tk.now_token); getsym;
	}//else
	while (type == INTTK || type == CHARTK) {
		PUSH; getsym;
		if (type == IDENFR) {
			getsym;
			if (type != LPARENT) {
				POP;
				isVariDefinition();
				if (type == SEMICN) {
					OUT(tk.now_token); getsym;
				}
			}
			else {
				POP;
				break;
			}
		}
	}
	//----------------pass-------------
	string s("<变量说明>");
	OUT_LABEL(s);
}
void gm_analyse::isVariDefinition(void){
	string iden;
	int res = isTypeIden();
	if (type == IDENFR) {
		iden = token;
		OUT_DEF(tk.now_token); getsym;
	
	}//else
	if (type == LBRACK) {
		OUT(tk.now_token); getsym;
		int num = isUnsigned();
		//else
		push_into_symtab(0, res, -1, 1);
		symtab.adjust_size(iden, num);
		if (isLocal)
			local_cnt += num - 1;
		else
			global_cnt += num - 1;

		if (type == RBRACK) {
			OUT(tk.now_token); getsym ;
		}
	}//not else
	else {
		push_into_symtab(0, res, -1, 1);
	}while (type == COMMA) {
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			iden = token;
			OUT_DEF(tk.now_token); getsym;
		}
		if (type == LBRACK) {
			OUT(tk.now_token); getsym;
			int num = isUnsigned();
			push_into_symtab(0, res, -1, 1);
			symtab.adjust_size(iden, num);
			if (isLocal)
				local_cnt += num - 1;
			else
				global_cnt += num - 1;

			if (type == RBRACK) {
				OUT(tk.now_token); getsym;
			}//else

		} 
		else {
			push_into_symtab(0, res, -1, 1);
		}
	}
	//----------------pass-------------
	string s("<变量定义>");
	OUT_LABEL(s);
}
void gm_analyse::isReturnFunction(void){
	begin_midcode();

	isDeclarationHeader();
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	isVariTable();
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}//else

	end_midcode(string("FUNC"));

	if (type == LBRACE) {
		OUT(tk.now_token); getsym;
	}//else
	isComStatement();
	if (type == RBRACE) {
		OUT(tk.now_token); getsym;
	}
	begin_midcode();
	end_midcode("ENDFUNC");
	//-------------pass----------------
	string s("<有返回值函数定义>");
	OUT_LABEL(s);
}
void gm_analyse::isDeclarationHeader(void){
	if (type == INTTK) {
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			symtab.func_push(token, vector<int>{}, INT);
			func_name_now = token;
			OUT_DEF(tk.now_token); getsym;
		}
	}
	else if (type == CHARTK) {
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			symtab.func_push(token, vector<int>{}, CHAR);
			OUT_DEF(tk.now_token); getsym;
		}
	}//else
	//------------pass-------------------
	string s("<声明头部>");
	OUT_LABEL(s);
}
int gm_analyse::isInteger(void){
	int plus = 1;
	if (type == PLUS || type == MINU) {
		if (type == MINU)
			plus = 0;
		OUT(tk.now_token); getsym;
	}
	int num = isUnsigned();
	//----------------pass------------------------
	string s("<整数>");
	OUT_LABEL(s);
	if (plus)
		return num;
	else
		return 0 - num;
}
int gm_analyse::isUnsigned(void){
	int num = 0;
	if (type == INTCON) {
		stringstream iss;
		iss << token;
		
		iss >> num;
		OUT(tk.now_token); getsym;
	}
	else if (type == INTERROR) {
		string code("a");
		er.push(tk.line, code);
		getsym;
	}
	//else
	//------------------pass-----------------
	string s("<无符号整数>");
	OUT_LABEL(s);
	return num;
}
char gm_analyse::isChar(void){
	char c = 'a';
	if (type == CHARCON) {
		stringstream  ss;
		ss << token;
		ss >> c;
		OUT(tk.now_token); getsym;
	}
	else if (type == CHARERROR) {
		string code("a");
		er.push(tk.line, code);
		getsym;
	}
	//else
	//-----------pass-------------
	return c;
}
void gm_analyse::isVariTable(void){
	string s("<参数表>");
	vector<int> vt{};
	if (type == RPARENT) {
		//empty
		OUT_LABEL(s);
		return;
	}
	int res = isTypeIden();
	if (type == IDENFR) {
		string iden = token;
		OUT_DEF(tk.now_token);push_into_symtab(0, res, -1, 1); getsym;
	}
	vt.push_back(res);
	while (type == COMMA) {
		OUT(tk.now_token); getsym;
		
		res = isTypeIden();
		if (type == IDENFR) {
			string iden = token;
			OUT_DEF(tk.now_token); push_into_symtab(0, res, -1, 1);getsym;//要求必须OUT后修改，否则修改后replace生效
		}//else
		vt.push_back(res);
	}
	//------------pass-----------------
	OUT_LABEL(s);
	symtab.func_table[symtab.func_table.size() - 1].v_table = vt;
}
void gm_analyse::isValueVariTable(void){
	string s("<值参数表>");
	if (type == RPARENT) {
		//empty
		OUT_LABEL(s);
		return;
	}
	isExpression();
	while (type == COMMA) {
		OUT(tk.now_token); getsym;
		isExpression();
	}
	//----------pass------------------
	OUT_LABEL(s);
}
int gm_analyse::isTypeIden(void) {
	int res = 0;
	if (type == INTTK || type == CHARTK) {
		if (type == INTTK)
			res = INT;
		else
			res = CHAR;
		OUT(tk.now_token); getsym;
	}
	//-----------pass--------------
	return res;
}
void gm_analyse::isComStatement(void){
	if (type == CONSTTK) {
		isConstDeclaration();
	}
	if (type == INTTK || type == CHARTK) {
		isVariDeclaration();
	}
	isStatementColumn();
	//------------------pass-----------------
	string s("<复合语句>");
	OUT_LABEL(s);
}
void gm_analyse::isStatementColumn(void){
	string s("<语句列>");
	if (type == RBRACE) {
		OUT_LABEL(s);
		return;
	}
	while (type == IFTK || type == WHILETK || type == FORTK || type == DOTK || type == LBRACE || type == IDENFR || type == SCANFTK || type == PRINTFTK || type == SEMICN || type == RETURNTK) {
		isStatement();
	}
	//------------------pass-----------------
	OUT_LABEL(s);
}
void gm_analyse::isStatement(void){
	string s("<语句>");
	if (type == IFTK) {
		isConditionStatement();
	}
	else if (type == WHILETK || type == FORTK || type == DOTK) {
		isLoopStatement();
	}
	else if (type == LBRACE) {
		OUT(tk.now_token); getsym;
		isStatementColumn();
		if (type == RBRACE) {
			OUT(tk.now_token); getsym;
		}
	}
	else if (type == SCANFTK) {
		isScanfStatement();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
	}
	else if (type == PRINTFTK) {
		isPrintfStatement();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
	}
	else if (type == RETURNTK) {
		isReturnStatement();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
	}
	else if (type == SEMICN) {
		OUT(tk.now_token); getsym;
	}
	else if (type == IDENFR) {
		PUSH; getsym;
		if (type == LPARENT) {
			POP;
			if(type == IDENFR && symtab.func_lookup(token).return_type != VOID){
				isReturnFunctionCall();
			}
			else if (type == IDENFR && symtab.func_lookup(token).return_type == VOID) {
				isVoidFunctionCall();
			}
			if (type == SEMICN) {
				OUT(tk.now_token); getsym;
			}//else
		}
		else {
			POP;
			isAssignStatement();
			if (type == SEMICN) {
				OUT(tk.now_token); getsym;
			}//else
		}
	}//else
	OUT_LABEL(s);
}
void gm_analyse::isConditionStatement(void){
	begin_midcode();
	if (type == IFTK) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	isCondition();
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	end_midcode("IFHEAD");

	isStatement();
	mc_gen.push(string("GOTO"), string("label_if_tail") + to_string(label_cnt), string(""), string(""));
	mc_gen.push(string("LABEL"), string("label_if_head"), to_string(label_cnt), string(""));
	if (type == ELSETK) {
		OUT(tk.now_token); getsym;
		isStatement();
		
	}//not else
	mc_gen.push(string("LABEL"), string("label_if_tail"), to_string(label_cnt++), string(""));
	//----------------------pass---------------------
	string s("<条件语句>");
	OUT_LABEL(s);
}
void gm_analyse::isLoopStatement(void){
	
	if (type == WHILETK) {
		begin_midcode();
		OUT(tk.now_token); getsym;
		if (type == LPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		isCondition();
		if (type == RPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		end_midcode("WHILE");
		isStatement();
		mc_gen.push(string("GOTO"), string("label_while_begin") + to_string(label_cnt), string(""), string(""));
		mc_gen.push(string("LABEL"), string("label_if_end"), to_string(label_cnt++), string(""));
	}
	else if (type == DOTK) {
		mc_gen.push(string("LABEL"), string("label_dowhile"), to_string(label_cnt), string(""));
		OUT(tk.now_token); getsym;
		isStatement();
		begin_midcode();
		if (type == WHILETK) {
			OUT(tk.now_token); getsym;
		}//else
		if (type == LPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		isCondition();
		if (type == RPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		end_midcode("DOWHILE");
	}
	else if (type == FORTK) {
		string s1;
		string s2;
		string result;
		string op;
		begin_midcode();
		OUT(tk.now_token); getsym;
		if (type == LPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		if (type == IDENFR) {
			OUT(tk.now_token); getsym;
		}//else
		if (type == ASSIGN) {
			OUT(tk.now_token); getsym;
		}//else
		isExpression();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
		isCondition();
		if (type == SEMICN) {
			OUT(tk.now_token); getsym;
		}//else
		if (type == IDENFR) {
			result = token;
			OUT(tk.now_token); getsym;
		}//else
		if (type == ASSIGN) {
			OUT(tk.now_token); getsym;
		}//else
		if (type == IDENFR) {
			s1 = token;
			OUT(tk.now_token); getsym;
		}//else
		if (type == PLUS || type == MINU) {
			if (type == PLUS)
				op = "add";
			else
				op = "sub";
			OUT(tk.now_token); getsym;
		}//else
		s2 = to_string(isStep());
		if (type == RPARENT) {
			OUT(tk.now_token); getsym;
		}//else
		end_midcode("FOR");
		isStatement();
		mc_gen.push(op, s1, s2, result);
		mc_gen.push("GOTO", "label_for_begin" + to_string(label_cnt), "", "");
		mc_gen.push("LABEL", "label_for_end", to_string(label_cnt++), "");
	}//else

	//---------------pass--------------------
	string s("<循环语句>");
	OUT_LABEL(s);
}
void gm_analyse::isCondition(void){
	isExpression();
	if (type == LSS || type == LEQ || type == GRE || type == GEQ || type == NEQ || type == EQL) {
		isRelationshipOperator();
		isExpression();
	}// not else
	//-----------------pass--------------
	string s("<条件>");
	OUT_LABEL(s);
}
void gm_analyse::isAssignStatement(void){
	string s1, s2, result;
	s2 = "";
	if (type == IDENFR) {
		result = token;
		OUT(tk.now_token); getsym;
		if (type == LBRACK) {
			OUT(tk.now_token); getsym;
			begin_midcode();
			isExpression();
			end_midcode("EXPR");
			s2 = mc_gen.get_last_result();
			if (type == RBRACK) {
				OUT(tk.now_token); getsym;
			}
		}
	}
	if (type == ASSIGN) {
		OUT(tk.now_token); getsym;
	}
	begin_midcode();
	isExpression();
	end_midcode("EXPR");
	s1 = mc_gen.get_last_result();
	mc_gen.push("=", s1, s2, result);
	//----------------pass---------------
	string s("<赋值语句>");
	OUT_LABEL(s);
}
void gm_analyse::isScanfStatement(void){
	if (type == SCANFTK) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == IDENFR) {
		mc_gen.push("scanf", token, "", "");
		OUT(tk.now_token); getsym;
	}//else
	while (type == COMMA) {
		OUT(tk.now_token); getsym;
		if (type == IDENFR) {
			mc_gen.push("scanf", token, "", "");
			OUT(tk.now_token); getsym;
		}
	}
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}
	//------------pass---------------
	string s("<读语句>");
	OUT_LABEL(s);
}
void gm_analyse::isPrintfStatement(void){
	string s1 = "";
	string s2 = "";
	if (type == PRINTFTK) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == STRCON) {
		s1 = "\"" + token + "\"";
		OUT(tk.now_token); getsym;
		string ss("<字符串>");
		OUT_LABEL(ss);
		if (type == COMMA) {
			OUT(tk.now_token); getsym;
			begin_midcode();
			isExpression();
			end_midcode("EXPR");
			s2 = mc_gen.get_last_result();
		}
	}
	else if (type == STRERROR) {
		string code("a");
		er.push(tk.line, code);
		getsym;
		if (type == COMMA) {
			OUT(tk.now_token); getsym;
			isExpression();
		}
	}
	else{
		begin_midcode();
		isExpression();
		end_midcode("EXPR");
		s2 = mc_gen.get_last_result();
	}
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}
	//------------pass---------------
	string s("<写语句>");
	mc_gen.push("printf", s1, s2, "");
	OUT_LABEL(s);
}
void gm_analyse::isReturnStatement(void){
	string s1 = "";
	if (type == RETURNTK) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
		begin_midcode();
		isExpression();
		end_midcode("EXPR");
		s1 = mc_gen.get_last_result();
		mc_gen.push("ret", s1, "", "");
		if (type == RPARENT) {
			OUT(tk.now_token); getsym;
		}
	}
	//------------pass---------------
	string s("<返回语句>");
	OUT_LABEL(s);
}
void gm_analyse::isPlusOperator(void){
	if (type == PLUS || type == MINU) {
		OUT(tk.now_token); getsym;
	}
}
void gm_analyse::isMultOperator(void){
	if (type == MULT || type == DIV) {
		OUT(tk.now_token); getsym;
	}
}
void gm_analyse::isRelationshipOperator(void){
	if (type == LSS || type == LEQ || type == GRE || type == GEQ || type == NEQ || type == EQL) {
		OUT(tk.now_token); getsym;
	}
}
void gm_analyse::isVoidFunction(void){
	
	begin_midcode();
	if (type == VOIDTK) {
		OUT(tk.now_token); getsym;
	}//else
	if (type == IDENFR) {
		func_name_now = token;
		symtab.func_push(token, vector<int>{}, VOID);
		OUT(tk.now_token); getsym;
	}//else
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	isVariTable();
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	end_midcode(string("FUNC"));

	if (type == LBRACE) {
		OUT(tk.now_token); getsym;
	}//else
	isComStatement();
	if (type == RBRACE) {
		OUT(tk.now_token); getsym;
	}//else
	begin_midcode();
	end_midcode("ENDFUNC");
	//-----------------------pass-----------------------
	string s("<无返回值函数定义>");
	OUT_LABEL(s);
}
void gm_analyse::isReturnFunctionCall(void){
	begin_midcode();
	if (type == IDENFR) {
		OUT(tk.now_token); getsym;
	}
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	isValueVariTable();
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}
	end_midcode(string("FUNCCALL"));

	string s("<有返回值函数调用语句>");
	OUT_LABEL(s);
}
void gm_analyse::isVoidFunctionCall(void) {
	if (type == IDENFR) {
		OUT(tk.now_token); getsym;
	}
	if (type == LPARENT) {
		OUT(tk.now_token); getsym;
	}//else
	isValueVariTable();
	if (type == RPARENT) {
		OUT(tk.now_token); getsym;
	}
	string s("<无返回值函数调用语句>");
	OUT_LABEL(s);
}
void gm_analyse::isExpression(void){
	if (type == PLUS || type == MINU) {
		OUT(tk.now_token); getsym;
	}// not else
	isTerm();
	while (type == PLUS || type == MINU) {
		isPlusOperator();
		isTerm();
	}
	//---------------pass------------------
	string s("<表达式>");
	OUT_LABEL(s);
}
void gm_analyse::isTerm(void){
	isFactor();
	while (type == MULT || type == DIV) {
		isMultOperator();
		isFactor();
	}
	//--------------pass------------------------
	string s("<项>");
	OUT_LABEL(s);
}
void gm_analyse::isFactor(void) {
	if (type == IDENFR) {
		PUSH; getsym;
		if (type == LPARENT) {
			POP;
			isReturnFunctionCall();
		}
		else {
			POP; 
			OUT(tk.now_token); getsym;
			if (type == LBRACK) {
				OUT(tk.now_token); getsym;
				isExpression();
				if (type == RBRACK) {
					OUT(tk.now_token); getsym;
				}
			}// not else

		}
	}          
	else if (type == LPARENT) {
		OUT(tk.now_token); getsym;
		isExpression();
		if (type == RPARENT) {
			OUT(tk.now_token); getsym;
		}
	}
	else if (type == PLUS || type == MINU || type == INTCON || type == INTERROR) {
		isInteger();
	}
	else if (type == CHARCON || type == CHARERROR) {
		isChar();
	}//else
	//------------pass----------------
	string s("<因子>");
	OUT_LABEL(s);
}
int gm_analyse::isStep(void) {
	int res = isUnsigned();
	//---------pass-----------
	string s("<步长>");
	OUT_LABEL(s);
	return res;
}

void gm_analyse::into_func(void) {
	isLocal = 1;
	local_cnt = 0;
}

void gm_analyse::out_func(void) {
	isLocal = 0;
	vector<var_info> vv = symtab.local_var_table;
	for (int i = 0; i < vv.size(); i++) {
		memory_table.push(func_name_now, vv[i]);
	}
	for (int i = 0; i < memory_table.temp.size(); i++)
		memory_table.push(func_name_now, memory_table.temp[i]);
	symtab.clear();
	memory_table.temp.clear();
}

void gm_analyse::begin_midcode(void) {
	midcode_flag += 1;
}

void gm_analyse::end_midcode(string s) {
	midcode_flag -= 1;
	if (midcode_flag)
		return;
	mc_gen.parse(s, midcode_vt);
	midcode_vt.clear();
}

token_info gm_analyse::replace(token_info t) {
	if (type != IDENFR)
		return t;
	else {
		if (symtab.var_lookup(token).isConst == 1) {
			if (symtab.var_lookup(token)._type == INT) {
				string a = to_string(symtab.var_lookup(token).value);
				return token_info(INTCON, a);
			}
			else {
				string a = "\'" + (char)symtab.var_lookup(token).value + '\'';
				return token_info(CHARCON, a);
			}
		}
		else
			return t;
	}
}