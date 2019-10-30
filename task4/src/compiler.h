/*
 * compiler.h
 *
 *  Created on: 2017/10/09
 *      Author: ryosuke
 */

#ifndef COMPILER_H_
#define COMPILER_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRSIZE 1024

/* Token */
#define	TNAME		1	/* Name : Alphabet { Alphabet | Digit } */
#define	TPROGRAM	2	/* program : Keyword */
#define	TVAR		3	/* var : Keyword */
#define	TARRAY		4	/* array : Keyword */
#define	TOF		5	/* of : Keyword */
#define	TBEGIN		6	/* begin : Keyword */
#define	TEND		7  	/* end : Keyword */
#define	TIF		8  	/* if : Keyword */
#define	TTHEN		9	/* then : Keyword */
#define	TELSE		10	/* else : Keyword */
#define	TPROCEDURE	11	/* procedure : Keyword */
#define	TRETURN		12	/* return : Keyword */
#define	TCALL		13	/* call : Keyword */
#define	TWHILE		14	/* while : Keyword */
#define	TDO		15 	/* do : Keyword */
#define	TNOT		16	/* not : Keyword */
#define	TOR		17	/* or : Keyword */
#define	TDIV		18 	/* div : Keyword */
#define	TAND		19 	/* and : Keyword */
#define	TCHAR		20	/* char : Keyword */
#define	TINTEGER	21	/* integer : Keyword */
#define	TBOOLEAN	22 	/* boolean : Keyword */
#define	TREADLN		23	/* readln : Keyword */
#define	TWRITELN	24	/* writeln : Keyword */
#define	TTRUE		25	/* true : Keyword */
#define	TFALSE		26	/* false : Keyword */
#define	TNUMBER		27	/* unsigned integer */
#define	TSTRING		28	/* String */
#define	TPLUS		29	/* + : symbol */
#define	TMINUS		30 	/* - : symbol */
#define	TSTAR		31 	/* * : symbol */
#define	TEQUAL		32 	/* = : symbol */
#define	TNOTEQ		33 	/* <> : symbol */
#define	TLE		34 	/* < : symbol */
#define	TLEEQ		35 	/* <= : symbol */
#define	TGR		36	/* > : symbol */
#define	TGREQ		37	/* >= : symbol */
#define	TLPAREN		38 	/* ( : symbol */
#define	TRPAREN		39 	/* ) : symbol */
#define	TLSQPAREN	40	/* [ : symbol */
#define	TRSQPAREN	41 	/* ] : symbol */
#define	TASSIGN		42	/* := : symbol */
#define	TDOT		43 	/* . : symbol */
#define	TCOMMA		44	/* , : symbol */
#define	TCOLON		45	/* : : symbol */
#define	TSEMI		46	/* ; : symbol */
#define	TREAD		47	/* read : Keyword */
#define	TWRITE		48	/* write : Keyword */
#define	TBREAK		49	/* break : Keyword */

/* type */
#define TPINT 1 /* integer */
#define TPCHAR 2 /* char */
#define TPBOOL 3 /* boolean */
#define TPARRAY 4 /* array */
#define TPPROC 5 /* procedure */

#define NUMOFTYPE 5


#define NORMAL 0
#define ERROR 1


#define NUMOFTOKEN	49

#define MAXMINUS 140
#define MAXSPACE1 30
#define MAXSPACE2 50

extern int token;
extern int typenum;
extern int paraflag;
extern int arraynum;
extern int arraytype;
extern FILE *fpw;
/* token-list.c */

#define KEYWORDSIZE	28

extern struct KEY {
	char * keyword;
	int keytoken;
} key[KEYWORDSIZE];

extern void error(char *mes);

/* scan.c */
extern int init_scan(char *filename);
extern int scan(void);
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int get_linenum(void);
extern void end_scan(void);

/* pretty printer.c */
extern int parse_program();
extern int block(int label);
extern int var_decl();
extern int var_names();
extern int type();
extern int st_type();
extern int ar_type();
extern int sub_decl();
extern int form_para();
extern int fukugou(int label);
extern int statement(int label);
extern int bunki(int label);
extern int kurikaeshi(int label);
extern int call_st(int label);
extern int exp_narabi();
extern int dainyu(int label);
extern int var();
extern int shiki();
extern int simple();
extern int kou();
extern int inshi();
extern int input_st(int label);
extern int output_st(int label);
extern int shitei();
extern int get_inlabel();
extern void lib();

extern void init_idtab();
extern struct ID *search_globalidtab(char *np);
extern struct ID *search_localidtab(char *np);
extern int globalid_def();
extern int procedure_def();
extern int globalid_ref(char *np);
extern int localid_def();
extern int localid_ref(char *np);
extern int type_mem(struct ID *p);
extern int joint_localtoglobal();
extern void release_idtab();
struct TYPE{
	int ttype;
	int arraysize;
	struct TYPE *etp;
	struct TYPE *paratp;
};

extern struct TYPE *paratype;

struct LINE{
	int reflinenum;
	struct LINE *nextlinep;
};

extern struct ID {
	char *name;
	char *procname;
	struct TYPE *itp;
	int ispara;
	int deflinenum;
	struct LINE *irefp;
	struct ID *left, *right;
}  *globalidroot, *localidroot;

extern struct NAME{
	char *name;
	struct NAME *next;
} *names;

extern char *procname;
extern struct ID *searchp;
#endif /* COMPILER_H_ */
