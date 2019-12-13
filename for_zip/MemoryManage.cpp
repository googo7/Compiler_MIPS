#include "MemoryManage.h"
#include <algorithm>



MemoryTable::MemoryTable() {
	this->map = {};
}

void MemoryTable::push(string func_name, var_info vi) {
	if (map.find(func_name) == map.end()) {
		this->map[func_name] = vector<var_info>{ vi };
	}
	else{
		for (int i = 0; i < this->map[func_name].size(); i++) {
			if (this->map[func_name][i].iden == vi.iden)
				return;
		}
		this->map[func_name].push_back(vi);
	}
}

void MemoryTable::push(string s, int type) {
	this->temp[s] = type;
}

void MemoryTable::push(string func_name, pair<string, int> temp_name) {
	if (map.find(func_name) == map.end()) {
		map[func_name] = vector<var_info>{ var_info(temp_name.first, 0, temp_name.second, -1, 0) };
		return;
	}
	int addr = map[func_name][map[func_name].size() - 1].addr + map[func_name][map[func_name].size() - 1].size;
	var_info v(temp_name.first, 0, temp_name.second, -1, addr);
	map[func_name].push_back(v);
}

var_info MemoryTable::lookup(string func_name, string iden) {
	var_info res;
	if (map.find(func_name) == map.end())
		return res;
	vector<var_info> vi = map[func_name];
	for (int i = 0; i < vi.size(); i++) {
		if (vi[i].iden == iden)
			res = vi[i];
	}
	return res;
}

void MemoryTable::setflag(string func_name, string iden, int num) {
	var_info res;
	if (map.find(func_name) == map.end())
		return;
	vector<var_info> vi = map[func_name];
	for (int i = 0; i < vi.size(); i++) {
		if (vi[i].iden == iden)
			map[func_name][i].value = num;
	}
}

int MemoryTable::top_addr(string func_name) {
	int res = 0;
	if (map.find(func_name) == map.end())
		return res;
	vector<var_info> vi = map[func_name];
	for (int i = 0; i < vi.size(); i++) {
			res = vi[i].addr + vi[i].size;
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


