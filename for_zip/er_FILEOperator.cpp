#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <stdlib.h>
#include "er_FILEOperator.h"


using namespace std;

static const char* er_input_filename = "testfile.txt";
static const char* er_output_filename = "output.txt";
static const char* er_error_filename = "error.txt";
static ifstream er_ifile;
static ofstream er_ofile;
static ofstream er_efile;

string er_file_string;

void er_open_file() {
	er_ifile.open(er_input_filename);
	er_ofile.open(er_output_filename);
	er_efile.open(er_error_filename);
	if (!er_ifile)
		cout << "!!!open input file failed!!!" << endl;
	if (!er_ofile)
		cout << "!!!open output file failed!!!" << endl;
	if (!er_efile)
		cout << "!!!open error file failed!!!" << endl;
}

void er_read_from_file() {
	ostringstream buf;
	char ch;
	while (buf && er_ifile.get(ch))
		buf.put(ch);
	er_file_string = buf.str();
	//return buf.str();
}

void er_write_into_file(er_token_info& er_tk) {
	cout << er_tk.out_type_string() << " " << er_tk.token << endl;
	er_ofile << er_tk.out_type_string() << " " << er_tk.token << endl;
}

void er_write_into_file(string s) {
	cout << s << endl;
	er_ofile << s << endl;
}

void er_write_into_file(er_error_info& er) {
	cout << er.line << " " << er.code << endl;
	er_efile << er.line << " " << er.code << endl;
}

void er_close_file() {
	if (er_ifile)
		er_ifile.close();
	if (er_ofile)
		er_ofile.close();
	if (er_efile)
		er_efile.close();
}