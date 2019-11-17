/*
	------------------------------------------------------------
	a	|	非法符号或不符合词法
	b	|	名字重定义
	c	|	未定义的名字
	d	|	函数参数个数不匹配
	e	|	函数参数类型不匹配
	f	|	条件判断中出现不合法的类型
	g	|	无返回值的函数存在不匹配的return语句
	h	|	有返回值的函数缺少return语句或存在不匹配的return语句
	i	|	数组元素的下标只能是整形表达式
	j	|	不能改变常量的值
	k	|	应为分号
	l	|	应为右小括号‘)’
	m	|	应为右中括号‘]’
	n	|	do-while语句中缺少while
	o	|	常量定义中=后面只能是整型或字符型常量
	------------------------------------------------------------
*/

#include <unordered_map>
#include <string>
#include "Error.h"
#include "FILEOperator.h"
typedef unordered_map<string, string> string_string_map;
string_string_map error_type_info = {
	{"a", "非法符号或不符合词法"},
	{"b", "名字重定义"},
	{"c", "未定义的名字"},
	{"d", "函数参数个数不匹配" },
	{"e", "函数参数类型不匹配"},
	{"f", "条件判断中出现不合法的类型"},
	{"g", "无返回值的函数存在不匹配的return语句"},
	{"h", "有返回值的函数缺少return语句或存在不匹配的return语句"},
	{"i", "数组元素的下标只能是整形表达式"},
	{"j", "不能改变常量的值"},
	{"k" , "应为分号"},
	{"l" , "应为右小括号‘)’"},
	{"m" , "应为右中括号‘]’"},
	{"n" , "do - while语句中缺少while"},
	{"o" , "常量定义中 = 后面只能是整型或字符型常量"}
};

error_info::error_info() {
	this->line = 0;
	this->code = "";
}

error_info::error_info(string c, unsigned l) {
	this->code = c;
	this->line = l;
}


error_analyse::error_analyse() {
	;
}

void error_analyse::push(unsigned line, string code) {
	error_info info(code, line);
	this->error_set.push_back(info);
}

void error_analyse::out() {
	int len = this->error_set.size();
	for (int i = 0; i < len; i++) {
		write_into_file(error_set[i]);
	}
}





