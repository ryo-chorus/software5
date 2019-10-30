/*
 * prettyprinter.c
 *
 *  Created on: 2017/10/30
 *      Author: b5122013
 */

#include "compiler.h"


int arraynum = 0;
int typenum = 0;
int paraflag = 0;
int gorl = 0; /* global or local */
int arraytype;
int shikitype, shikiarraysize, shikiarraytype;
int simpletype, simplearraysize, simplearraytype;
int vartype, vararraysize, vararraytype;
int koutype, kouarraysize, kouarraytype;
int inshitype, inshiarraysize, inshiarraytype;
int arrayflag = 0;
int paranum;
int expnum;
struct NAME *names;
struct TYPE *paratype;
struct ID *searchp;
char *token_str[NUMOFTOKEN+1] = {
		"", "",
		"program", "var", "array", "of", "begin", "end", "if", "then",
		"else", "procedure", "return", "call", "while", "do", "not", "or",
		"div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
		"false", "", "", "+", "-", "*", "=", "<>", "<", "<=", ">",
		">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read","write", "break"
};
char *procname;
int parse_program(){   /* program */
	if(token != TPROGRAM){
		error("Keyword 'program' is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TNAME){
		error("Program name is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(block() == ERROR) return ERROR;
	if(token != TDOT){
		error("Period is not found at the end of program!");
		return ERROR;
	}
	token = scan();
	return NORMAL;
}

int block(){      /* block */
	while(token == TVAR || token == TPROCEDURE){
		switch(token){
		case TVAR:
			if(var_decl() == ERROR){
				return ERROR;
			}
			break;
		case TPROCEDURE:
			if(sub_decl() == ERROR){
				return ERROR;
			}
			break;
		}
	}

	if(fukugou() == ERROR){
		return ERROR;
	}
	return NORMAL;
}

int var_decl(){       /* variable declaration */
	if((token = scan()) == -1){
		return ERROR;
	}
	if(var_names() == ERROR){
		return ERROR;
	}
	if(token != TCOLON){
		error("Colon is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(type() == ERROR){
		return ERROR;
	}
	if(gorl == 0){
		if(globalid_def() == ERROR){
			return ERROR;
		}
	}
	else{
		if(localid_def() == ERROR){
			return ERROR;
		}
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	while(token == TNAME){
		if(var_names() == ERROR){
			return ERROR;
		}
		if(token != TCOLON){
			error("Colon is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
		if(type() == ERROR){
			return ERROR;
		}
		if(gorl == 0){
			if(globalid_def() == ERROR){
				return ERROR;
			}
		}
		else{
			if(localid_def() == ERROR){
				return ERROR;
			}
		}
		if(token != TSEMI){
			error("Semicolon is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
	}

	return NORMAL;
}

int var_names(){      /* variable names */
	struct NAME *p, *q;
	char *cp;
	names = NULL;
	if(token != TNAME){
		error("Variable name is not found!");
		return ERROR;
	}
	if((p= (struct NAME *)malloc(sizeof(struct NAME))) == NULL){
		error("can not malloc in vat_names!");
		return ERROR;
	}
	if((cp = (char *)malloc(strlen(string_attr)+1)) == NULL) {
		error("can not malloc-2 in var_names");
		return ERROR;
	}
	strcpy(cp, string_attr);
	p->name = cp;
	p->next = NULL;
	names = p;
	q = names;
	if((token = scan()) == -1){
		return ERROR;
	}
	while(token == TCOMMA){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(token != TNAME){
			error("Variable name is not found after comma!");
			return ERROR;
		}
		if((p= (struct NAME *)malloc(sizeof(struct NAME))) == NULL){
			error("can not malloc-3 in vat_names!");
			return ERROR;
		}
		if((cp = (char *)malloc(strlen(string_attr)+1)) == NULL) {
			error("can not malloc-4 in var_names!");
			return ERROR;
		}
		strcpy(cp, string_attr);
		p->name = cp;
		p->next = NULL;
		while(q->next != NULL){
			q = q->next;
		}
		q->next = p;
		if((token = scan()) == -1){
			return ERROR;
		}
	}
	return NORMAL;
}

int type(){      /* type */
	int a;
	if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		switch(token){
		case TINTEGER: typenum = TPINT;break;
		case TBOOLEAN: typenum = TPBOOL;break;
		case TCHAR: typenum = TPCHAR;break;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if((a = ar_type()) == NORMAL){
		return NORMAL;
	}
	else if(a == ERROR){
		return ERROR;
	}
	else{
		error("Illegal type!");
		return ERROR;
	}
}

int ar_type(){     /* array type */
	if(token != TARRAY){
		return ERROR;
	}
	typenum = TPARRAY;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TLSQPAREN){
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TNUMBER){
		return ERROR;
	}
	if(num_attr < 1){
		error("Invalid number of elements!");
		return ERROR;
	}
	arraynum = num_attr;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TRSQPAREN){
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TOF){
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TINTEGER && token != TBOOLEAN && token != TCHAR){
		return ERROR;
	}
	switch(token){
	case TINTEGER: arraytype = TPINT;break;
	case TBOOLEAN: arraytype = TPBOOL; break;
	case TCHAR: arraytype = TPCHAR; break;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	return NORMAL;
}

int sub_decl(){    /* subprogram declaration */
	gorl =  1;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	if((procname = (char *)malloc(strlen(string_attr))) == NULL){
		error("cannot malloc in sub_decl!");
		return ERROR;
	}
	strcpy(procname, string_attr);
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLPAREN){
		if(form_para()==ERROR){
			return ERROR;
		}
	}
	if(procedure_def() == ERROR){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TVAR){
		if(var_decl() == ERROR){
			return ERROR;
		}
	}

	if(fukugou() == ERROR){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	gorl = 0;
	joint_localtoglobal();
	localidroot = NULL;
	free(procname);
	if((token = scan()) == -1){
		return ERROR;
	}

	return NORMAL;
}

int form_para(){        /* formal parameters */
	struct NAME *n;
	struct TYPE *p, *q;

	paraflag = 1;
	paratype = NULL;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(var_names() == ERROR){
		return ERROR;
	}
	if(token != TCOLON){
		error("Colon is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(type() == ERROR){
		return ERROR;
	}
	if(localid_def() == ERROR){
		return ERROR;
	}
	for(n = names; n != NULL; n = n->next){
		if((p = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL){
			error("can not malloc in form_para!");
			return ERROR;
		}
		p->ttype = typenum;
		if(p->ttype != TPINT && p->ttype != TPCHAR && p->ttype != TPBOOL){
			error("Formal parameter type is not standard type!");
			return ERROR;
		}
		p->paratp = NULL;
		if(paratype == NULL){
			paratype = p;
			q = paratype;
		}
		else{
			q->paratp = p;
			q = q->paratp;
		}
		free(n);

	}
	while(token == TSEMI){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(var_names() == ERROR){
			return ERROR;
		}
		if(token != TCOLON){
			error("Colon is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
		if(type() == ERROR){
			return ERROR;
		}
		if(localid_def() == ERROR){
			return ERROR;
		}
		for(n = names; n != NULL; n = n->next){
			if((p = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL){
				error("can not malloc-2 in form_para");
				return ERROR;
			}
			p->ttype = typenum;
			if(p->ttype == TARRAY){
				error("Formal parameter is not standard type!");
				return ERROR;
			}
			p->paratp = NULL;
			q->paratp = p;
			q = q->paratp;

		}
	}
	if(token != TRPAREN){
		error("\')\' is not found!");
		return ERROR;
	}
	paraflag = 0;
	if((token = scan()) == -1){
		return ERROR;
	}
	return NORMAL;
}

int fukugou(){      /* compound statement */
	if(token != TBEGIN){
		error("Keyword \"begin\" is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(statement() == ERROR){
		return ERROR;
	}
	while(token == TSEMI){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(statement() == ERROR){
			return ERROR;
		}
	}
	if(token != TEND){
		error("Keyword \"end\" is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	return NORMAL;
}

int statement(){      /* statement */
	if(token == TNAME){
		if(dainyu() == ERROR){
			return ERROR;
		}
	}
	else if(token == TIF){
		if(bunki() == ERROR){
			return ERROR;
		}
	}
	else if(token == TWHILE){
		if(kurikaeshi() == ERROR){
			return ERROR;
		}
	}
	else if(token == TBREAK){
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if(token == TCALL){
		if(call_st() == ERROR){
			return ERROR;
		}
	}
	else if(token == TRETURN){
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if((token == TREAD) || (token == TREADLN)){
		if(input_st() == ERROR){
			return ERROR;
		}
	}
	else if((token == TWRITE) || (token == TWRITELN)){
		if(output_st() == ERROR){
			return ERROR;
		}
	}
	else if(token == TBEGIN){
		if(fukugou() == ERROR){
			return ERROR;
		}
	}
	return NORMAL;
}

int bunki(){      /* condition statement */
	int flag = 0;

	if((token = scan()) == -1){
		return ERROR;
	}
	if(shiki() == ERROR){
		return ERROR;
	}
	if(shikitype != TPBOOL){
		error("The type of the conditional expression is not boolean!");
		return ERROR;
	}
	if(token != TTHEN){
		error("Keyword \"then\" is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TBEGIN){
		flag = 1;
	}
	if(statement() == ERROR){
		return ERROR;
	}
	if(flag == 1){
		flag = 0;
	}
	if(token == TELSE){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(token != TBEGIN){
			flag = 1;
		}
		if(statement() == ERROR){
			return ERROR;
		}
		if(flag == 1){
			flag = 0;
		}
	}

	return NORMAL;
}

int kurikaeshi(){         /* iteration statement */
	int flag = 0;

	if((token = scan()) == -1){
		return ERROR;
	}
	if(shiki() == ERROR){
		return ERROR;
	}
	if(shikitype != TPBOOL){
		error("The type of the conditional expression is not boolean!");
		return ERROR;
	}
	if(token != TDO){
		error("Keyword \"do\" is not found!");
		return ERROR;
	}
	token =scan();
	if(token != TBEGIN){
		flag = 1;
	}
	if(statement() == ERROR){
		return ERROR;
	}
	if(flag == 1){
		flag = 0;
	}

	return NORMAL;
}

int call_st(){   /* call statement */
	struct TYPE *q;

	paranum = 0;
	expnum = 0;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token !=TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	if(strcmp(string_attr, procname) == 0){
		error("Illegal call!");
		return ERROR;
	}
	if(gorl == 0){
		if(globalid_ref(string_attr) == ERROR){
			return ERROR;
		}
	}
	else{
		if(localid_ref(string_attr) == ERROR){
			return ERROR;
		}
	}
	q = searchp->itp->paratp;
	while(q != NULL){
		paranum++;
		q = q->paratp;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLPAREN){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(exp_narabi() == ERROR){
			return ERROR;
		}
		if(token != TRPAREN){
			error("\")\" is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
	}
	if(paranum != expnum){
		error("The number of arguments is invalid!");
		return ERROR;
	}

	return NORMAL;
}

int exp_narabi(){       /* expressions */
	struct TYPE *q;

	q = searchp->itp->paratp;
	if(shiki() == ERROR){
		return ERROR;
	}
	if(shikitype != q->ttype){
		error("The argument type is invalid!");
		return ERROR;
	}
	q = q->paratp;
	expnum++;
	while(token == TCOMMA){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(shiki() == ERROR){
			return ERROR;
		}
		if(shikitype != q->ttype){
			error("The argument type is invalid!");
			return ERROR;
		}
		q = q->paratp;
		expnum++;
	}

	return NORMAL;
}

int dainyu(){          /* assignment statement */
	int type1, type2;
	if(var() == ERROR){
		return ERROR;
	}
	type1 = vartype;
	if(token != TASSIGN){
		error("\":=\" is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(shiki() == ERROR){
		return ERROR;
	}
	type2 = shikitype;
	if(type1 != type2){
		error("The type of expression to assign is illegal!");
		return ERROR;
	}
	if(type1 == TPARRAY){
		error("The type of expression to assign is illegal!");
		return ERROR;
	}

	return NORMAL;
}

int var(){          /* variable */
	int arraytype;
	if(token != TNAME){
		error("Variable name is not found!");
		return ERROR;
	}
	if(gorl == 0){
		if(globalid_ref(string_attr) == ERROR){
			return ERROR;
		}
	}
	else{
		if(localid_ref(string_attr) == ERROR){
			return ERROR;
		}
	}


	vartype = searchp->itp->ttype;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLSQPAREN){
		if(vartype != TPARRAY){
			error("The type of the variable name is invalid!");
			return ERROR;
		}
		arraytype = searchp->itp->etp->ttype;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(shiki() == ERROR){
			return ERROR;
		}
		if(shikitype != TPINT){
			error("The type of array index is not integer!");
			return ERROR;
		}
		if(token != TRSQPAREN){
			error("\"]\" is not found!");
			return ERROR;
		}
		vartype = arraytype;
		if((token = scan()) == -1){
			return ERROR;
		}
		arrayflag = 1;
	}

	return NORMAL;
}

int shiki(){	/* expression */
	int type1, type2;
	int arraysize1 = 0, arraysize2 = 0;
	int arraytype1 = 0, arraytype2 = 0;
	if(simple() == ERROR){
		return ERROR;
	}
	shikitype = simpletype;
	type1 = simpletype;
	if(arrayflag == 1){
		arraysize1 = simplearraysize;
		arraytype1 = simplearraytype;
	}
	while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(simple() == ERROR){
			return ERROR;
		}
		type2 = simpletype;
		if(arrayflag == 1){
			arraysize2 = simplearraysize;
			arraytype2 = simplearraytype;
		}
		if((type1 != type2) || (arraysize1 != arraysize2) || (arraytype1 != arraytype2)){
			error("Different type comparing!");
			return ERROR;
		}
		shikitype = TPBOOL;
		type1 = type2;
		arraysize1 = arraysize2;
		arraytype1 = arraytype2;
	}

	return NORMAL;
}

int simple(){	/* simple expression */
	int flag = 0;
	int type1, type2;
	int kahou;

	if(token == TPLUS || token == TMINUS){
		flag = 1;
		if((token = scan()) == -1){
			return ERROR;
		}
	}
	if(kou() == ERROR){
		return ERROR;
	}
	if(flag == 1){
		if(koutype != TPINT){
			error("The type of term is not integer!");
			return ERROR;
		}
	}
	simpletype = koutype;
	type1 = simpletype;
	if(arrayflag == 1){
		simplearraysize = kouarraysize;
		simplearraytype = kouarraytype;
	}
	while(token == TPLUS || token == TMINUS || token == TOR){
		kahou = token;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(kou() == ERROR){
			return ERROR;
		}
		type2 = koutype;
		if(kahou == TPLUS || kahou == TMINUS){
			if(type1 != TPINT || type2 !=TPINT){
				error("The operand type of the arithmetic operator is not integer!");
				return ERROR;
			}
			simpletype = TPINT;
		}
		else if(kahou == TOR){
			if(type1 != TPBOOL || type2 != TPBOOL){
				error("The operand type of the logical operator is not boolean!");
				return ERROR;
			}
			simpletype = TPBOOL;
		}
		type1 = type2;
	}

	return NORMAL;
}

int kou(){	/* term */
	int type1, type2;
	int jouhou;
	if(inshi() == ERROR){
		return ERROR;
	}
	koutype = inshitype;
	type1 = koutype;
	if(arrayflag == 1){
		kouarraysize = inshiarraysize;
		kouarraytype = inshiarraytype;
	}
	while(token == TSTAR || token == TAND || token == TDIV) {
		jouhou = token;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(inshi() == ERROR){
			return ERROR;
		}
		type2 = inshitype;
		if(jouhou == TSTAR || jouhou == TDIV){
			if(type1 != TPINT ||type2 != TPINT){
				error("The operand type of the arithmetic operator is not integer!");
				return ERROR;
			}
			koutype = TPINT;
		}
		else if(jouhou == TAND){
			if(type1 != TPBOOL || type2 != TPBOOL){
				error("The operand type of the logical operator is not boolean!");
				return ERROR;
			}
			koutype = TPBOOL;
		}
	}
	return NORMAL;
}

int inshi(){	/* factor */
	int type1;

	arrayflag = 0;
	if(token == TNAME){
		if(var() == ERROR){
			return ERROR;
		}
		inshitype = vartype;
		if(arrayflag == 1){
			inshiarraysize = vararraysize;
			inshiarraytype = vararraytype;
		}
	}
	else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
		if(token == TNUMBER){
			inshitype = TPINT;
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TFALSE){
			inshitype = TPBOOL;
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TTRUE){
			inshitype = TPBOOL;
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TSTRING){
			inshitype = TPCHAR;
			if((token = scan()) == -1){
				return ERROR;
			}
		}
	}
	else if(token == TLPAREN){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(shiki() == ERROR){
			return ERROR;
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		inshitype = shikitype;
		if(arrayflag == 1){
			inshiarraysize = shikiarraysize;
			inshiarraytype = shikiarraytype;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
	}
	else if(token == TNOT){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(inshi() == ERROR){
			return ERROR;
		}
		if(inshitype != TPBOOL){
			error("The type of operand of 'not' is not boolean!");
			return ERROR;
		}
	}
	else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		switch(token){
		case TINTEGER: inshitype = TPINT;break;
		case TBOOLEAN: inshitype = TPBOOL;break;
		case TCHAR: inshitype = TPCHAR;
		}
		type1 = inshitype;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(token != TLPAREN){
			error("\'(\' is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
		if(shiki() == ERROR){
			return ERROR;
		}
		if(shikitype != TPINT && shikitype != TPCHAR && shikitype != TPBOOL){
			error("The type of expression is not standard type!");
			return ERROR;
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		inshitype = type1;
		if((token = scan()) == -1){
			return ERROR;
		}
	}
	else{
		error("Illegal factor!");
		return ERROR;
	}
	return NORMAL;
}

int input_st(){	/* input statement */
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLPAREN){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(var() == ERROR){
			return ERROR;
		}
		if(vartype != TPINT && vartype != TPCHAR){
			error("The type of the expression is neither integer nor char!");
			return ERROR;
		}
		while(token == TCOMMA){
			if((token = scan()) == -1){
				return ERROR;
			}
			if(var() == ERROR){
				return ERROR;
			}
			if(vartype != TPINT && vartype != TPCHAR){
				error("The type of the expression is neither integer nor char!");
				return ERROR;
			}
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
	}

	return NORMAL;
}

int output_st(){	/* output statement */
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLPAREN){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(shitei() == ERROR){
			return ERROR;
		}
		while(token == TCOMMA){
			if((token = scan()) == -1){
				return ERROR;
			}
			if(shitei() == ERROR){
				return ERROR;
			}
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		if((token = scan()) == -1){
			return ERROR;
		}
	}

	return NORMAL;

}


int shitei(){	/* output format */
	if(token == TSTRING && strlen(string_attr) != 1){
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if(token == TPLUS || token == TMINUS || token == TNAME || token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING || token == TLPAREN || token == TNOT || token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		if(shiki() == ERROR){
			return ERROR;
		}
		if((shikitype != TPINT) && (shikitype != TPCHAR) && (shikitype != TPBOOL)){
			error("The type of the expression is not standard type!");
			return ERROR;
		}
		if(token == TCOLON){
			if((token = scan()) == -1){
				return ERROR;
			}
			if(token != TNUMBER){
				error("Number is not found!");
				return ERROR;
			}
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		return NORMAL;
	}

	error("Illegal output format!");
	return ERROR;
}

