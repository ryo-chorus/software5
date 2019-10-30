/*
 * prettyprinter.c
 *
 *  Created on: 2017/10/30
 *      Author: b5122013
 */

#include "compiler.h"

#define NORMAL 0
#define ERROR 1

int indentnum = 0;
char *token_str[NUMOFTOKEN+1] = {
		"", "",
		"program", "var", "array", "of", "begin", "end", "if", "then",
		"else", "procedure", "return", "call", "while", "do", "not", "or",
		"div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
		"false", "", "", "+", "-", "*", "=", "<>", "<", "<=", ">",
		">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read","write", "break"
};
void indent();
int parse_program(){  /* program */
	if(token != TPROGRAM){
		error("Keyword 'program' is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(token != TNAME){
		error("Program name is not found!");
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	if(block() == ERROR) return ERROR;
	if(token != TDOT){
		error("Period is not found at the end of program!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	return NORMAL;
}

int block(){       /* block */
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
	indentnum = 0;
	if(fukugou() == ERROR){
		return ERROR;
	}
	return NORMAL;
}

int var_decl(){     /* variable declaration */
	indentnum = 1;
	indent();
	printf("%s ", token_str[token]);
	token = scan();
	if(var_names() == ERROR){
		return ERROR;
	}
	if(token != TCOLON){
		error("Colon is not found!");
		return ERROR;
	}
	printf(" %s ", token_str[token]);
	token = scan();
	if(type() == ERROR){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	while(token == TNAME){
		indentnum = 2;
		indent();
		if(var_names() == ERROR){
			return ERROR;
		}
		if(token != TCOLON){
			error("Colon is not found!");
			return ERROR;
		}
		printf(" %s ", token_str[token]);
		token = scan();
		if(type() == ERROR){
			return ERROR;
		}
		if(token != TSEMI){
			error("Semicolon is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		putchar('\n');
		token = scan();
	}

	return NORMAL;
}

int var_names(){    /* variable names */
	if(token != TNAME){
		error("Variable name is not found!");
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	while(token == TCOMMA){
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(token != TNAME){
			error("Variable name is not found after comma!");
			return ERROR;
		}
		printf("%s", string_attr);
		token = scan();
	}
	return NORMAL;
}

int type(){     /* type */
	if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		printf("%s", token_str[token]);
		token = scan();
		return NORMAL;
	}
	else if(ar_type() == NORMAL){
		return NORMAL;
	}
	else{
		error("Illegal type!");
		return ERROR;
	}
}


int ar_type(){   /* array type */
	if(token != TARRAY){
		return ERROR;
	}
	printf("%s", token_str[token]);
	token = scan();
	if(token != TLSQPAREN){
		return ERROR;
	}
	printf("%s", token_str[token]);
	token = scan();
	if(token != TNUMBER){
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	if(token != TRSQPAREN){
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(token != TOF){
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(token != TINTEGER && token != TBOOLEAN && token != TCHAR){
		return ERROR;
	}
	printf("%s", token_str[token]);
	token = scan();
	return NORMAL;
}

int sub_decl(){   /* subprogram declaration */
	indentnum = 1;
	indent();
	printf("%s ", token_str[token]);
	token = scan();
	if(token != TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	if(token == TLPAREN){
		if(form_para()==ERROR){
			return ERROR;
		}
	}

	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	if(token == TVAR){
		if(var_decl() == ERROR){
			return ERROR;
		}
	}

	indent();
	if(fukugou() == ERROR){
		return ERROR;
	}
	if(token != TSEMI){
		error("Semicolon is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	return NORMAL;
}

int form_para(){	/* formal parameters */
	printf("%s", token_str[token]);
	token = scan();
	if(var_names() == ERROR){
		return ERROR;
	}
	if(token != TCOLON){
		error("Colon is not found!");
		return ERROR;
	}
	putchar(' ');
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(type() == ERROR){
		return ERROR;
	}
	while(token == TSEMI){
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(var_names() == ERROR){
			return ERROR;
		}
		if(token != TCOLON){
			error("Colon is not found!");
			return ERROR;
		}
		putchar(' ');
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(type() == ERROR){
			return ERROR;
		}
	}
	if(token != TRPAREN){
		error("\')\' is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	token = scan();
	return NORMAL;
}

int fukugou(){	/* compound statement */
	if(token != TBEGIN){
		error("Keyword \"begin\" is not found!");
		return ERROR;
	}
	printf("%s", token_str[token]);
	putchar('\n');
	indentnum++;
	token = scan();
	if(statement() == ERROR){
		return ERROR;
	}
	while(token == TSEMI){
		printf("%s", token_str[token]);
		putchar('\n');
		token = scan();
		if(statement() == ERROR){
			return ERROR;
		}
	}
	putchar('\n');
	if(token != TEND){
		error("Keyword \"end\" is not found!");
		return ERROR;
	}
	indentnum--;
	indent();
	printf("%s", token_str[token]);
	token = scan();
	return NORMAL;
}

int statement(){	/* statement */
	indent();
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
		printf("%s", token_str[token]);
		token = scan();
		return NORMAL;
	}
	else if(token == TCALL){
		if(call_st() == ERROR){
			return ERROR;
		}
	}
	else if(token == TRETURN){
		printf("%s", token_str[token]);
		token = scan();
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
	else{
		if(token != TSEMI && token != TEND){
			putchar('\n');
		}
	}
	return NORMAL;
}

int bunki(){	/* condition statement */
	int flag = 0;
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(shiki() == ERROR){
		return ERROR;
	}
	if(token != TTHEN){
		error("Keyword \"then\" is not found!");
		return ERROR;
	}
	putchar(' ');
	printf("%s", token_str[token]);
	putchar('\n');
	token = scan();
	if(token != TBEGIN){
		indentnum++;
		flag = 1;
	}
	if(statement() == ERROR){
		return ERROR;
	}
	if(flag == 1){
		indentnum--;
		flag = 0;
	}
	if(token == TELSE){
		putchar('\n');
		indent();
		printf("%s", token_str[token]);
		putchar('\n');
		token = scan();
		if(token != TBEGIN){
			indentnum++;
			flag = 1;
		}
		if(statement() == ERROR){
			return ERROR;
		}
		if(flag == 1){
			indentnum--;
			flag = 0;
		}
	}

	return NORMAL;
}

int kurikaeshi(){	/* iteration statement */
	int flag = 0;
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(shiki() == ERROR){
		return ERROR;
	}
	if(token != TDO){
		error("Keyword \"do\" is not found!");
		return ERROR;
	}
	putchar(' ');
	printf("%s", token_str[token]);
	putchar('\n');
	token =scan();
	if(token != TBEGIN){
		indentnum++;
		flag = 1;
	}
	if(statement() == ERROR){
		return ERROR;
	}
	if(flag == 1){
		indentnum--;
		flag = 0;
	}

	return NORMAL;
}

int call_st(){	/* call statement */
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(token !=TNAME){
		error("Procedure name is not found!");
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	if(token == TLPAREN){
		printf("%s", token_str[token]);
		token = scan();
		if(exp_narabi() == ERROR){
			return ERROR;
		}
		if(token != TRPAREN){
			error("\")\" is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
	}

	return NORMAL;
}

int exp_narabi(){	/* expressions */
	if(shiki() == ERROR){
		return ERROR;
	}
	while(token == TCOMMA){
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(shiki() == ERROR){
			return ERROR;
		}
	}

	return NORMAL;
}

int dainyu(){	/* assignment statement */
	if(var() == ERROR){
		return ERROR;
	}
	if(token != TASSIGN){
		error("\":=\" is not found!");
		return ERROR;
	}
	putchar(' ');
	printf("%s", token_str[token]);
	putchar(' ');
	token = scan();
	if(shiki() == ERROR){
		return ERROR;
	}

	return NORMAL;
}

int var(){	/* variable */
	if(token != TNAME){
		error("Variable name is not found!");
		return ERROR;
	}
	printf("%s", string_attr);
	token = scan();
	if(token == TLSQPAREN){
		printf("%s", token_str[token]);
		token = scan();
		if(shiki() == ERROR){
			return ERROR;
		}
		if(token != TRSQPAREN){
			error("\"]\" is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
	}

	return NORMAL;
}

int shiki(){	/* expression */
	if(simple() == ERROR){
		return ERROR;
	}
	while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
		putchar(' ');
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(simple() == ERROR){
			return ERROR;
		}
	}

	return NORMAL;
}

int simple(){	/* simple expression */
	if(token == TPLUS || token == TMINUS){
		printf("%s", token_str[token]);
		token = scan();
	}
	if(kou() == ERROR){
		return ERROR;
	}
	while(token == TPLUS || token == TMINUS || token == TOR){
		putchar(' ');
		printf("%s",token_str[token]);
		putchar(' ');
		token = scan();
		if(kou() == ERROR){
			return ERROR;
		}
	}

	return NORMAL;
}

int kou(){	/* term */
	if(inshi() == ERROR){
		return ERROR;
	}
	while(token == TSTAR || token == TAND || token == TDIV) {
		putchar(' ');
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(inshi() == ERROR){
			return ERROR;
		}
	}
	return NORMAL;
}

int inshi(){	/* factor */
	if(token == TNAME){
		if(var() == ERROR){
			return ERROR;
		}
	}
	else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
		if(token == TNUMBER){
			printf("%d", num_attr);
			token = scan();
		}
		else if(token == TFALSE){
			printf("%s", token_str[token]);
			token = scan();
		}
		else if(token == TTRUE){
			printf("%s", token_str[token]);
			token = scan();
		}
		else if(token == TSTRING){
			putchar('\'');
			printf("%s", string_attr);
			putchar('\'');
			token = scan();
		}
	}
	else if(token == TLPAREN){
		printf("%s", token_str[token]);
		token = scan();
		if(shiki() == ERROR){
			return ERROR;
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
	}
	else if(token == TNOT){
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(inshi() == ERROR){
			return ERROR;
		}
	}
	else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
		if(token != TLPAREN){
			error("\'(\' is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
		if(shiki() == ERROR){
			return ERROR;
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		putchar(' ');
		token = scan();
	}
	else{
		error("Illegal factor!");
		return ERROR;
	}
	return NORMAL;
}

int input_st(){	/* input statement */
	printf("%s", token_str[token]);
	token = scan();
	if(token == TLPAREN){
		printf("%s", token_str[token]);
		token = scan();
		if(var() == ERROR){
			return ERROR;
		}
		while(token == TCOMMA){
			printf("%s", token_str[token]);
			putchar(' ');
			token = scan();
			if(var() == ERROR){
				return ERROR;
			}
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
	}

	return NORMAL;
}

int output_st(){	/* output statement */
	printf("%s", token_str[token]);
	token = scan();
	if(token == TLPAREN){
		printf("%s", token_str[token]);
		token = scan();
		if(shitei() == ERROR){
			return ERROR;
		}
		while(token == TCOMMA){
			printf("%s", token_str[token]);
			putchar(' ');
			token = scan();
			if(shitei() == ERROR){
				return ERROR;
			}
		}
		if(token != TRPAREN){
			error("\')\' is not found!");
			return ERROR;
		}
		printf("%s", token_str[token]);
		token = scan();
	}

	return NORMAL;

}

int shitei(){	/* output format */
	if(token == TSTRING && strlen(string_attr) != 1){
		putchar('\'');
		printf("%s", string_attr);
		putchar('\'');
		token = scan();
		return NORMAL;
	}
	else if(token == TPLUS || token == TMINUS || token == TNAME || token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING || token == TLPAREN || token == TNOT || token == TINTEGER || token == TBOOLEAN || token == TCHAR){
		if(shiki() == ERROR){
			return ERROR;
		}
		if(token == TCOLON){
			printf("%s",token_str[token]);
			token = scan();
			if(token == TNUMBER){
				printf("%d", num_attr);
				token = scan();
			}
		}
		return NORMAL;
	}


	error("Illegal output format!");
	return ERROR;
}

void indent(){
	int i;
	for(i = 0; i < indentnum; i++){
		printf("    ");
	}
}
