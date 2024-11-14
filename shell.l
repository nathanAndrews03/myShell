
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%x src

%{


#include <cstring>
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

int is_current_buffer_null() {
    return YY_CURRENT_BUFFER == NULL;
}

%}

%option noyywrap



%%

"#".*\n {
  /* Discard comments starting with # */
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

">>" {
  return GREATGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"&" {
  return AMPERSAND;
}
"2>" {
  return TWOGREAT;
}

"[" {
  return LBRACKET;
}

"]" {
  return RBRACKET;
}

"if" {
  return IF;
}

"then" {
  return THEN;
}

"fi" {
  return FI;
}

"while" {
  return WHILE;
}

"do" {
  return DO;
}

"done" {
  return DONE;
}

"for" {
  return FOR;
}

"in" {
  return IN;
}

";" {
  return SEMI;
}

"exit" {
  return EXIT;
}

"source"[ ]+ {
	BEGIN(src);
}

<src>{
	[^ \t\n]+ {
		FILE *fp = fopen(yytext, "r");
		if (!fp) {
            		perror("source");
			exit(1);
		}
		YY_BUFFER_STATE newBuf = yy_create_buffer(fp, YY_BUF_SIZE);
        	yypush_buffer_state(newBuf);
        	BEGIN(INITIAL);
	}
}

<<EOF>> {
    yypop_buffer_state();
    if (!YY_CURRENT_BUFFER) {
        yyterminate();
    }
}



\$\([^\n)]*\)|`[^\n]*` {
	yylval.cpp_string = new std::string(yytext);
	return WORD;
}

\"[^\"\n]*\" {
	yylval.cpp_string = new std::string(yytext + 1, yyleng - 2);
	//printf("quo yytext=%s\n", yytext);
	return WORD;
}

~[^ \n\t]* {
	std::string str = std::string(yytext);
	if (str.size() == 1) {
		yylval.cpp_string = new std::string(getenv("HOME"));
	} else {
		str = std::string("/homes/") + str.substr(1, -1);
		yylval.cpp_string = new std::string(str);
	}
	return WORD;
}


 [^ \n\t;]*\\[^ \t\n]* {                                      
  /* Assume that file names have only alpha chars */
  //printf("yyleng=%d    len=%zu\n", yyleng, strlen(yytext));
  char *esc = (char *) malloc(yyleng +1);
  char *tmp = yytext;
  int i = 0;
  //printf("yytext=%s    tmp=%s\n", yytext, tmp);
  while (*tmp) {
	if (*tmp == '\\') {
		esc[i] = *(++tmp);
		i++;
	} else {
		esc[i] = *tmp;
		i++;
	}
	tmp++;
  }
  esc[i] = '\0';
  yylval.cpp_string = new std::string(esc);
  return WORD;            
}

[^ \n\t;|<>][^ \n\t;|<>]* {
  yylval.cpp_string = new std::string(yytext);
  //printf("base yytext=%s\n", yytext);
  return WORD;
}



