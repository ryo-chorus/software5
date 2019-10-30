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
int inlab_num = 0;
int adflag = 0;
int callflag = 0;
int eorv = 0;
int inputflag = 0;

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
struct STRING{
	char *string;
	int label;
	struct STRING *next;
}*stringhead,*stringtail;
char *procname;
struct NAME *para;

int parse_program(){   /* program */
	int label;
	struct STRING *p,*q;

	stringtail = stringhead;
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
	fprintf(fpw, "$$%s\tSTART\n", string_attr);
	label = get_inlabel();
	fprintf(fpw, "\tLAD     gr0,0\n");
	fprintf(fpw, "\tCALL    L%04d\n", label);
	fprintf(fpw, "\tCALL    FLUSH\n");
	fprintf(fpw, "\tSVC     0\n");
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
	if(block(label) == ERROR) return ERROR;
	if(token != TDOT){
		error("Period is not found at the end of program!");
		return ERROR;
	}
	fprintf(fpw, "    RET\n");
	for(p = stringhead; p != NULL; p = q){
		fprintf(fpw, "L%04d  DC     %s\n", p->label, p->string);
		free(p->string);
		q = p->next;
		free(p);
	}
	token = scan();
	lib();
	return NORMAL;
}

int block(int label){      /* block */
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
	fprintf(fpw, "L%04d\n", label);
	if(fukugou(label) == ERROR){
		return ERROR;
	}
	return NORMAL;
}

