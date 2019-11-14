#include "FILEOperator.h"
#include "TkAnalysis.h"
#include "GMAnalysis.h"
#include "Error.h"
#include "SymbolTable.h"
#include "MemoryManage.h"
#include <iostream>
using namespace std;
extern string file_string;
extern vector<MemoryTableItem> memory_table;
void symtab_test() {
	Symtab symtab = Symtab();
	symtab.func_push(string("fun1"), vector<int> {1}, 1);
	symtab.func_push(string("fun2"), vector<int>{INT, CHAR, INT}, VOID);
	symtab.func_push(string("fun3"), vector<int>{INT, CHAR}, CHAR);
	symtab.func_push(string("fun4"), vector<int>{INT, CHAR, INT}, INT);
	symtab.func_push(string("fun5"), vector<int>{INT, CHAR, INT, CHAR}, VOID);
	cout << symtab.local_lookup(string("a")) << endl;
	cout << symtab.local_lookup(string("a1")) << endl;
	cout << symtab.global_lookup(string("b")) << endl;
	cout << symtab.local_lookup(string("fun1")) << endl;
	cout << symtab.global_lookup(string("g")) << endl;
	cout << symtab.global_lookup(string("g1")) << endl;
	cout << symtab.local_lookup(string("fun4")) << endl;
	cout << symtab.local_lookup(string("f")) << endl;


}
int main() {
	open_file();
	read_from_file();
	/*tk_analyse tk = tk_analyse();
	while (tk.now_token.type != EMPTYS) {
		tk.output();
		tk.get_token();
	}*/

	gm_analyse gm = gm_analyse();
	gm.isProgram();
	gm.er.out();
	close_file();
	gm.mc_gen.out();
	//symtab_test();
	return 0;
}