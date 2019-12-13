#include "er_FILEOperator.h"
#include "er_TkAnalysis.h"
#include "er_GMAnalysis.h"
#include "er_Error.h"
#include "er_SymbolTable.h"
#include <iostream>

using namespace std;
extern string er_file_string;

int er_main() {
	er_open_file();
	er_read_from_file();
	/*tk_analyse tk = tk_analyse();
	while (tk.now_token.type != EMPTYS) {
		tk.output();
		tk.get_token();
	}*/

	er_gm_analyse gm = er_gm_analyse();
	gm.isProgram();
	gm.er_er.out();
	er_close_file();
	//symtab_test();
	return 0;
}

