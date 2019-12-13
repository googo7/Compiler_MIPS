#pragma once

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;


enum er_Type {
	EMPTYS,
	IDENFR, INTCON, CHARCON, STRCON, CONSTTK,
	INTTK, CHARTK, VOIDTK, MAINTK, IFTK,
	ELSETK, DOTK, WHILETK, FORTK, SCANFTK,
	PRINTFTK, RETURNTK, PLUS, MINU, MULT,
	DIV, LSS, LEQ, GRE, GEQ,
	EQL, NEQ, ASSIGN, SEMICN, COMMA,
	LPARENT, RPARENT, LBRACK, RBRACK, LBRACE,
	RBRACE, INTERROR, CHARERROR, STRERROR
};


class er_token_info {
public:
	int type = 0;
	string token = "";
	er_token_info();
	er_token_info(int, string&);
	string out_type_string();
};


class er_tk_analyse {
public:
	er_tk_analyse();
	er_token_info now_token;
	unsigned ch_pt;
	unsigned line;
	char ch;
	void get_token(void);
	void get_char(void);
	void unget_char(void);
	int is_alpha(void);
	int is_space(void);
	int is_digit(void);
	int is_alnum(void);
	void output();
};