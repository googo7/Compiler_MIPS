#include <iostream>
#include <string>
#include <stack>
#include "er_GMAnalysis.h"
#include "er_FILEOperator.h"
#include <set>

//函数宏简介
//WA用来debug,做代码静态检查
//type && token 用来表征词法分析器中单词寄存器的type和token
//getsym 调用词法分析器读入下一个单词
//！！！PUSH表示预读开始前保存现场，通常是现行单词分析(但没有解析输出)完成后使用PUSH记录栈帧
//！！！POP回到PUSH现场，通常是确认进入分支，进入之前使用
//OUT 用于输出到文件

#define er_WA do{cout << "----------WARNING-------" << __LINE__ << "---------" << endl;}while(0)
#define er_type (er_tk.now_token.type)
#define er_token (er_tk.now_token.token)
#define er_getsym do{er_semicn_line = er_tk.line; (er_tk.get_token());}while(0)
#define er_PUSH do{(er_sp.push(er_tk.ch_pt - er_token.size())); er_line_reg = er_tk.line;}while(0)
#define er_POP do{er_tk.ch_pt = er_sp.top(); er_sp.pop(); er_tk.get_token(); er_tk.line = er_line_reg;}while(0)
#define er_OUT(A) do{er_write_into_file(A);}while(0)
#define er_push_into_symtab(A, B) do{if (er_isLocal) isPush = symtab.local_push(iden, A, B); else isPush = symtab.global_push(iden, A, B);} while(0)
int er_isLocal = 0;
int er_line_reg = 0;
vector<int> er_vvt_now{};

int er_semicn_line;


int er_no_return_flag = 0;
int er_return_flag = 0;
int er_return_type_flag = 0;
int er_has_return = 0;
//WARNING!

extern string er_file_string;

er_gm_analyse::er_gm_analyse() {
	this->er_tk = er_tk_analyse();
	this->er_er = er_error_analyse();
	this->symtab = er_Symtab();
}

