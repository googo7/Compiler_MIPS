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
#include "er_Error.h"
#include "er_FILEOperator.h"
typedef unordered_map<string, string> string_string_map;
/*string_string_map error_type_info = {
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
};*/

er_error_info::er_error_info() {
	this->line = 0;
	this->code = "";
}

er_error_info::er_error_info(string c, unsigned l) {
	this->code = c;
	this->line = l;
}


er_error_analyse::er_error_analyse() {
	;
}

void er_error_analyse::push(unsigned line, string code) {
	er_error_info info(code, line);
	this->error_set.push_back(info);
}

void er_error_analyse::out() {
	int len = this->error_set.size();
	for (int i = 0; i < len; i++) {
		//if (error_set[i].code == "g")
			//continue;
		er_write_into_file(error_set[i]);
	}
}





