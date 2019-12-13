#pragma once
#include <iostream>
#include <string>
#include <stack>
#include "er_TkAnalysis.h"
#include "er_Error.h"
#include "er_SymbolTable.h"
#include <set>
using namespace std;

class er_gm_analyse {
public:
	er_gm_analyse();
	stack<unsigned> er_sp;
	er_tk_analyse er_tk;
	er_error_analyse er_er;
	er_Symtab symtab;
	void isProgram(void);
	void isMainFunction(void);
	void isConstDeclaration(void);
	void isConstDefinition(void);
	void isVariDeclaration(void);
	void isVariDefinition(void);
	void isReturnFunction(void);
	void isDeclarationHeader(void);
	bool isInteger(void);
	bool isUnsigned(void);
	void isChar(void);
	void isVariTable(void);
	void isValueVariTable(void);
	int isTypeIden(void);
	void isComStatement(void);
	void isStatementColumn(void);
	void isStatement(void);
	void isConditionStatement(void);
	void isLoopStatement(void);
	void isCondition(void);
	void isAssignStatement(void);
	void isScanfStatement(void);
	void isPrintfStatement(void);
	void isReturnStatement(void);
	void isPlusOperator(void);
	void isMultOperator(void);
	void isRelationshipOperator(void);
	void isVoidFunction(void);
	void isReturnFunctionCall(void);
	void isVoidFunctionCall(void);

	int isExpression(void);
	void isTerm(void);
	void isFactor(void);
	void isStep(void);



	void go(int);

	void into_func(void);
	void out_func(void);

	void find_vari();
	int compare_vvt(vector<int>);
};