void er_gm_analyse::isProgram() {
	if (er_type == CONSTTK) {
		isConstDeclaration();
	}
	if (er_type == INTTK || er_type == CHARTK) {
		//pre-read
		er_PUSH; er_getsym; 
		if (er_type == IDENFR) {
			er_getsym;
			if (er_type != LPARENT) {
				er_POP;
				isVariDeclaration();
			}
			else {
				er_POP;
			}
		}
	}
	while (er_type == INTTK || er_type == CHARTK || er_type == VOIDTK) {
		if (er_type == INTTK || er_type == CHARTK) {
			into_func();
			isReturnFunction();
			out_func();
		}
		else if (er_type == VOIDTK) {
			er_PUSH; er_getsym;
			if (er_type == MAINTK) {
				er_POP;  break;
			}
			else {
				er_POP;
				into_func();
				isVoidFunction();
				out_func();
			}
		}
	}

	//-------------main function ----------
	into_func();
	er_no_return_flag = 1;//开始检测
	isMainFunction();
	er_no_return_flag = 0;//复位
	out_func();
	//-------------OUT--------------------
	string s("<程序>");
	er_OUT(s);
}
void er_gm_analyse::isMainFunction(void){
	//--------------main function-----------------------
	if (er_type == VOIDTK) {
		er_OUT(er_tk.now_token); er_getsym;
	} //else error
	if (er_type == MAINTK) {
		er_OUT(er_tk.now_token); er_getsym;
	} //else error
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	} //else error
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	} //else error
	else {
		er_er.push(er_tk.line, string("l"));
	}
	if (er_type == LBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	isComStatement();

	if (er_type == RBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	//---------------------pass-----------------
	string s("<主函数>");
	er_OUT(s);
}
void er_gm_analyse::isConstDeclaration(void){
	if (er_type == CONSTTK) {
		er_OUT(er_tk.now_token); er_getsym;
	} //else error
	isConstDefinition();
	if (er_type == SEMICN) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_semicn_line, string("k"));
	}
	while (er_type == CONSTTK) {
		er_OUT(er_tk.now_token); er_getsym;
		isConstDefinition();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}
		else {
			er_er.push(er_semicn_line, string("k"));
		}
	}
	//----------------pass--------------------
	string s("<常量说明>");
	er_OUT(s);
}
void er_gm_analyse::isConstDefinition(void){
	string iden;
	if (er_type == INTTK) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			iden = er_token;
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == ASSIGN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (!isInteger())
			er_er.push(er_tk.line, string("o"));
		int isPush;
		er_push_into_symtab(1, INT);
		if (!isPush) {
			er_er.push(er_tk.line, string("b"));
		}
		while (er_type == COMMA) {
			er_OUT(er_tk.now_token); er_getsym;
			if (er_type == IDENFR) {
				iden = er_token;
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			if (er_type == ASSIGN) {
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			int isPush;
			if (!isInteger())
				er_er.push(er_tk.line, string("o"));
			er_push_into_symtab(1, INT);
			if (!isPush) {
				er_er.push(er_tk.line, string("b"));
			}
		}
	}
	else if (er_type == CHARTK) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			iden = er_token;
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == ASSIGN) {
			er_OUT(er_tk.now_token); er_getsym;
		} //else
		if (er_type == CHARCON) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else if (er_type == CHARERROR) {
			string code("a");
			er_er.push(er_tk.line, code);
			er_er.push(er_tk.line, string("o"));
			er_getsym;
		}
		else {
			er_er.push(er_tk.line, string("o"));
			er_getsym;
		}
		int isPush;
		er_push_into_symtab(1, CHAR);
		if (!isPush) {
			er_er.push(er_tk.line, string("b"));
		}
		while (er_type == COMMA) {
			er_OUT(er_tk.now_token); er_getsym;
			if (er_type == IDENFR) {
				iden = er_token;
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			if (er_type == ASSIGN) {
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			if (er_type == CHARCON) {
				er_OUT(er_tk.now_token); er_getsym;
			}
			else if (er_type == CHARERROR) {
				string code("a");
				er_er.push(er_tk.line, code);
				er_getsym;
			}
			int isPush;
			er_push_into_symtab(1, CHAR);
			if (!isPush) {
				er_er.push(er_tk.line, string("b"));
			}
			//else
		}
	}//else
	//---------------pass------------
	string s("<常量定义>");
	er_OUT(s);
}
void er_gm_analyse::isVariDeclaration(void){
	isVariDefinition();
	if (er_type == SEMICN) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_semicn_line, string("k"));
	}
	while (er_type == INTTK || er_type == CHARTK) {
		er_PUSH; er_getsym;
		if (er_type == IDENFR) {
			er_getsym;
			if (er_type != LPARENT) {
				er_POP;
				isVariDefinition();
				if (er_type == SEMICN) {
					er_OUT(er_tk.now_token); er_getsym;
				}
				else {
					er_er.push(er_semicn_line, string("k"));
				}
			}
			else {
				er_POP;
				break;
			}
		}
	}
	//----------------pass-------------
	string s("<变量说明>");
	er_OUT(s);
}
void er_gm_analyse::isVariDefinition(void){
	string iden;
	int res = isTypeIden();
	if (er_type == IDENFR) {
		iden = er_token;
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LBRACK) {
		er_OUT(er_tk.now_token); er_getsym;
		isUnsigned();
		//else
		if (er_type == RBRACK) {
			er_OUT(er_tk.now_token); er_getsym;
		}
		else {
			er_er.push(er_tk.line, string("m"));
		}
	}//not else
	int isPush;
	er_push_into_symtab(0, res);
	if (!isPush) {
		er_er.push(er_tk.line, string("b"));
	}
	while (er_type == COMMA) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			iden = er_token;
			er_OUT(er_tk.now_token); er_getsym;
		}
		if (er_type == LBRACK) {
			er_OUT(er_tk.now_token); er_getsym;
			isUnsigned();
			if (er_type == RBRACK) {
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			else {
				er_er.push(er_tk.line, string("m"));
			}
		}
		int isPush;
		er_push_into_symtab(0, res);
		if (!isPush) {
			er_er.push(er_tk.line, string("b"));
		}
	}
	//----------------pass-------------
	string s("<变量定义>");
	er_OUT(s);
}
void er_gm_analyse::isReturnFunction(void){
	isDeclarationHeader();
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	isVariTable();
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	else {
		er_er.push(er_tk.line, string("l"));
	}
	if (er_type == LBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	er_has_return = 0;
	er_return_flag = 1;
	isComStatement();
	er_return_flag = 0;
	if (er_has_return == 0) {
		er_er.push(er_tk.line, string("h"));
	}
	if (er_type == RBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	//-------------pass----------------
	string s("<有返回值函数定义>");
	er_OUT(s);
}
void er_gm_analyse::isDeclarationHeader(void){
	if (er_type == INTTK) {
		er_return_type_flag = INT;
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			symtab.func_push(er_token, vector<int>{}, INT);
			er_OUT(er_tk.now_token); er_getsym;
		}
	}
	else if (er_type == CHARTK) {
		er_return_type_flag = CHAR;
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			symtab.func_push(er_token, vector<int>{}, CHAR);
			er_OUT(er_tk.now_token); er_getsym;
		}
	}//else
	//------------pass-------------------
	string s("<声明头部>");
	er_OUT(s);
}
bool er_gm_analyse::isInteger(void){
	if (er_type == PLUS || er_type == MINU) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	return isUnsigned();
	//----------------pass------------------------
	string s("<整数>");
	er_OUT(s);
}
bool er_gm_analyse::isUnsigned(void){
	if (er_type == INTCON) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else if (er_type == INTERROR) {
		string code("a");
		er_er.push(er_tk.line, code);
		er_getsym;
		return false;
	}
	else {
		er_er.push(er_tk.line, string("o"));
		er_getsym;
	}
	//else
	//------------------pass-----------------
	string s("<无符号整数>");
	er_OUT(s);
	return true;
}
void er_gm_analyse::isChar(void){
	if (er_type == CHARCON) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else if (er_type == CHARERROR) {
		string code("a");
		er_er.push(er_tk.line, code);
		er_getsym;
	}
	//else
	//-----------pass-------------
}
void er_gm_analyse::isVariTable(void){
	string s("<参数表>");
	vector<int> vt{};
	if (er_type == RPARENT) {
		//empty
		er_OUT(s);
		return;
	}
	int res = isTypeIden();
	if (er_type == IDENFR) {
		string iden = er_token;
		int isPush;
		er_push_into_symtab(0, res);
		if (!isPush) {
			er_er.push(er_tk.line, string("b"));
		}
		er_OUT(er_tk.now_token); er_getsym;
	}
	vt.push_back(res);
	while (er_type == COMMA) {
		er_OUT(er_tk.now_token); er_getsym;
		res = isTypeIden();
		if (er_type == IDENFR) {
			string iden = er_token;
			int isPush;
			er_push_into_symtab(0, res);
			if (!isPush) {
				er_er.push(er_tk.line, string("b"));
			}
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		vt.push_back(res);
	}
	//------------pass-----------------
	er_OUT(s);
	symtab.func_table[symtab.func_table.size() - 1].v_table = vt;
}
void er_gm_analyse::isValueVariTable(void){
	vector<int> vvt{};
	string s("<值参数表>");
	if (er_type == RPARENT) {
		//empty
		er_OUT(s);
		er_vvt_now = vvt;
		return;
	}
	int res = isExpression();
	vvt.push_back(res);
	while (er_type == COMMA) {
		er_OUT(er_tk.now_token); er_getsym;
		res = isExpression();
		vvt.push_back(res);
	}
	//----------pass------------------
	er_OUT(s);
	er_vvt_now = vvt;
}
int er_gm_analyse::isTypeIden(void){
	int res = 0;
	if (er_type == INTTK || er_type == CHARTK) {
		if (er_type == INTTK)
			res = INT;
		else
			res = CHAR;
		er_OUT(er_tk.now_token); er_getsym;
	}
	//-----------pass--------------
	return res;
}
void er_gm_analyse::isComStatement(void){
	if (er_type == CONSTTK) {
		isConstDeclaration();
	}
	if (er_type == INTTK || er_type == CHARTK) {
		isVariDeclaration();
	}
	isStatementColumn();
	//------------------pass-----------------
	string s("<复合语句>");
	er_OUT(s);
}
void er_gm_analyse::isStatementColumn(void){
	string s("<语句列>");
	if (er_type == RBRACE) {
		er_OUT(s);
		return;
	}
	while (er_type == IFTK || er_type == WHILETK || er_type == FORTK || er_type == DOTK || er_type == LBRACE || er_type == IDENFR || er_type == SCANFTK || er_type == PRINTFTK || er_type == SEMICN || er_type == RETURNTK) {
		isStatement();
	}
	//------------------pass-----------------
	er_OUT(s);
}
void er_gm_analyse::isStatement(void){
	string s("<语句>");
	if (er_type == IFTK) {
		isConditionStatement();
	}
	else if (er_type == WHILETK || er_type == FORTK || er_type == DOTK) {
		isLoopStatement();
	}
	else if (er_type == LBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
		isStatementColumn();
		if (er_type == RBRACE) {
			er_OUT(er_tk.now_token); er_getsym;
		}
	}
	else if (er_type == SCANFTK) {
		isScanfStatement();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_semicn_line, string("k"));
		}
	}
	else if (er_type == PRINTFTK) {
		isPrintfStatement();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_semicn_line, string("k"));
		}
	}
	else if (er_type == RETURNTK) {
		isReturnStatement();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_semicn_line, string("k"));
		}
	}
	else if (er_type == SEMICN) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else if (er_type == IDENFR) {
		er_PUSH; er_getsym;
		if (er_type == LPARENT) {
			er_POP;
			if (er_type == IDENFR) {
				if (symtab.func_lookup(er_token).iden == string("")) {
					er_er.push(er_tk.line, string("c"));
					go(SEMICN);
				}
				else {
					if (symtab.func_lookup(er_token).return_type != VOID) {
						isReturnFunctionCall();
					}
					else if (symtab.func_lookup(er_token).return_type == VOID) {
						isVoidFunctionCall();
					}
				}
			}
			if (er_type == SEMICN) {
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			else {
				er_er.push(er_semicn_line, string("k"));
			}
		}
		else {
			er_POP;
			isAssignStatement();
			if (er_type == SEMICN) {
				er_OUT(er_tk.now_token); er_getsym;
			}//else
			else {
				er_er.push(er_semicn_line, string("k"));
			}
		}
	}//else
	er_OUT(s);
}
void er_gm_analyse::isConditionStatement(void){
	if (er_type == IFTK) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	isCondition();
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	else {
		er_er.push(er_tk.line, string("l"));
	}
	isStatement();
	if (er_type == ELSETK) {
		er_OUT(er_tk.now_token); er_getsym;
		isStatement();
	}//not else
	//----------------------pass---------------------
	string s("<条件语句>");
	er_OUT(s);
}
void er_gm_analyse::isLoopStatement(void){
	if (er_type == WHILETK) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == LPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		isCondition();
		if (er_type == RPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_tk.line, string("l"));
		}
		isStatement();
	}
	else if (er_type == DOTK) {
		er_OUT(er_tk.now_token); er_getsym;
		isStatement();
		if (er_type == WHILETK) {
			er_OUT(er_tk.now_token); er_getsym;
		}
		else {
			er_er.push(er_tk.line, string("n"));
		}
		if (er_type == LPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		isCondition();
		if (er_type == RPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_tk.line, string("l"));
		}
	}
	else if (er_type == FORTK) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == LPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == IDENFR) {
			find_vari();
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == ASSIGN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		isExpression();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_semicn_line, string("k"));
		}
		isCondition();
		if (er_type == SEMICN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_semicn_line, string("k"));
		}
		if (er_type == IDENFR) {
			find_vari();
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == ASSIGN) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == IDENFR) {
			find_vari();
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		if (er_type == PLUS || er_type == MINU) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		isStep();
		if (er_type == RPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}//else
		else {
			er_er.push(er_tk.line, string("l"));
		}
		isStatement();
	}//else

	//---------------pass--------------------
	string s("<循环语句>");
	er_OUT(s);
}
void er_gm_analyse::isCondition(void){
	int res = isExpression();
	if (res != INT) {
		er_er.push(er_tk.line, string("f"));
	}
	if (er_type == LSS || er_type == LEQ || er_type == GRE || er_type == GEQ || er_type == NEQ || er_type == EQL) {
		isRelationshipOperator();
		res = isExpression();
		if (res != INT){
			er_er.push(er_tk.line, string("f"));
		}
	}// not else
	//-----------------pass--------------
	string s("<条件>");
	er_OUT(s);
}
void er_gm_analyse::isAssignStatement(void){
	if (er_type == IDENFR) {
		find_vari();
		if (symtab.var_lookup(er_token).isConst == 1) {
			er_er.push(er_tk.line, string("j"));
		}
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == LBRACK) {
			er_OUT(er_tk.now_token); er_getsym;
			if (isExpression() == CHAR) {
				er_er.push(er_tk.line, string("i"));
			}
			if (er_type == RBRACK) {
				er_OUT(er_tk.now_token); er_getsym;
			}
			else {
				er_er.push(er_tk.line, string("m"));
			}
		}
	}
	if (er_type == ASSIGN) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	isExpression();
	//----------------pass---------------
	string s("<赋值语句>");
	er_OUT(s);
}
void er_gm_analyse::isScanfStatement(void){
	if (er_type == SCANFTK) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == IDENFR) {
		find_vari();
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	while (er_type == COMMA) {
		er_OUT(er_tk.now_token); er_getsym;
		if (er_type == IDENFR) {
			find_vari();
			er_OUT(er_tk.now_token); er_getsym;
		}
	}
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_tk.line, string("l"));
	}
	//------------pass---------------
	string s("<读语句>");
	er_OUT(s);
}
void er_gm_analyse::isPrintfStatement(void){
	if (er_type == PRINTFTK) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == STRCON) {
		er_OUT(er_tk.now_token); er_getsym;
		string ss("<字符串>");
		er_OUT(ss);
		if (er_type == COMMA) {
			er_OUT(er_tk.now_token); er_getsym;
			isExpression();
		}
	}
	else if (er_type == STRERROR) {
		string code("a");
		er_er.push(er_tk.line, code);
		er_getsym;
		if (er_type == COMMA) {
			er_OUT(er_tk.now_token); er_getsym;
			isExpression();
		}
	}
	else{
		isExpression();
	}
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_tk.line, string("l"));
	}
	//------------pass---------------
	string s("<写语句>");
	er_OUT(s);
}
void er_gm_analyse::isReturnStatement(void){
	if (er_type == RETURNTK) {
		
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_has_return = 1;
		if (er_no_return_flag == 1) {
			er_er.push(er_tk.line, string("g"));
		}
		er_OUT(er_tk.now_token); er_getsym;
		int res = isExpression();
		if (er_return_flag == 1 && res != er_return_type_flag) {
			er_er.push(er_tk.line, string("h"));
		}
		if (er_type == RPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}
		else {
			er_er.push(er_tk.line, string("l"));
		}
	}
	else if (er_return_flag == 1 ) {
		er_has_return = 1;
		er_er.push(er_tk.line, string("h"));
	}
	//------------pass---------------
	string s("<返回语句>");
	er_OUT(s);
}
void er_gm_analyse::isPlusOperator(void){
	if (er_type == PLUS || er_type == MINU) {
		er_OUT(er_tk.now_token); er_getsym;
	}
}
void er_gm_analyse::isMultOperator(void){
	if (er_type == MULT || er_type == DIV) {
		er_OUT(er_tk.now_token); er_getsym;
	}
}
void er_gm_analyse::isRelationshipOperator(void){
	if (er_type == LSS || er_type == LEQ || er_type == GRE || er_type == GEQ || er_type == NEQ || er_type == EQL) {
		er_OUT(er_tk.now_token); er_getsym;
	}
}
void er_gm_analyse::isVoidFunction(void){
	string iden;
	
	if (er_type == VOIDTK) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == IDENFR) {
		symtab.func_push(er_token, vector<int>{}, VOID);
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	isVariTable();
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	else {
		er_er.push(er_tk.line, string("l"));
	}
	if (er_type == LBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	er_no_return_flag = 1;//开始检测
	isComStatement();
	er_no_return_flag = 0;//复位
	if (er_type == RBRACE) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	//-----------------------pass-----------------------
	string s("<无返回值函数定义>");
	er_OUT(s);
}
void er_gm_analyse::isReturnFunctionCall(void){
	vector<int> vt;
	if (er_type == IDENFR) {
		if (symtab.func_lookup(er_token).return_type < INT) {
			er_er.push(er_tk.line, string("c"));
			go(RPARENT);
			er_getsym;
			return;
		}
		vt = symtab.func_lookup(er_token).v_table;
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	isValueVariTable();
	compare_vvt(vt);
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_tk.line, string("l"));
	}

	string s("<有返回值函数调用语句>");
	er_OUT(s);
}
void er_gm_analyse::isVoidFunctionCall(void) {
	vector<int> vt;
	if (er_type == IDENFR) {
		if (symtab.func_lookup(er_token).return_type != VOID) {
			er_er.push(er_tk.line, string("c"));
			go(RPARENT);
			er_getsym;
			return;
		}
		vt = symtab.func_lookup(er_token).v_table;
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}//else
	isValueVariTable();
	compare_vvt(vt);
	if (er_type == RPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
	}
	else {
		er_er.push(er_tk.line, string("l"));
	}
	string s("<无返回值函数调用语句>");
	er_OUT(s);
}
int er_gm_analyse::isExpression(void){
	int expr_type = INT;
	if (er_type == CHARCON) {
		er_PUSH;
		er_sp.top() = er_sp.top() - 2;
		er_getsym;
		if (er_type != PLUS && er_type != MINU && er_type != MULT && er_type != DIV)
			expr_type = CHAR;
		er_POP;
	}
	if (er_type == IDENFR) {
		string iden = er_token;
		er_PUSH; er_getsym;
		if (er_type == LBRACK) {
			int cnt = 0;
			do {
				if (er_type == RBRACK) {
					cnt -= 1;
				}
				else if (er_type == LBRACK) {
					cnt += 1;
				}
				er_getsym;
			} while (cnt);
			if (er_type != PLUS && er_type != MINU && er_type != MULT && er_type != DIV) {
				if (symtab.var_lookup(iden)._type == CHAR) {
					expr_type = CHAR;
				}
			}
		}
		else if (er_type == LPARENT) {
			int cnt = 0;
			do {
				if (er_type == RPARENT) {
					cnt -= 1;
				}
				else if (er_type == LPARENT) {
					cnt += 1;
				}
				er_getsym;
			} while (cnt);
			if (er_type != PLUS && er_type != MINU && er_type != MULT && er_type != DIV) {
				if (symtab.func_lookup(iden).return_type == CHAR) {
					expr_type = CHAR;
				}
			}
		}
		else if (er_type != PLUS && er_type != MINU && er_type != MULT && er_type != DIV) {
			if (symtab.var_lookup(iden)._type == CHAR) {
				expr_type = CHAR;
			}
		}
		er_POP;
	}


	if (er_type == PLUS || er_type == MINU) {
		er_OUT(er_tk.now_token); er_getsym;
	}// not else
	isTerm();
	while (er_type == PLUS || er_type == MINU) {
		isPlusOperator();
		isTerm();
	}
	//---------------pass------------------
	string s("<表达式>");
	er_OUT(s);
	return expr_type;
}
void er_gm_analyse::isTerm(void){
	isFactor();
	while (er_type == MULT || er_type == DIV) {
		isMultOperator();
		isFactor();
	}
	//--------------pass------------------------
	string s("<项>");
	er_OUT(s);
}
void er_gm_analyse::isFactor(void) {
	if (er_type == IDENFR) {
		er_PUSH; er_getsym;
		if (er_type == LPARENT) {
			er_POP;
			isReturnFunctionCall();
		}
		else {
			er_POP; 
			find_vari();
			er_OUT(er_tk.now_token); er_getsym;
			if (er_type == LBRACK) {
				er_OUT(er_tk.now_token); er_getsym;
				if (isExpression() == CHAR)
					er_er.push(er_tk.line, string("i"));
				if (er_type == RBRACK) {
					er_OUT(er_tk.now_token); er_getsym;
				}
				else {
					er_er.push(er_tk.line, string("m"));
				}
			}// not else

		}
	}          
	else if (er_type == LPARENT) {
		er_OUT(er_tk.now_token); er_getsym;
		isExpression();
		if (er_type == RPARENT) {
			er_OUT(er_tk.now_token); er_getsym;
		}
		else {
			er_er.push(er_tk.line, string("l"));
		}
	}
	else if (er_type == PLUS || er_type == MINU || er_type == INTCON || er_type == INTERROR) {
		isInteger();
	}
	else if (er_type == CHARCON || er_type == CHARERROR) {
		isChar();
	}//else
	//------------pass----------------
	string s("<因子>");
	er_OUT(s);
}
void er_gm_analyse::isStep(void) {
	isUnsigned();
	//---------pass-----------
	string s("<步长>");
	er_OUT(s);
}
void er_gm_analyse::go(int i) {
	while (i != er_type) {
		er_getsym;
	}
}

void er_gm_analyse::into_func(void) {
	er_isLocal = 1;
}

void er_gm_analyse::out_func(void) {
	er_isLocal = 0;
	symtab.clear();
}

void er_gm_analyse::find_vari() {
	if (!symtab.lookup(er_token)) {
		er_er.push(er_tk.line, string("c"));
	}
}

int er_gm_analyse::compare_vvt(vector<int> v) {
	if (v.size() != er_vvt_now.size()) {
		er_er.push(er_tk.line, string("d"));
		return 1;		//	个数不匹配
	}
	int n = v.size();
	for (int i = 0; i < n; i++) {
		if (v[i] != er_vvt_now[i]) {
			er_er.push(er_tk.line, string("e"));
			return 2;	//类型不匹配
		}
	}
	return 0;
}