int var_decl(){       /* variable declaration */
	struct NAME *p;
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
	if(typenum == TPARRAY){
		for(p = names; p != NULL; p = p->next){
			if(procname == NULL){
				fprintf(fpw, "$%s\tDS      %d\n", p->name, arraynum);
			}
			else{
				fprintf(fpw, "$%s%%%s\tDS      %d\n",p->name, procname, arraynum);
			}
		}
	}
	else{
		for(p = names; p != NULL; p = p->next){
			if(procname == NULL){
				fprintf(fpw, "$%s\tDC      0\n", p->name);
			}
			else{
				fprintf(fpw, "$%s%%%s\tDC      0\n",p->name, procname);
			}
		}
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
		if(typenum == TPARRAY){
			for(p = names; p != NULL; p = p->next){
				if(procname == NULL){
					fprintf(fpw, "$%s\tDS      %d\n", p->name, arraynum);
				}
				else{
					fprintf(fpw, "$%s%%%s\tDS      %d\n",p->name, procname, arraynum);
				}
			}
		}
		else{
			for(p = names; p != NULL; p = p->next){
				if(procname == NULL){
					fprintf(fpw, "$%s\tDC      0\n", p->name);
				}
				else{
					fprintf(fpw, "$%s%%%s\tDC      0\n",p->name, procname);
				}
			}
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
	struct NAME *p, *q;
	int flag = 0;
	gorl =  1;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token != TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	if((procname = (char *)malloc(strlen(string_attr)+1)) == NULL){
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
		flag = 1;
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
	fprintf(fpw, "$%s\n", procname);
	if(flag == 1){
		fprintf(fpw, "    POP     gr2\n");
		for(p = para; p != NULL; p = q) {
			fprintf(fpw, "    POP     gr1\n");
			fprintf(fpw, "    ST      gr1,$%s%%%s\n", p->name, procname);
			free(p->name);
			q = p->next;
			free(p);
		}
		para = NULL;
		fprintf(fpw, "    PUSH    0,gr2\n");
		flag = 0;
	}
	if(fukugou(0) == ERROR){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	fprintf(fpw, "\tRET\n");
	gorl = 0;
	joint_localtoglobal();
	localidroot = NULL;
	free(procname);
	procname = NULL;
	if((token = scan()) == -1){
		return ERROR;
	}

	return NORMAL;
}

int form_para(){        /* formal parameters */
	struct NAME *n, *m, *l;
	struct TYPE *p, *q;
	char *cp;

	paraflag = 1;
	paratype = NULL;
	para = NULL;
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
	if(typenum == TPARRAY){
		for(m = names; m != NULL; m = m->next){
			fprintf(fpw, "$%s%%%s\tDS      %d\n",m->name, procname, arraynum);

		}
	}
	else{
		for(m = names; m != NULL; m = m->next){
			fprintf(fpw, "$%s%%%s\tDC      0\n",m->name, procname);

		}
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
		if((l = (struct NAME *)malloc(sizeof(struct NAME))) == NULL) {
			fprintf(stderr, "can not malloc in form_para\n");
			return ERROR;
		}
		if((cp = (char *)malloc(strlen(n->name)+1)) == NULL) {
			fprintf(stderr, "can not malloc-2 in id_countup\n");
			return ERROR;
		}
		strcpy(cp, n->name);
		l->name = cp;
		l->next = para;
		para = l;
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
			if((l = (struct NAME *)malloc(sizeof(struct NAME))) == NULL) {
				fprintf(stderr, "can not malloc in form_para\n");
				return ERROR;
			}
			if((cp = (char *)malloc(strlen(n->name)+1)) == NULL) {
				fprintf(stderr, "can not malloc-2 in id_countup\n");
				return ERROR;
			}
			strcpy(cp, n->name);
			l->name = cp;
			l->next = para;
			para = l;
			free(n);


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

int fukugou(int label){      /* compound statement */
	if(token != TBEGIN){
		error("Keyword \"begin\" is not found!");
		return ERROR;
	}
	if((token = scan()) == -1){
		return ERROR;
	}
	if(statement(label) == ERROR){
		return ERROR;
	}
	while(token == TSEMI){
		if((token = scan()) == -1){
			return ERROR;
		}
		if(statement(label) == ERROR){
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

int statement(int label){      /* statement */
	if(token == TNAME){
		if(dainyu(label) == ERROR){
			return ERROR;
		}
	}
	else if(token == TIF){
		if(bunki(label) == ERROR){
			return ERROR;
		}
	}
	else if(token == TWHILE){
		if(kurikaeshi(label) == ERROR){
			return ERROR;
		}
	}
	else if(token == TBREAK){
		fprintf(fpw, "    JUMP    L%04d\n", label);
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if(token == TCALL){
		if(call_st(label) == ERROR){
			return ERROR;
		}
	}
	else if(token == TRETURN){
		fprintf(fpw, "    RET\n");
		if((token = scan()) == -1){
			return ERROR;
		}
		return NORMAL;
	}
	else if((token == TREAD) || (token == TREADLN)){
		if(input_st(label) == ERROR){
			return ERROR;
		}
	}
	else if((token == TWRITE) || (token == TWRITELN)){
		if(output_st(label) == ERROR){
			return ERROR;
		}
	}
	else if(token == TBEGIN){
		if(fukugou(label) == ERROR){
			return ERROR;
		}
	}
	return NORMAL;
}

int bunki(int label){      /* condition statement */
	int flag = 0;
	int label1, label2;

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
	label1 = get_inlabel();
	fprintf(fpw, "    POP    gr1\n");
	fprintf(fpw, "    CPA    gr1,gr0\n");
	fprintf(fpw, "    JZE    L%04d\n", label1);
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
	if(statement(label) == ERROR){
		return ERROR;
	}
	if(flag == 1){
		flag = 0;
	}
	if(token == TELSE){
		label2 = get_inlabel();
		fprintf(fpw, "    JUMP   L%04d\n", label2);
		fprintf(fpw, "L%04d\n", label1);
		if((token = scan()) == -1){
			return ERROR;
		}
		if(token != TBEGIN){
			flag = 1;
		}
		if(statement(label) == ERROR){
			return ERROR;
		}
		fprintf(fpw,"L%04d\n", label2);
		if(flag == 1){
			flag = 0;
		}
	}
	else{
		fprintf(fpw, "L%04d\n", label1);
	}

	return NORMAL;
}

int kurikaeshi(int label){         /* iteration statement */
	int flag = 0;
	int label1,label2;

	label1 = get_inlabel();
	fprintf(fpw,"L%04d\n", label1);
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
	label2=get_inlabel();
	fprintf(fpw, "    POP      gr1\n");
	fprintf(fpw, "    CPA      gr1,gr0\n");
	fprintf(fpw, "    JZE      L%04d\n", label2);
	token =scan();
	if(token != TBEGIN){
		flag = 1;
	}
	if(statement(label2) == ERROR){
		return ERROR;
	}
	if(flag == 1){
		flag = 0;
	}
	fprintf(fpw, "    JUMP     L%04d\n",label1);
	fprintf(fpw, "L%04d\n", label2);

	return NORMAL;
}

int call_st(int label){   /* call statement */
	struct TYPE *q;
	char *callname;

	adflag = 1;
	callflag = 1;
	paranum = 0;
	expnum = 0;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token !=TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	if((callname = (char *)malloc(strlen(string_attr)+1)) == NULL){
		fprintf(stderr, "can not malloc in call_st\n");
		return ERROR;
	}

	strcpy(callname,string_attr);
	if(procname != NULL){
		if(strcmp(string_attr, procname) == 0){
			error("Illegal call!");
			return ERROR;
		}
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

	adflag = 0;
	callflag = 0;

	fprintf(fpw, "    CALL    $%s\n", callname);


	return NORMAL;
}

int exp_narabi(){       /* expressions */
	struct TYPE *q;
	int label;
	struct STRING *p;
	char *cp;


	eorv = 0;
	q = searchp->itp->paratp;
	if(shiki() == ERROR){
		return ERROR;
	}
	if(shikitype != q->ttype){
		error("The argument type is invalid!");
		return ERROR;
	}
	if(eorv == 1){
		fprintf(fpw, "    POP     gr1\n");
		label = get_inlabel();
		fprintf(fpw, "    LAD     gr2,L%04d\n", label);
		fprintf(fpw, "    ST      gr1,0,gr2\n");
		fprintf(fpw, "    PUSH    0,gr2\n");
		if((p = (struct STRING *)malloc(sizeof(struct STRING))) == NULL){
			fprintf(stderr,"can not malloc in inshi");
			return ERROR;
		}
		if((cp = (char *)malloc(strlen("0")+1)) == NULL){
			fprintf(stderr, "can not malloc-2 in inshi");
			return ERROR;
		}
		sprintf(cp, "0");
		p->string = cp;
		p->label = label;
		p->next = NULL;
		if(stringhead == NULL){
			stringhead = p;
			stringtail = stringhead;
		}
		else{
			stringtail->next = p;
			stringtail = stringtail->next;
		}
		eorv = 0;
	}
	q = q->paratp;
	expnum++;
	while(token == TCOMMA){
		eorv = 0;
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
		if(eorv == 1){
			fprintf(fpw, "    POP     gr1\n");
			label = get_inlabel();
			fprintf(fpw, "    LAD     gr2,L%04d\n", label);
			fprintf(fpw, "    ST      gr1,0,gr2\n");
			fprintf(fpw, "    PUSH    0,gr2\n");
			if((p = (struct STRING *)malloc(sizeof(struct STRING))) == NULL){
				fprintf(stderr,"can not malloc in inshi");
				return ERROR;
			}
			if((cp = (char *)malloc(strlen("0")+1)) == NULL){
				fprintf(stderr, "can not malloc-2 in inshi");
				return ERROR;
			}
			sprintf(cp, "0");
			p->string = cp;
			p->label = label;
			p->next = NULL;
			if(stringhead == NULL){
				stringhead = p;
				stringtail = stringhead;
			}
			else{
				stringtail->next = p;
				stringtail = stringtail->next;
			}
			eorv = 0;
		}
		q = q->paratp;
		expnum++;
	}

	return NORMAL;
}

int dainyu(int label){          /* assignment statement */
	int type1, type2;

	adflag = 1;
	if(var() == ERROR){
		return ERROR;
	}
	adflag=0;
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
	fprintf(fpw, "    POP      gr1\n");
	fprintf(fpw, "    POP      gr2\n");
	fprintf(fpw, "    ST       gr1,0,gr2\n");

	return NORMAL;
}

int var(){          /* variable */
	int arraytype, arraysize;
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
	if(searchp->itp->ttype == TPARRAY){
		adflag = 1;
	}
	if(adflag == 1){
		if(inputflag == 1){
			if(searchp->procname == NULL){
				fprintf(fpw, "    LAD      gr1,    $%s\n", string_attr);
			}
			else{
				if(searchp->ispara == 1){
					fprintf(fpw, "    LD      gr1,    $%s%%%s\n", string_attr, searchp->procname);
				}
				else{
					fprintf(fpw, "    LAD      gr1,    $%s%%%s\n", string_attr, searchp->procname);
				}
			}
		}
		else{
			if(searchp->procname == NULL){
				fprintf(fpw, "    LAD      gr1,    $%s\n", string_attr);
			}
			else{
				if(searchp->ispara == 0){
					fprintf(fpw, "    LAD      gr1,    $%s%%%s\n", string_attr, searchp->procname);
				}
				else{
					fprintf(fpw, "    LD      gr1,    $%s%%%s\n", string_attr, searchp->procname);
				}
			}
		}
	}
	else{
		if(searchp->procname == NULL){
			fprintf(fpw, "    LD       gr1,    $%s\n", string_attr);
		}
		else{
			fprintf(fpw, "    LD       gr1,    $%s%%%s\n", string_attr, searchp->procname);
			if(searchp->ispara == 1){
				fprintf(fpw, "    LD      gr1,0,gr1\n");
			}
		}
	}

	fprintf(fpw, "    PUSH    0,gr1\n");

	vartype = searchp->itp->ttype;
	if((token = scan()) == -1){
		return ERROR;
	}
	if(token == TLSQPAREN){
		adflag = 0;
		if(vartype != TPARRAY){
			error("The type of the variable name is invalid!");
			return ERROR;
		}
		arraytype = searchp->itp->etp->ttype;
		arraysize = searchp->itp->arraysize;
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
		fprintf(fpw, "    POP    gr1\n");
		fprintf(fpw, "    LAD    gr2,%d\n", arraysize-1);
		fprintf(fpw, "    CPA    gr1,gr2\n");
		fprintf(fpw, "    JPL    EROV\n");
		fprintf(fpw, "    CPA    gr1,gr0\n");
		fprintf(fpw, "    JMI    EROV\n");
		fprintf(fpw, "    POP    gr2\n");
		fprintf(fpw, "    ADDA   gr1,gr2\n");
		fprintf(fpw, "    PUSH   0,gr1\n");
		arrayflag = 1;
	}

	return NORMAL;
}

int shiki(){	/* expression */
	int type1, type2;
	int arraysize1 = 0, arraysize2 = 0;
	int arraytype1 = 0, arraytype2 = 0;
	int opr;
	int label1,label2;
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
		eorv = 1;
		opr = token;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(simple() == ERROR){
			return ERROR;
		}
		type2 = simpletype;
		if(type2 == TPARRAY){
			arraysize2 = simplearraysize;
			arraytype2 = simplearraytype;
		}
		if((type1 != type2) || (arraysize1 != arraysize2) || (arraytype1 != arraytype2)){
			error("Different type comparing!");
			return ERROR;
		}
		label1 = get_inlabel();
		label2 = get_inlabel();
		fprintf(fpw, "    POP     gr2\n");
		fprintf(fpw, "    POP     gr1\n");
		if(callflag == 1){
			fprintf(fpw, "    LD      gr1,0,gr1\n");
		}
		fprintf(fpw, "    CPA     gr1,gr2\n");
		if(opr == TEQUAL){
			fprintf(fpw, "    JZE     L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
		}
		else if(opr == TNOTEQ){
			fprintf(fpw, "    JZE     L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
		}
		else if(opr == TLE){
			fprintf(fpw, "    JMI     L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
		}
		else if(opr == TLEEQ){
			fprintf(fpw, "    JPL     L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
		}
		else if(opr == TGR){
			fprintf(fpw, "    JPL     L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
		}
		else{
			fprintf(fpw, "    JMI     L%04d\n", label1);
			fprintf(fpw, "    LAD     gr1,1\n");
			fprintf(fpw, "    JUMP    L%04d\n", label2);
			fprintf(fpw, "L%04d\n", label1);
			fprintf(fpw, "    LD     gr1,gr0\n");
		}
		fprintf(fpw, "L%04d\n", label2);
		fprintf(fpw, "    PUSH    0,gr1\n");
		shikitype = TPBOOL;
		type1 = type2;
		arraysize1 = arraysize2;
		arraytype1 = arraytype2;
	}

	return NORMAL;
}

int simple(){	/* simple expression */
	int flag = 0;
	int minusflag = 0;
	int type1, type2;
	int kahou;

	if(token == TPLUS || token == TMINUS){
		flag = 1;
		if(token == TMINUS){
			minusflag = 1;
		}
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
		eorv = 1;
		kahou = token;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(kou() == ERROR){
			return ERROR;
		}
		fprintf(fpw, "    POP    gr2\n");
		fprintf(fpw, "    POP    gr1\n");
		if(callflag == 1){
			fprintf(fpw, "    LD      gr1,0,gr1\n");
		}
		type2 = koutype;
		if(kahou == TPLUS || kahou == TMINUS){
			if(type1 != TPINT || type2 !=TPINT){
				error("The operand type of the arithmetic operator is not integer!");
				return ERROR;
			}
			if(kahou == TPLUS){
				fprintf(fpw, "    ADDA    gr1,gr2\n");
			}
			else{
				fprintf(fpw, "    SUBA    gr1,gr2\n");
			}
			fprintf(fpw, "    JOV     EOVF\n");
			simpletype = TPINT;
		}
		else if(kahou == TOR){
			if(type1 != TPBOOL || type2 != TPBOOL){
				error("The operand type of the logical operator is not boolean!");
				return ERROR;
			}
			fprintf(fpw, "    OR      gr1,gr2\n");
			simpletype = TPBOOL;
		}
		fprintf(fpw, "    PUSH    0,gr1\n");
		type1 = type2;
	}

	if(minusflag == 1){
		eorv = 1;
		fprintf(fpw, "    POP     gr1\n");
		if(callflag == 1){
			fprintf(fpw, "    LD      gr1,0,gr1\n");
		}
		fprintf(fpw, "    LAD     gr2,-1\n");
		fprintf(fpw, "    MULA    gr1,gr2\n");
		fprintf(fpw, "    JOV     EOVF\n");
		fprintf(fpw, "    PUSH    0,gr1\n");
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
		eorv = 1;
		jouhou = token;
		if((token = scan()) == -1){
			return ERROR;
		}
		if(inshi() == ERROR){
			return ERROR;
		}
		type2 = inshitype;
		fprintf(fpw, "    POP    gr2\n");
		fprintf(fpw, "    POP    gr1\n");
		if(callflag == 1){
			fprintf(fpw, "    LD      gr1,0,gr1\n");
		}
		if(jouhou == TSTAR || jouhou == TDIV){
			if(type1 != TPINT ||type2 != TPINT){
				error("The operand type of the arithmetic operator is not integer!");
				return ERROR;
			}
			if(jouhou == TSTAR){
				fprintf(fpw, "    MULA    gr1,gr2\n");
				fprintf(fpw, "    JOV     EOVF\n");
			}
			else if(jouhou == TDIV){
				fprintf(fpw, "    DIVA    gr1,gr2\n");
				fprintf(fpw, "    JOV     E0DIV\n");
			}
			koutype = TPINT;
		}
		else if(jouhou == TAND){
			if(type1 != TPBOOL || type2 != TPBOOL){
				error("The operand type of the logical operator is not boolean!");
				return ERROR;
			}
			fprintf(fpw, "    AND    gr1,gr2\n");
			koutype = TPBOOL;
		}
		fprintf(fpw, "    PUSH    0,gr1\n");
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
			fprintf(fpw, "    POP    gr1\n");
			fprintf(fpw, "    LD     gr1,0,gr1\n");
			fprintf(fpw, "    PUSH   0,gr1\n");
		}
	}
	else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
		if(token == TNUMBER){
			inshitype = TPINT;
			fprintf(fpw, "    LAD     gr1,%d\n", num_attr);
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TFALSE){
			inshitype = TPBOOL;
			fprintf(fpw, "    LD      gr1,gr0\n");
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TTRUE){
			inshitype = TPBOOL;
			fprintf(fpw, "    LAD      gr1,1\n");
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		else if(token == TSTRING){
			inshitype = TPCHAR;

			if(strlen(string_attr) != 1){
				error("Illegal String length!");
				return ERROR;
			}
			fprintf(fpw, "    LAD     gr1,%d\n", (int)string_attr[0]);
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		fprintf(fpw, "    PUSH    0,gr1\n");
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
		if(inshitype == TPARRAY){
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
		fprintf(fpw, "    LAD     gr2,1\n");
		fprintf(fpw, "    POP     gr1\n");
		fprintf(fpw, "    XOR     gr1,gr2\n");
		fprintf(fpw, "    PUSH    0,gr1\n");
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
		if(inshitype == TPCHAR && shikitype == TPINT){
			fprintf(fpw, "    POP     gr1\n");
			fprintf(fpw, "    LAD     gr2,#7F");
			fprintf(fpw, "    AND     gr1,gr2\n");
			fprintf(fpw, "    PUSH    0,gr1\n");
		}
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

int input_st(int label){	/* input statement */
	int op;

	adflag = 1;
	inputflag = 1;
	op = token;
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
		fprintf(fpw, "    POP     gr1\n");
		if(vartype == TPINT){
			fprintf(fpw, "    CALL    READINT\n");
		}
		else{
			fprintf(fpw, "    CALL    READCHAR\n");
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
			if(vartype == TPINT){
				fprintf(fpw, "    CALL    READINT\n");
			}
			else{
				fprintf(fpw, "    CALL    READCHAR\n");
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
	if(op == TREADLN){
		fprintf(fpw, "    CALL     READLINE\n");
	}

	adflag = 0;
	inputflag = 0;

	return NORMAL;
}

int output_st(int label){	/* output statement */
	int op;

	op = token;
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
	if(op == TWRITELN){
		fprintf(fpw, "    CALL     WRITELINE\n");
	}

	return NORMAL;

}


int shitei(){	/* output format */
	struct STRING *p;
	char *cp;
	int label;
	if(token == TSTRING && strlen(string_attr) != 1){
		label = get_inlabel();
		if((p = (struct STRING *)malloc(sizeof(struct STRING))) == NULL){
			fprintf(stderr,"can not malloc in inshi");
			return ERROR;
		}
		if((cp = (char *)malloc(strlen(string_attr)+3)) == NULL){
			fprintf(stderr, "can not malloc-2 in inshi");
			return ERROR;
		}
		sprintf(cp, "\'%s\'", string_attr);
		p->string = cp;
		p->label = label;
		p->next = NULL;
		if(stringhead == NULL){
			stringhead = p;
			stringtail = stringhead;
		}
		else{
			stringtail->next = p;
			stringtail = stringtail->next;
		}
		fprintf(fpw, "    LAD     gr1,    L%04d\n", label);
		fprintf(fpw, "    LD      gr2,gr0\n");
		fprintf(fpw, "    CALL    WRITESTR\n");
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
		fprintf(fpw, "    POP    gr1\n");
		fprintf(fpw, "    LD     gr2,gr0\n");
		if(token == TCOLON){
			if((token = scan()) == -1){
				return ERROR;
			}
			if(token != TNUMBER){
				error("Number is not found!");
			}
			fprintf(fpw, "    LAD     gr2,%d\n", num_attr);
			if((token = scan()) == -1){
				return ERROR;
			}
		}
		if(shikitype == TPINT){
			fprintf(fpw, "    CALL    WRITEINT\n");
		}
		else if(shikitype ==TPBOOL){
			fprintf(fpw, "    CALL    WRITEBOOL\n");
		}
		else{
			fprintf(fpw, "    CALL    WRITECHAR\n");
		}
		return NORMAL;
	}



	error("Illegal output format!");
	return ERROR;
}

int get_inlabel(){
	return ++inlab_num;
}

void lib(){
	fprintf(fpw,"EOVF\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  LAD  gr1, EOVF1\n");
	fprintf(fpw,"  LD  gr2, gr0\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  SVC  1\n");
	fprintf(fpw,"EOVF1    DC  \'***** Run-Time Error : Overflow *****\'\n");
	fprintf(fpw,"E0DIV\n");
	fprintf(fpw,"  JNZ  EOVF\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  LAD  gr1, E0DIV1\n");
	fprintf(fpw,"  LD  gr2, gr0\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  SVC  2\n");
	fprintf(fpw,"E0DIV1    DC  \'***** Run-Time Error : Zero-Divide *****\'\n");
	fprintf(fpw,"EROV\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  LAD  gr1, EROV1\n");
	fprintf(fpw,"  LD  gr2, gr0\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  CALL  WRITELINE\n");
	fprintf(fpw,"  SVC  3\n");
	fprintf(fpw,"EROV1    DC  \'***** Run-Time Error : Range-Over in Array Index *****\'\n");
	fprintf(fpw,"WRITECHAR\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD  gr6, SPACE\n");
	fprintf(fpw,"  LD  gr7, OBUFSIZE\n");
	fprintf(fpw,"WC1\n");
	fprintf(fpw,"  SUBA  gr2, ONE\n");
	fprintf(fpw,"  JZE  WC2\n");
	fprintf(fpw,"  JMI  WC2\n");
	fprintf(fpw,"  ST  gr6, OBUF,gr7\n");
	fprintf(fpw,"  CALL  BOVFCHECK\n");
	fprintf(fpw,"  JUMP  WC1\n");
	fprintf(fpw,"WC2\n");
	fprintf(fpw,"  ST  gr1, OBUF,gr7\n");
	fprintf(fpw,"  CALL  BOVFCHECK\n");
	fprintf(fpw,"  ST  gr7, OBUFSIZE\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"WRITESTR\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD  gr6, gr1\n");
	fprintf(fpw,"WS1\n");
	fprintf(fpw,"  LD  gr4, 0,gr6\n");
	fprintf(fpw,"  JZE  WS2\n");
	fprintf(fpw,"  ADDA  gr6, ONE\n");
	fprintf(fpw,"  SUBA  gr2, ONE\n");
	fprintf(fpw,"  JUMP  WS1\n");
	fprintf(fpw,"WS2\n");
	fprintf(fpw,"  LD  gr7, OBUFSIZE\n");
	fprintf(fpw,"  LD  gr5, SPACE\n");
	fprintf(fpw,"WS3\n");
	fprintf(fpw,"  SUBA  gr2, ONE\n");
	fprintf(fpw,"  JMI  WS4\n");
	fprintf(fpw,"  ST  gr5, OBUF,gr7\n");
	fprintf(fpw,"  CALL  BOVFCHECK\n");
	fprintf(fpw,"  JUMP  WS3\n");
	fprintf(fpw,"WS4\n");
	fprintf(fpw,"  LD  gr4, 0,gr1\n");
	fprintf(fpw,"  JZE  WS5\n");
	fprintf(fpw,"  ST  gr4, OBUF,gr7\n");
	fprintf(fpw,"  ADDA  gr1, ONE\n");
	fprintf(fpw,"  CALL  BOVFCHECK\n");
	fprintf(fpw,"  JUMP  WS4\n");
	fprintf(fpw,"WS5\n");
	fprintf(fpw,"  ST  gr7, OBUFSIZE\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"BOVFCHECK\n");
	fprintf(fpw,"    ADDA  gr7, ONE\n");
	fprintf(fpw,"    CPA   gr7, BOVFLEVEL\n");
	fprintf(fpw,"    JMI  BOVF1\n");
	fprintf(fpw,"    CALL  WRITELINE\n");
	fprintf(fpw,"    LD gr7, OBUFSIZE\n");
	fprintf(fpw,"BOVF1\n");
	fprintf(fpw,"    RET\n");
	fprintf(fpw,"BOVFLEVEL  DC 256\n");
	fprintf(fpw,"WRITEINT\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD  gr7, gr0\n");
	fprintf(fpw,"  CPA  gr1, gr0\n");
	fprintf(fpw,"  JPL  WI1\n");
	fprintf(fpw,"  JZE  WI1\n");
	fprintf(fpw,"  LD  gr4, gr0\n");
	fprintf(fpw,"  SUBA  gr4, gr1\n");
	fprintf(fpw,"  CPA  gr4, gr1\n");
	fprintf(fpw,"  JZE  WI6\n");
	fprintf(fpw,"  LD  gr1, gr4\n");
	fprintf(fpw,"  LD  gr7, ONE\n");
	fprintf(fpw,"WI1\n");
	fprintf(fpw,"  LD  gr6, SIX\n");
	fprintf(fpw,"  ST  gr0, INTBUF,gr6\n");
	fprintf(fpw,"  SUBA  gr6, ONE\n");
	fprintf(fpw,"  CPA  gr1, gr0\n");
	fprintf(fpw,"  JNZ  WI2\n");
	fprintf(fpw,"  LD  gr4, ZERO\n");
	fprintf(fpw,"  ST  gr4, INTBUF,gr6\n");
	fprintf(fpw,"  JUMP  WI5\n");
	fprintf(fpw,"WI2\n");
	fprintf(fpw,"  CPA  gr1, gr0\n");
	fprintf(fpw,"  JZE  WI3\n");
	fprintf(fpw,"  LD  gr5, gr1\n");
	fprintf(fpw,"  DIVA  gr1, TEN\n");
	fprintf(fpw,"  LD  gr4, gr1\n");
	fprintf(fpw,"  MULA  gr4, TEN\n");
	fprintf(fpw,"  SUBA  gr5, gr4\n");
	fprintf(fpw,"  ADDA  gr5, ZERO\n");
	fprintf(fpw,"  ST  gr5, INTBUF,gr6\n");
	fprintf(fpw,"  SUBA  gr6, ONE\n");
	fprintf(fpw,"  JUMP  WI2\n");
	fprintf(fpw,"WI3\n");
	fprintf(fpw,"  CPA  gr7, gr0\n");
	fprintf(fpw,"  JZE  WI4\n");
	fprintf(fpw,"  LD  gr4, MINUS \n");
	fprintf(fpw,"  ST  gr4, INTBUF,gr6\n");
	fprintf(fpw,"  JUMP  WI5\n");
	fprintf(fpw,"WI4\n");
	fprintf(fpw,"  ADDA  gr6, ONE\n");
	fprintf(fpw,"WI5\n");
	fprintf(fpw,"  LAD  gr1, INTBUF,gr6\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"WI6\n");
	fprintf(fpw,"  LAD  gr1, MMINT\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"MMINT    DC  '-32768'\n");
	fprintf(fpw,"WRITEBOOL\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  CPA  gr1, gr0\n");
	fprintf(fpw,"  JZE  WB1\n");
	fprintf(fpw,"  LAD  gr1, WBTRUE\n");
	fprintf(fpw,"  JUMP  WB2\n");
	fprintf(fpw,"WB1\n");
	fprintf(fpw,"  LAD  gr1, WBFALSE\n");
	fprintf(fpw,"WB2\n");
	fprintf(fpw,"  CALL  WRITESTR\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"WBTRUE    DC  'TRUE'\n");
	fprintf(fpw,"WBFALSE    DC  'FALSE'\n");
	fprintf(fpw,"WRITELINE\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD  gr7, OBUFSIZE\n");
	fprintf(fpw,"  LD  gr6, NEWLINE\n");
	fprintf(fpw,"  ST  gr6, OBUF,gr7\n");
	fprintf(fpw,"  ADDA  gr7, ONE\n");
	fprintf(fpw,"  ST  gr7, OBUFSIZE\n");
	fprintf(fpw,"  OUT  OBUF, OBUFSIZE\n");
	fprintf(fpw,"  ST  gr0, OBUFSIZE\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"FLUSH\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD gr7, OBUFSIZE\n");
	fprintf(fpw,"  JZE FL1\n");
	fprintf(fpw,"  CALL WRITELINE\n");
	fprintf(fpw,"FL1\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"READCHAR\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"  LD  gr5, RPBBUF\n");
	fprintf(fpw,"  JZE  RC0\n");
	fprintf(fpw,"  ST  gr5, 0,gr1\n");
	fprintf(fpw,"  ST  gr0, RPBBUF\n");
	fprintf(fpw,"  JUMP  RC3\n");
	fprintf(fpw,"RC0\n");
	fprintf(fpw,"  LD  gr7, INP\n");
	fprintf(fpw,"  LD  gr6, IBUFSIZE\n");
	fprintf(fpw,"  JNZ  RC1\n");
	fprintf(fpw,"  IN  IBUF, IBUFSIZE\n");
	fprintf(fpw,"  LD  gr7, gr0\n");
	fprintf(fpw,"RC1\n");
	fprintf(fpw,"  CPA  gr7, IBUFSIZE\n");
	fprintf(fpw,"  JNZ  RC2\n");
	fprintf(fpw,"  LD  gr5, NEWLINE\n");
	fprintf(fpw,"  ST  gr5, 0,gr1\n");
	fprintf(fpw,"  ST  gr0, IBUFSIZE\n");
	fprintf(fpw,"  ST  gr0, INP\n");
	fprintf(fpw,"  JUMP  RC3\n");
	fprintf(fpw,"RC2\n");
	fprintf(fpw,"  LD  gr5, IBUF,gr7\n");
	fprintf(fpw,"  ADDA  gr7, ONE\n");
	fprintf(fpw,"  ST  gr5, 0,gr1\n");
	fprintf(fpw,"  ST  gr7, INP\n");
	fprintf(fpw,"RC3\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"READINT\n");
	fprintf(fpw,"  RPUSH\n");
	fprintf(fpw,"RI1\n");
	fprintf(fpw,"  CALL  READCHAR\n");
	fprintf(fpw,"  LD  gr7, 0,gr1\n");
	fprintf(fpw,"  CPA  gr7, SPACE\n");
	fprintf(fpw,"  JZE  RI1\n");
	fprintf(fpw,"  CPA  gr7, TAB\n");
	fprintf(fpw,"  JZE  RI1\n");
	fprintf(fpw,"  CPA  gr7, NEWLINE\n");
	fprintf(fpw,"  JZE  RI1\n");
	fprintf(fpw,"  LD  gr5, ONE\n");
	fprintf(fpw,"  CPA  gr7, MINUS\n");
	fprintf(fpw,"  JNZ  RI4\n");
	fprintf(fpw,"  LD  gr5, gr0\n");
	fprintf(fpw,"  CALL  READCHAR\n");
	fprintf(fpw,"  LD  gr7, 0,gr1\n");
	fprintf(fpw,"RI4\n");
	fprintf(fpw,"  LD  gr6, gr0\n");
	fprintf(fpw,"RI2\n");
	fprintf(fpw,"  CPA  gr7, ZERO\n");
	fprintf(fpw,"  JMI  RI3\n");
	fprintf(fpw,"  CPA  gr7, NINE\n");
	fprintf(fpw,"  JPL  RI3\n");
	fprintf(fpw,"  MULA  gr6, TEN\n");
	fprintf(fpw,"  ADDA  gr6, gr7\n");
	fprintf(fpw,"  SUBA  gr6, ZERO\n");
	fprintf(fpw,"  CALL  READCHAR\n");
	fprintf(fpw,"  LD  gr7, 0,gr1\n");
	fprintf(fpw,"  JUMP  RI2\n");
	fprintf(fpw,"RI3\n");
	fprintf(fpw,"  ST  gr7, RPBBUF\n");
	fprintf(fpw,"  ST  gr6, 0,gr1\n");
	fprintf(fpw,"  CPA  gr5, gr0\n");
	fprintf(fpw,"  JNZ  RI5\n");
	fprintf(fpw,"  SUBA  gr5, gr6\n");
	fprintf(fpw,"  ST  gr5, 0,gr1\n");
	fprintf(fpw,"RI5\n");
	fprintf(fpw,"  RPOP\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"READLINE\n");
	fprintf(fpw,"  ST  gr0, IBUFSIZE\n");
	fprintf(fpw,"  ST  gr0, INP\n");
	fprintf(fpw,"  ST  gr0, RPBBUF\n");
	fprintf(fpw,"  RET\n");
	fprintf(fpw,"ONE    DC  1\n");
	fprintf(fpw,"SIX    DC  6\n");
	fprintf(fpw,"TEN    DC  10\n");
	fprintf(fpw,"SPACE    DC  #0020\n");
	fprintf(fpw,"MINUS    DC  #002D\n");
	fprintf(fpw,"TAB    DC  #0009\n");
	fprintf(fpw,"ZERO    DC  #0030\n");
	fprintf(fpw,"NINE    DC  #0039\n");
	fprintf(fpw,"NEWLINE    DC  #000A\n");
	fprintf(fpw,"INTBUF    DS  8\n");
	fprintf(fpw,"OBUFSIZE  DC  0\n");
	fprintf(fpw,"IBUFSIZE  DC  0\n");
	fprintf(fpw,"INP    DC  0\n");
	fprintf(fpw,"OBUF    DS  257\n");
	fprintf(fpw,"IBUF    DS  257\n");
	fprintf(fpw,"RPBBUF    DC  0\n");
	fprintf(fpw,"	END\n");
}

