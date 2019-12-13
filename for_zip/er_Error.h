#pragma once
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
	func
	global
	local
*/
#include <string>
#include <vector>
using namespace std;

class er_error_info{
public:
	string code;
	unsigned line;
	er_error_info();
	er_error_info(string, unsigned);
};

class er_error_analyse {
public:
	vector<er_error_info> error_set;
	er_error_analyse();
	void push(unsigned, string);
	void out();
};
