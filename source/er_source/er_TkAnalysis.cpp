#include <iostream>
#include <string>
#include <unordered_map>
#include "er_TkAnalysis.h"
#include "er_FILEOperator.h"
extern string er_file_string;

const static vector<string> er_tk_type_string = {
	"EMPTYS",
	"IDENFR", "INTCON", "CHARCON", "STRCON", "CONSTTK",
	"INTTK", "CHARTK", "VOIDTK", "MAINTK", "IFTK",
	"ELSETK", "DOTK", "WHILETK", "FORTK", "SCANFTK",
	"PRINTFTK", "RETURNTK", "PLUS", "MINU", "MULT",
	"DIV", "LSS", "LEQ", "GRE", "GEQ",
	"EQL", "NEQ", "ASSIGN", "SEMICN", "COMMA",
	"LPARENT", "RPARENT", "LBRACK", "RBRACK", "LBRACE",
	"RBRACE"
};	//单词token的所有类型的对应字符串，注意与enum结构一一对应。

typedef unordered_map<string, int> er_string_int_map;
er_string_int_map er_reserved_word = {
	{"const", CONSTTK}, {"int", INTTK}, {"char", CHARTK}, {"void", VOIDTK}, {"main",MAINTK},
	{"if",IFTK}, {"else",ELSETK}, {"do",DOTK}, {"while", WHILETK}, {"for",FORTK},
	{"scanf",SCANFTK},{"printf",PRINTFTK}, {"return", RETURNTK}
};			//保留字的对应map(string->ENUM)

er_token_info::er_token_info() {
	;
}//默认构造


//常用的构造函数
er_token_info::er_token_info(int type, string& token) {
	this->type = type;
	this->token = token;
}


//从type(int型变量)转换到字符串的tostring
string er_token_info::out_type_string(){
	return er_tk_type_string[this->type];
}


//词法分析器的默认构造完成初始化过程：a)指针归0    b)当前字符置'\0'   c)读取第一个单词，存入单词寄存器
er_tk_analyse::er_tk_analyse() {
	this->ch_pt = 0;
	this->line = 1;
	this->ch = '\0';
	get_token();
}

