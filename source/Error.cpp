/*
	------------------------------------------------------------
	a	|	�Ƿ����Ż򲻷��ϴʷ�
	b	|	�����ض���
	c	|	δ���������
	d	|	��������������ƥ��
	e	|	�����������Ͳ�ƥ��
	f	|	�����ж��г��ֲ��Ϸ�������
	g	|	�޷���ֵ�ĺ������ڲ�ƥ���return���
	h	|	�з���ֵ�ĺ���ȱ��return������ڲ�ƥ���return���
	i	|	����Ԫ�ص��±�ֻ�������α��ʽ
	j	|	���ܸı䳣����ֵ
	k	|	ӦΪ�ֺ�
	l	|	ӦΪ��С���š�)��
	m	|	ӦΪ�������š�]��
	n	|	do-while�����ȱ��while
	o	|	����������=����ֻ�������ͻ��ַ��ͳ���
	------------------------------------------------------------
*/

#include <unordered_map>
#include <string>
#include "Error.h"
#include "FILEOperator.h"
typedef unordered_map<string, string> string_string_map;
string_string_map error_type_info = {
	{"a", "�Ƿ����Ż򲻷��ϴʷ�"},
	{"b", "�����ض���"},
	{"c", "δ���������"},
	{"d", "��������������ƥ��" },
	{"e", "�����������Ͳ�ƥ��"},
	{"f", "�����ж��г��ֲ��Ϸ�������"},
	{"g", "�޷���ֵ�ĺ������ڲ�ƥ���return���"},
	{"h", "�з���ֵ�ĺ���ȱ��return������ڲ�ƥ���return���"},
	{"i", "����Ԫ�ص��±�ֻ�������α��ʽ"},
	{"j", "���ܸı䳣����ֵ"},
	{"k" , "ӦΪ�ֺ�"},
	{"l" , "ӦΪ��С���š�)��"},
	{"m" , "ӦΪ�������š�]��"},
	{"n" , "do - while�����ȱ��while"},
	{"o" , "���������� = ����ֻ�������ͻ��ַ��ͳ���"}
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





