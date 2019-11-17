#include "MemoryManage.h"




MemoryTable::MemoryTable() {
	this->map = {};
}

void MemoryTable::push(string func_name, var_info vi) {
	if (map.find(func_name) == map.end()) {
		this->map[func_name] = vector<var_info>{ vi };
	}
	else {
		this->map[func_name].push_back(vi);
	}
}

void MemoryTable::push(string s) {
	this->temp.push_back(s);
}

void MemoryTable::push(string func_name, string temp_name) {
	if (map.find(func_name) == map.end()) {
		map[func_name] = vector<var_info>{ var_info(temp_name, 0, TEMP, -1, 0) };
		return;
	}
	int addr = map[func_name][map[func_name].size() - 1].addr;
	var_info v(temp_name, 0, TEMP, -1, addr + 1);
	map[func_name].push_back(v);
}

var_info MemoryTable::lookup(string func_name, string iden) {
	var_info res;
	vector<var_info> vi = map[func_name];
	for (int i = 0; i < vi.size(); i++) {
		if (vi[i].iden == iden)
			res = vi[i];
	}
	return res;
}

int MemoryTable::top_addr(string func_name) {
	int res = 0;
	vector<var_info> vi = map[func_name];
	for (int i = 0; i < vi.size(); i++) {
			res = vi[i].addr;
	}
	return res;
}

int MemoryTable::lookup_addr(string func_name, string iden) {
	var_info v = lookup(func_name, iden);
	return v.addr << 2;
}

RegTableItem::RegTableItem(){}

RegTableItem::RegTableItem(string id, int iH, var_info v) {
	this->reg = id;
	this->isHit = iH;
	this->var = v;
}


