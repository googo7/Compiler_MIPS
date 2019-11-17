#pragma once
#include <iostream>
#include <string>
#include <stack>
#include "TkAnalysis.h"
#include "Error.h"
#include "SymbolTable.h"
#include "MidCode.h"
#include <set>
using namespace std;

class gm_analyse {
public:
	gm_analyse();
	stack<unsigned> sp;
	tk_analyse tk;
	error_analyse er;
	Symtab symtab;
	MidCodeGen mc_gen;
	void isProgram(void);
	void isMainFunction(void);
	void isConstDeclaration(void);
	void isConstDefinition(void);
	void isVariDeclaration(void);
	void isVariDefinition(void);
	void isReturnFunction(void);
	void isDeclarationHeader(void);
	int isInteger(void);
	int isUnsigned(void);
	char isChar(void);
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

	void isExpression(void);
	void isTerm(void);
	void isFactor(void);
	int isStep(void);


	void into_func(void);
	void out_func(void);
	void begin_midcode(void);
	void end_midcode(string);
	token_info replace(token_info);
};