/*
 * scan.c
 *
 *  Created on: 2017/10/09
 *      Author: ryosuke
 */

#include "token-list.h"
#include <ctype.h>


int keyword_search(char *string);

int cbuf, line_cnt, newline;
FILE *fp;

int init_scan(char *filename){ /* Initialize */
	line_cnt = 0;
	newline = 0;

	if((fp = fopen(filename, "r")) == NULL){
		return -1;
	}

	init_idtab();
	cbuf = fgetc(fp);

	return 0;
}

int keyword_search(char *string){ /* Search keywords */
	int l = 0;
	int r = KEYWORDSIZE - 1;
	int c, comp;

	while(l <= r){
		c = (l + r) >> 1;
		comp = strcmp(string, key[c].keyword);
		if(comp < 0){
			r = c - 1;
		}
		else if(comp > 0){
			l = c + 1;
		}
		else{
			return key[c].keytoken;
		}
	}

	id_countup(string);
	return TNAME;
}

int scan(){
	int i = 0;
	int num_attr;
	char string_attr[MAXSTRSIZE];

	memset(string_attr, '\0', sizeof(string_attr));




	while(cbuf != EOF){
		if(newline == 0){
			line_cnt++;
			newline = 1;
		}
		if(isalpha(cbuf)){
			string_attr[i++] = cbuf;
			cbuf=fgetc(fp);
			while(isalnum(cbuf)){
				if(i >= MAXSTRSIZE){
					error("Illegal name!\n");
					return -1;
				}
				string_attr[i++] = cbuf;
				cbuf=fgetc(fp);
			}
			return keyword_search(string_attr);
		}
		else if(isdigit(cbuf)){
			string_attr[i++] = cbuf;
			cbuf = fgetc(fp);
			while(isdigit(cbuf)){
				if(i >= MAXSTRSIZE){
					error("Illegal number!\n");
					return -1;
				}
				string_attr[i++] = cbuf;
				cbuf = fgetc(fp);
			}
			num_attr = atoi(string_attr);
			if(num_attr > 32767){
				error("Illegal number!\n");
				return -1;
			}
			return TNUMBER;
		}

		switch(cbuf){
		case '\'':
			cbuf = fgetc(fp);
			while(1){
				if(i >= MAXSTRSIZE){
					error("Illegal string!\n");
					return -1;
				}
				if(isprint(cbuf)){
					if(cbuf == '\''){
						cbuf = fgetc(fp);
						if(cbuf == '\''){
							string_attr[i++] = (char)cbuf;
							string_attr[i++] = (char)cbuf;
							cbuf = fgetc(fp);
						}
						else{
							return TSTRING;
						}
					}
					else{
						string_attr[i++] = (char)cbuf;
						cbuf = fgetc(fp);
					}
				}else{
					error("Illegal string.\n");
					return -1;
				}
			}
			break;
		case ' ':
			cbuf = fgetc(fp);
			break;
		case '\t':
			cbuf = fgetc(fp);
			break;
		case '\n':
			cbuf = fgetc(fp);
			newline = 0;
			if(cbuf == '\r'){
				cbuf = fgetc(fp);
			}
			break;
		case '\r':
			cbuf = fgetc(fp);
			newline = 0;
			if(cbuf == '\n'){
				cbuf = fgetc(fp);
			}
			break;

		case '{':
			cbuf = fgetc(fp);
			while((cbuf != '}') && (cbuf != EOF)){
				if(newline == 0){
					line_cnt++;
					newline = 1;
				}
				if(cbuf == '\n'){
					cbuf = fgetc(fp);
					newline = 0;
					if(cbuf == '\r'){
						cbuf = fgetc(fp);
					}
					continue;
				}
				else if(cbuf == '\r'){
					cbuf = fgetc(fp);
					newline = 0;
					if(cbuf == '\n'){
						cbuf = fgetc(fp);
					}
					continue;
				}
				else{
					cbuf = fgetc(fp);
				}
			}
			if(cbuf == EOF){
				error("Comment Error!\n");
				return -1;
			}
			cbuf = fgetc(fp);
			break;
		case '/':
			cbuf = fgetc(fp);
			if(cbuf == '*'){
				do{
					do{
						if(cbuf == '\n'){
							cbuf = fgetc(fp);
							newline = 0;;
							if(cbuf == '\r'){
								cbuf = fgetc(fp);
							}
						}
						else if(cbuf == '\r'){
							cbuf = fgetc(fp);
							newline=0;
							if(cbuf == '\n'){
								cbuf = fgetc(fp);
							}
						}
						else{
							cbuf = fgetc(fp);
						}
						if(cbuf == EOF){
							error("Comment Error!\n");
							return -1;
						}

						if(newline == 0){
							line_cnt++;
							newline = 1;
						}
					}while(cbuf != '*');
					cbuf = fgetc(fp);
				}while(cbuf != '/');
				cbuf = fgetc(fp);
				break;
			}
			else{
				error("Illegal \'/\' position.\n");
				return -1;
			}
		case '+':
			cbuf = fgetc(fp);
			return TPLUS;
		case '-':
			cbuf = fgetc(fp);
			return TMINUS;
		case '*':
			cbuf = fgetc(fp);
			return TSTAR;
		case '=':
			cbuf = fgetc(fp);
			return TEQUAL;
		case '<':
			cbuf = fgetc(fp);
			if(cbuf == '>'){
				cbuf = fgetc(fp);
				return TNOTEQ;
			}
			else if(cbuf == '='){
				cbuf = fgetc(fp);
				return TLEEQ;
			}
			else{
				return TLE;
			}
		case '>':
			cbuf = fgetc(fp);
			if(cbuf == '='){
				cbuf = fgetc(fp);
				return TGREQ;
			}
			else{
				return TGR;
			}
		case '(':
			cbuf = fgetc(fp);
			return TLPAREN;
		case ')':
			cbuf = fgetc(fp);
			return TRPAREN;
		case '[':
			cbuf = fgetc(fp);
			return TLSQPAREN;
		case ']':
			cbuf = fgetc(fp);
			return TRSQPAREN;
		case ':':
			cbuf = fgetc(fp);
			if(cbuf == '='){
				cbuf = fgetc(fp);
				return TASSIGN;
			}
			else{
				return TCOLON;
			}
		case '.':
			cbuf = fgetc(fp);
			return TDOT;
		case ',':
			cbuf=fgetc(fp);
			return TCOMMA;
		case ';':
			cbuf = fgetc(fp);
			return TSEMI;

		default:
			error("Illegal character.\n");
			return -1;
		}
	}
	return -1;
}

int get_linenum(){
	return line_cnt;
}

void end_scan(){
	fclose(fp);
}


