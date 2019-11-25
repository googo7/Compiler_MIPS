#pragma once

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;


enum Type {
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


class token_info {
public:
	int type = 0;
	string token = "";
	token_info();
	token_info(int, string);
	string out_type_string();
};


class tk_analyse {
public:
	tk_analyse();
	token_info now_token;
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