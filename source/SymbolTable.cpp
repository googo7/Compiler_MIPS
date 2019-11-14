#include "SymbolTable.h"

func_info::func_info() { ; }
func_info::func_info(string s, vector<int> si, int i) {
	this->iden = s;
	this->v_table = si;
	this->return_type = i;
}

var_info::var_info() {  }
var_info::var_info(string s, int i, int t, int v, int a) {
	this->iden = s;
	this->isConst = i;
	this->_type = t;
	this->value = v;
	this->addr = a;
}

bool Symtab::func_push(string s, vector<int> si, int i) {
	if (lookup(s))
		return false;
	func_info fi(s, si, i);
	this->func_table.push_back(fi);
	return true;
}

bool Symtab::global_push(string s, int i, int t, int v, int a) {
	if (lookup(s))
		return false;
	var_info vi(s, i, t, v, a);
	this->global_var_table.push_back(vi);
	return true;
}

bool Symtab::local_push(string s, int i, int t, int v, int a) {
	if (local_lookup(s))
		return false;
	var_info vi(s, i, t, v, a);
	this->local_var_table.push_back(vi);
	return true;
}

bool Symtab::global_lookup(string iden) {
	for (int i = 0; i < this->func_table.size(); i++) {
		func_info fi = this->func_table[i];
		if (fi.iden == iden)
			return true;
	}
	for (int i = 0; i < this->global_var_table.size(); i++) {
		var_info vi = this->global_var_table[i];
		if (vi.iden == iden)
			return true;
	}
	return false;
}

func_info Symtab::func_lookup(string iden) {
	func_info a(string(""), vector<int>{}, -1);
	for (int i = 0; i < this->func_table.size(); i++) {
		func_info fi = this->func_table[i];
		if (fi.iden == iden)
			return fi;
	}
	return a;
}

var_info Symtab::var_lookup(string iden) {
	var_info a(string(""), -1, -1, -1, -1);
	for (int i = 0; i < this->local_var_table.size(); i++) {
		var_info vi = this->local_var_table[i];
		if (vi.iden == iden)
			return vi;
	}
	for (int i = 0; i < this->global_var_table.size(); i++) {
		var_info vi = this->global_var_table[i];
		if (vi.iden == iden)
			return vi;
	}
	return a;
}

bool Symtab::local_lookup(string iden) {
	for (int i = 0; i < this->local_var_table.size(); i++) {
		var_info vi = this->local_var_table[i];
		if (vi.iden == iden)
			return true;
	}
	return false;
}

bool Symtab::lookup(string iden) {
	bool res = local_lookup(iden);
	if (res)
		return res;
	else
		return global_lookup(iden);
}

void Symtab::clear() {
	this->local_var_table.clear();
}

void Symtab::adjust_size(string s, int value) {
	for (int i = 0; i < this->local_var_table.size(); i++) {
		var_info vi = this->local_var_table[i];
		if (vi.iden == s) {
			vi.size = value;
			return;
		}
	}
	for (int i = 0; i < this->global_var_table.size(); i++) {
		var_info vi = this->global_var_table[i];
		if (vi.iden == s) {
			vi.size = value;
			return;
		}
	}
}