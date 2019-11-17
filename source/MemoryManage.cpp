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

int MemoryTable::lookup_addr(string func_name, string iden) {
	var_info v = lookup(func_name, iden);
	return v.addr << 2;
}

RegTableItem::RegTableItem(){}

RegTableItem::RegTableItem(string id, int iH, string r, int iL) {
	this->iden = id;
	this->isHit = iH;
	this->reg = r;
	this->isLocal = iL;
}



RegTable::RegTable() {
	this->reg_table = {};
}



string RegTable::lookup(string id, int iL, string type) {
	vector<RegTableItem> reg_table_now = this->reg_table;
	for (int i = 0; i < reg_table_now.size(); i++) {
		RegTableItem reg_item = reg_table_now[i];
		if (reg_item.iden == id && reg_item.isLocal == iL) {
			if (reg_item.isHit) {
				//LRU
				return reg_item.reg;
			}
		}
	}
	if (alloc(id, type)) {
		if (type == "s")
			return use_s_reg[use_s_reg.size() - 1];
		else if(type == "t")
			return use_t_reg[use_t_reg.size() - 1];
	}

	return "";
}

int RegTable::alloc(string id, string type) {
	if (type == "s") {
		if (!free_s_reg.size()) {
			//lw -> xxx -> sw
			return 0;
		}
		else {
			string res = free_s_reg[0];
			free_s_reg.erase(free_s_reg.begin(), free_s_reg.begin() + 1);
			use_s_reg.push_back(res);
			this->reg_table.push_back(RegTableItem(id, 1, res, 1));
		}
	}
	else if (type == "t") {
		if (!free_t_reg.size()) {
			//lw -> xxx -> sw
			return 0;
		}
		else {
			string res = free_t_reg[0];
			free_t_reg.erase(free_t_reg.begin(), free_t_reg.begin() + 1);
			use_t_reg.push_back(res);
			this->reg_table.push_back(RegTableItem(id, 1, res, 1));
		}
	}
	return 1;
}

void RegTable::clear_t() {
	this->use_t_reg = {};
	this->free_t_reg = { "t0", "t1", "t2", "t3", "t4", "t5", "t6","t7" };
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[0] == 't') {
			this->reg_table.erase(reg_table.begin() + i, reg_table.begin() + i + 1);
			i--;
		}
	}
}

void RegTable::clear_s() {
	this->use_s_reg = {};
	this->free_s_reg = { "s0", "s1", "s2", "s3", "s4", "s5", "s6","s7" };
	for (int i = 0; i < this->reg_table.size(); i++) {
		if (this->reg_table[i].reg[0] == 's') {
			this->reg_table.erase(reg_table.begin() + i, reg_table.begin() + i + 1);
			i--;
		}
	}
}