//词法分析的核心，读取并分析字符，将分析出的单词存入寄存器now_token
void er_tk_analyse::get_token() {
	get_char();
	while (isspace(ch)) {
		if (ch == '\n') {
			//cout << ch;
			line++;
			//cout << line << "  " << this->now_token.token << endl;
		}
		get_char(); 
	}
	if (is_alpha()) {
		string tk_temp;
		while (is_alnum()) {
			tk_temp.push_back(ch); get_char();
		}
		unget_char();
		if (er_reserved_word.find(tk_temp) == er_reserved_word.end()) { //IDENFR
			now_token = er_token_info(IDENFR, tk_temp);
		}
		else {
			now_token = er_token_info(er_reserved_word[tk_temp], tk_temp);
		}
	}
	else if(is_digit()){
		string tk_temp;
		if (ch == '0') {
			get_char();
			if (is_digit()) {
				//error
				while (is_digit()) {
					get_char();
				}
				unget_char();
				tk_temp = "";
				now_token = er_token_info(INTERROR, tk_temp);
				return;
			}
			else {
				unget_char();
				tk_temp.push_back(ch);
			}
		}
		else {
			while (is_digit()) {
				tk_temp.push_back(ch); get_char();
			}
			unget_char();
		}
		now_token = er_token_info(INTCON, tk_temp);
	}
	else {
		string tk_temp = "";
		switch (ch)
		{
		case '\'': {
			get_char();
			if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || is_alnum()) {
				tk_temp.push_back(ch);
				get_char();
				if (ch == '\'')
					now_token = er_token_info(CHARCON, tk_temp);
				else {/*error*/ 
					while (ch != '\n') {
						get_char();
					}
					unget_char();
					now_token = er_token_info(CHARERROR, tk_temp);
				}
			}
			else {
				get_char();
				if (ch == '\'') {
					now_token = er_token_info(CHARERROR, tk_temp);
				}
				else {
					while (ch != '\n') {
						get_char();
					}
					unget_char();
					now_token = er_token_info(CHARERROR, tk_temp);
				}
			}
			break;
		}
		case '"': {
			get_char();
			if (ch == '"') {
				tk_temp = "";
				now_token = er_token_info(STRCON, tk_temp);
				break;
			}
			while (ch == 32 || ch == 33 || (ch >= 35 && ch <= 126)) {
				tk_temp.push_back(ch);
				get_char();
			}
			if (ch == '"')
				now_token = er_token_info(STRCON, tk_temp);
			else {
				while (ch != '\n') {
					get_char();
				}
				unget_char();
				now_token = er_token_info(STRERROR, tk_temp);
			}
			break;
		}
		case '+':{
			tk_temp = "+";
			now_token = er_token_info(PLUS, tk_temp);
			break;
		}
		case '-' : {
			tk_temp = "-";
			now_token = er_token_info(MINU, tk_temp);
			break;
		}
		case '*': {
			tk_temp = "*";
			now_token = er_token_info(MULT, tk_temp);
			break;
		}
		case '/': {
			tk_temp = "/";
			now_token = er_token_info(DIV, tk_temp);
			break;
		}
		case '<': {
			get_char();
			if (ch == '=') {
				tk_temp = "<=";
				now_token = er_token_info(LEQ, tk_temp);
				break;
			}
			unget_char();
			tk_temp = "<";
			now_token = er_token_info(LSS, tk_temp);
			break;
		}
		case '>': {
			get_char();
			if (ch == '=') {
				tk_temp = ">=";
				now_token = er_token_info(GEQ, tk_temp);
				break;
			}
			unget_char();
			tk_temp = ">";
			now_token = er_token_info(GRE, tk_temp);
			break;
		}
		case '!': {
			get_char();
			if (ch == '=') {
				tk_temp = "!=";
				now_token = er_token_info(NEQ, tk_temp);
				break;
			}
			else{/*error*/ }
			break;
		}
		case '=': {
			get_char();
			if (ch == '=') {
				tk_temp = "==";
				now_token = er_token_info(EQL, tk_temp);
				break;
			}
			unget_char();
			tk_temp = "=";
			now_token = er_token_info(ASSIGN, tk_temp);
			break;
		}
		case ';': {
			tk_temp = ";";
			now_token = er_token_info(SEMICN, tk_temp);
			break;
		}
		case ',': {
			tk_temp = ",";
			now_token = er_token_info(COMMA, tk_temp);
			break;
		}
		case '(': {
			tk_temp = "(";
			now_token = er_token_info(LPARENT, tk_temp);
			break;
		}
		case ')': {
			tk_temp = ")";
			now_token = er_token_info(RPARENT, tk_temp);
			break;
		}
		case '[': {
			tk_temp = "[";
			now_token = er_token_info(LBRACK, tk_temp);
			break;
		}
		case ']': {
			tk_temp = "]";
			now_token = er_token_info(RBRACK, tk_temp);
			break;
		}
		case '{': {
			tk_temp = "{";
			now_token = er_token_info(LBRACE, tk_temp);
			break;
		}
		case '}': {
			tk_temp = "}";
			now_token = er_token_info(RBRACE, tk_temp);
			break;
		}
		case '\0': {
			tk_temp = "";
			now_token = er_token_info(EMPTYS, tk_temp);
			break;
		}
		default:
			break;
		}
	}
}


//读入下一个字符，指针永远指向当前字符的下一个字符处
//tips：做出判别，若指针越界，则将ch置'\0'
void er_tk_analyse::get_char() {
	if (ch_pt >= er_file_string.size()) {
		this->ch = '\0';
	}
	this->ch = er_file_string[this->ch_pt++];
}


//返回上一个字符并还原指针
void er_tk_analyse::unget_char() {
	this->ch_pt--;
	
	this->ch = er_file_string[this->ch_pt - 1];
	if (this->ch == '\n')
		this->line--;
}


//以下为对应的判断ch字符类型语句，其中字母集加入'_'
int er_tk_analyse::is_alpha() {
	return (isalpha(this->ch) || this->ch == '_');
}

int er_tk_analyse::is_space() {
	return isspace(this->ch);
	
}

int er_tk_analyse::is_digit() {
	return isdigit(this->ch);
}

int er_tk_analyse::is_alnum() {
	return (isalnum(this->ch) || this->ch == '_');
}

void er_tk_analyse::output() {
	er_write_into_file(this->now_token);
}

