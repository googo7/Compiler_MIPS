#include "MipsGen.h"
#include "MemoryManage.h"
/*
	func
	para
	func_push_para
	func_call
	add
	sub
	mult
	div
	BZ
	BNZ
	GOTO
	LABEL
	=
	!=
	==
	<
	<=
	>=
	>
	scanf
	printf
	ret
*/
void MipsGen::parse(MidCode mc) {
	string op = mc.op;

	if (op == "add") {
		string s1 = mc.s1;
		string s2 = mc.s2;
		string result = mc.result;

	}
}