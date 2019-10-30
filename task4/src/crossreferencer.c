/*
 * crossreferencer.c
 *
 *  Created on: 2017/12/04
 *      Author: b5122013
 */

#include "compiler.h"


struct ID  *globalidroot, *localidroot;
/* string of type */
char *typename[NUMOFTYPE + 1] = {
		"",
		"integer", "char", "boolean", "array", "procedure"
};

void init_idtab() {		/* Initialise the table */
	globalidroot = NULL;
	localidroot = NULL;
}

struct ID *search_globalidtab(char *np) {
	struct ID *p;

	p = globalidroot;
	while(p != NULL) {
		if(strcasecmp(np, p->name) == 0){
			if(p->procname == NULL){
				return(p);
			}
			else{
				p = p->left;
			}
		}
		else if(strcasecmp(np, p->name) < 0){
			p = p->left;
		}
		else{
			p = p->right;
		}
	}
	return(NULL);
}

struct ID *search_localidtab(char *np) {
	struct ID *p;

	p = localidroot;
	while(p != NULL) {
		if(strcasecmp(np, p->name) == 0) return(p);
		else if(strcasecmp(np, p->name) < 0){
			p = p->left;
		}
		else{
			p = p->right;
		}
	}
	return(NULL);
}



int globalid_def() {	/* Register the global variable */
	struct ID **p, *new;
	struct NAME *np, *nq;

	for(np = names; np != NULL; np = nq){
		p = &globalidroot;
		while(*p != NULL){
			if(strcasecmp(np->name, (*p)->name) == 0){
				if((*p)->procname == NULL){
					error("Multiple defined!");
					return ERROR;
				}
				else{
					p = &(*p)->left;
				}
			}
			else if(strcasecmp(np->name, (*p)->name) < 0){
				p = &(*p)->left;
			}
			else{
				p = &(*p)->right;
			}
		}
		if((new = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
			error("can not malloc in globalid_def!");
			return ERROR;
		}
		new->name = np->name;
		type_mem(new);
		new->deflinenum = get_linenum();
		new->right = NULL;
		new->left = NULL;
		nq = np->next;
		free(np);
		*p = new;
	}

	return NORMAL;
}

int procedure_def() {	/* Register procedure */
	struct ID **p, *new;
	char *cp;

	p = &globalidroot;

	while(*p != NULL){
		if(strcasecmp(procname, (*p)->name) == 0){
			if((*p)->procname == NULL){
				error("Multiple defined!");
				return ERROR;
			}
			else{
				p = &(*p)->left;
			}
		}
		else if(strcasecmp(procname, (*p)->name) < 0){
			p = &(*p)->left;
		}
		else{
			p = &(*p)->right;
		}
	}
	if((new = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
		error("can not malloc in globalid_def!");
		return ERROR;
	}
	if((cp = (char *)malloc(strlen(procname)+1)) == NULL) {
		error("can not malloc-2 in procedure_def!");
		return ERROR;
	}
	strcpy(cp, procname);
	new->name = cp;
	typenum = TPPROC;
	type_mem(new);
	new->deflinenum = get_linenum();
	new->right = NULL;
	new->left = NULL;
	*p = new;

	return NORMAL;
}

int globalid_ref(char *np) {	/* Remember where the global variable appears. */
	struct ID *p;
	struct LINE *next, *prev, *m;


	if((p = search_globalidtab(np)) != NULL){
		searchp = p;
		if((m = (struct LINE *)malloc(sizeof(struct LINE))) == NULL){
			error("can not malloc in globalid_ref\n");
			return ERROR;
		}
		m->reflinenum = get_linenum();
		m->nextlinep = NULL;
		if(p->irefp == NULL){
			p->irefp = m;
		}
		else{
			for(next = p->irefp; next != NULL; next = next->nextlinep){
				prev = next;
			}
			prev->nextlinep = m;

		}
	}
	else {
		error("The name is undefined!");
		return ERROR;
	}

	return NORMAL;
}

int localid_def() {	/* Register local variable or formal parameter */
	struct ID **p, *new;
	struct NAME *np, *nq;
	char *cp;

	for(np = names; np != NULL; np = nq){
		p = &localidroot;
		while(*p != NULL){
			if(strcasecmp(np->name, (*p)->name) == 0){
				if((*p)->procname == NULL){
					error("Multiple defined!");
					return ERROR;
				}
				else{
					p = &(*p)->left;
				}
			}
			else if(strcasecmp(np->name, (*p)->name) < 0){
				p = &(*p)->left;
			}
			else{
				p = &(*p)->right;
			}
		}
		if((new = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
			error("can not malloc in localid_def!");
			return ERROR;
		}
		if((cp = (char *)malloc(strlen(procname)+1)) == NULL) {
			error("can not malloc-2 in localid_def!");
			return ERROR;
		}
		strcpy(cp, procname);
		new->name = np->name;
		new->procname = cp;
		type_mem(new);
		new->ispara = paraflag;
		new->deflinenum = get_linenum();
		nq = np->next;
		if(paraflag != 1){
			free(np);
		}
		new->right = NULL;
		new->left = NULL;
		*p = new;
	}

	return NORMAL;
}

int localid_ref(char *np) {	/* Remember where the formal parameter or the local variable appears. */
	struct ID *p;
	struct LINE *next, *prev, *m;


	if((p = search_localidtab(np)) != NULL){
		searchp = p;
		if((m = (struct LINE *)malloc(sizeof(struct LINE))) == NULL){
			error("can not malloc in localid_ref!");
			return ERROR;
		}
		m->reflinenum = get_linenum();
		m->nextlinep = NULL;
		if(p->irefp == NULL){
			p->irefp = m;
		}
		else{
			for(next = p->irefp; next != NULL; next = next->nextlinep){
				prev = next;
			}
			prev->nextlinep = m;

		}
	}
	else {
		if((p = search_globalidtab(np)) != NULL){
			searchp = p;
			if(p->procname == NULL){
				if((m = (struct LINE *)malloc(sizeof(struct LINE))) == NULL){
					error("can not malloc-2 in localid_ref!");
					return ERROR;
				}
				m->reflinenum = get_linenum();
				m->nextlinep = NULL;
				if(p->irefp == NULL){
					p->irefp = m;
				}
				else{
					for(next = p->irefp; next != NULL; next = next->nextlinep){
						prev = next;
					}
					prev->nextlinep = m;

				}
			}
		}
		else{
			error("The name is undefined!");
			return ERROR;
		}
	}

	return NORMAL;
}

int type_mem(struct ID *p){ /* Register the type */
	struct TYPE *q, *r;
	struct TYPE *pt;

	if((q = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL){
		error("can not malloc in type_mem!");
		return ERROR;
	}
	q->ttype=typenum;

	p->itp = q;

	if(typenum == TARRAY){
		p->itp->arraysize = arraynum;
		if((r = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
			error("can not malloc-2 in type_mem!");
			return ERROR;
		}
		r->ttype = arraytype;
		p->itp->etp = r;

	}
	else if(typenum == TPPROC){
		for(pt = paratype; pt != NULL; paratype = pt){
			if((r = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
				error("can not malloc-3 in type_mem!");
				return ERROR;
			}
			r->ttype = pt->ttype;
			r->paratp = NULL;
			if(p->itp->paratp == NULL){
				p->itp->paratp = r;
				q = p->itp->paratp;
			}
			else{
				q->paratp = r;
				q = q->paratp;
			}

			pt = pt->paratp;
			free(paratype);
		}
	}
	return NORMAL;
}

int joint_localtoglobal(){ /* Combine lists. */
	struct ID **p, **q,*x;


	while(localidroot != NULL){
		p = &localidroot;
		q = &globalidroot;
		while((*p)->left != NULL){
			p = &(*p)->left;
		}
		x = *p;
		*p = (*p)->right;
		while(*q != NULL){
			if(strcasecmp(x->name, (*q)->name) == 0){
				if((*q)->procname == NULL){
					q = &(*q)->right;
				}
				else {
					if(strcasecmp(x->procname, (*q)->procname) == 0){
						error("Multiple defined!");
						return ERROR;
					}
					else if(strcasecmp(x->procname, (*q)->procname) < 0){
						q = &(*q)->left;
					}
					else{
						q = &(*q)->right;
					}
				}
			}
			else if(strcasecmp(x->name, (*q)->name) < 0){
				q = &(*q)->left;
			}
			else{
				q = &(*q)->right;
			}
		}
		x->right = NULL;
		x->left = NULL;
		*q = x;

	}

	return NORMAL;
}



void release_idtab(struct ID *p) {	/* Release tha data structure */

	if(p == NULL) return;
	release_idtab(p->left);
	release_idtab(p->right);
	free(p);
	init_idtab();
}

