#include "er_SymbolTable.h"

er_func_info::er_func_info() { ; }
er_func_info::er_func_info(string s, vector<int> si, int i) {
	this->iden = s;
	this->v_table = si;
	this->return_type = i;
}

er_var_info::er_var_info() { ; }
er_var_info::er_var_info(string s, int i, int t, int v) {
	this->iden = s;
	this->isConst = i;
	this->_type = t;
	this->value = v;
}

bool er_Symtab::func_push(string s, vector<int> si, int i) {
	if (lookup(s))
		return false;
	er_func_info fi(s, si, i);
	this->func_table.push_back(fi);
	return true;
}

bool er_Symtab::global_push(string s, int i, int t, int v) {
	if (lookup(s))
		return false;
	er_var_info vi(s, i, t, v);
	this->global_var_table.push_back(vi);
	return true;
}

bool er_Symtab::local_push(string s, int i, int t, int v) {
	if (local_lookup(s))
		return false;
	er_var_info vi(s, i, t, v);
	this->local_var_table.push_back(vi);
	return true;
}

bool er_Symtab::global_lookup(string iden) {
	for (int i = 0; i < this->func_table.size(); i++) {
		er_func_info fi = this->func_table[i];
		if (fi.iden == iden)
			return true;
	}
	for (int i = 0; i < this->global_var_table.size(); i++){
		er_var_info vi = this->global_var_table[i];
		if (vi.iden == iden)
			return true;
	}
	return false;
}

er_func_info er_Symtab::func_lookup(string iden) {
	er_func_info a(string(""), vector<int>{}, -1);
	for (int i = 0; i < this->func_table.size(); i++) {
		er_func_info fi = this->func_table[i];
		if (fi.iden == iden)
			return fi;
	}
	return a;
}

er_var_info er_Symtab::var_lookup(string iden) {
	er_var_info a(string(""), -1, -1, -1);
	for (int i = 0; i < this->local_var_table.size(); i++) {
		er_var_info vi = this->local_var_table[i];
		if (vi.iden == iden)
			return vi;
	}
	for (int i = 0; i < this->global_var_table.size(); i++) {
		er_var_info vi = this->global_var_table[i];
		if (vi.iden == iden)
			return vi;
	}
	return a;
}

bool er_Symtab::local_lookup(string iden) {
	for (int i = 0; i < this->local_var_table.size(); i++) {
		er_var_info vi = this->local_var_table[i];
		if (vi.iden == iden)
			return true;
	}
	return false;
}

bool er_Symtab::lookup(string iden) {
	bool res = local_lookup(iden);
	if (res)
		return res;
	else
		return global_lookup(iden);
}

void er_Symtab::clear() {
	this->local_var_table.clear();
}