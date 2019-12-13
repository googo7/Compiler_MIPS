#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <stdlib.h>
#include "FILEOperator.h"


using namespace std;

static const char* input_filename = "testfile.txt";
static const char* output_filename = "output.txt";
static const char* error_filename = "error.txt";
static const char* midcode_filename = "midcode.txt";
static const char* mips_filename = "mips.txt";
static ifstream ifile;
static ofstream ofile;
static ofstream efile;
static ofstream midfile;
static ofstream mipsfile;
string file_string;

void open_file() {
	ifile.open(input_filename);
	ofile.open(output_filename);
	efile.open(error_filename);
	midfile.open(midcode_filename);
	mipsfile.open(mips_filename);
	if (!ifile)
		cout << "!!!open input file failed!!!" << endl;
	if (!ofile)
		cout << "!!!open output file failed!!!" << endl;
	if (!efile)
		cout << "!!!open error file failed!!!" << endl;
	if (!midfile)
		cout << "!!!open midcode file failed!!!" << endl;
	if (!mipsfile)
		cout << "!!!open mips file failed!!!" << endl;
}

void read_from_file() {
	ostringstream buf;
	char ch;
	while (buf && ifile.get(ch))
		buf.put(ch);
	file_string = buf.str();
	//return buf.str();
}

void write_into_file(token_info tk) {
	//cout << tk.out_type_string() << " " << tk.token << endl;
	ofile << tk.out_type_string() << " " << tk.token << endl;
}

void write_into_file(string s) {
	//cout << s << endl;
	ofile << s << endl;
}

void write_into_file(error_info er) {
	//cout << er.line << " " << er.code << endl;
	efile << er.line << " " << er.code << endl;
}

void write_into_file(MidCode mc) {
	midfile << mc.out() << endl;
}

void write_into_mfile(string s) {
	mipsfile << s << endl;
}

void close_file() {
	if (ifile)
		ifile.close();
	if (ofile)
		ofile.close();
	if (efile)
		efile.close();
	if (midfile)
		midfile.close();
	if (mipsfile)
		mipsfile.close();
}