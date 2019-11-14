#include "MemoryManage.h"


MemoryTableItem::MemoryTableItem() {
	
}

MemoryTableItem::MemoryTableItem(string i, string f, int il, int a) {
	this->iden = i;
	this->func = f;
	this->isLocal = il;
	this->arr = a;
}