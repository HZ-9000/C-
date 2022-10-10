%{
#include "scanType.h"
#include "AST.h"
#include "parser.tab.h"
#include <string.h>
int line = 1;
int numErrors;
int savedType;
bool isStatic = false;

void upperCase(char **s){
    int len;
    char *temp = strdup(*s);
    for(len = 0; len < strlen(*s); len++){
	    temp[len] = temp[len] - 32;
    }
    *s = temp;
}

static int setValue(int linenum, int tokenClass, char *svalue)
{
    yylval.tokenData = new TokenData;
    
    yylval.tokenData->linenum = linenum;
    yylval.tokenData->tokenstr = strdup(svalue);
    
    if(tokenClass == NUMCONST){
        yylval.tokenData->nvalue = atoi(svalue);
    }
    else if(tokenClass == CHARCONST){ 
	if(strlen(svalue) > 3){
	    if(svalue[1] == '\\'){
	        switch(svalue[2]){
		    case '0':
		        yylval.tokenData->cvalue = '\0';
			break;
		    case 'n':
			yylval.tokenData->cvalue = '\n';
			break;
		    case '\\':
			yylval.tokenData->cvalue = '\\';
			break;
		   default:
			yylval.tokenData->cvalue = svalue[2];
			break;
		}
	    }
	    else
	    {
	        printf("WARNING(%d): character is %d characters long and not a single character: \'%s\'.  The first char will be used.\n", linenum, int(strlen(svalue) - 2), svalue);
		yylval.tokenData->cvalue = svalue[1];
	    }
	}else{
            yylval.tokenData->cvalue = svalue[1];
        }
    }
    else if(tokenClass == STRINGCONST){
        int len;
        int count = 0;
        char temps[int(strlen(svalue))] = {};
        for(len = 0; len < strlen(svalue); len++){
            if(svalue[len] == '\\'){
		switch(svalue[len + 1]){
                    case '0':
                        temps[count] = '\0'; 
                        break;
                    case 'n':
                        temps[count] = '\n';
                        break;
                    case '\\':
                        temps[count] = '\\';
                        break;
                   default:
                        temps[count] = svalue[len + 1];
                        break;
                 }
                 len++;
            }else{
                temps[count] = svalue[len];
            }
            count++;
        }
        temps[count] = '\0';
        yylval.tokenData->strvalue = strdup(temps);
    }
    else if(tokenClass == ID){
        yylval.tokenData->strvalue = strdup(svalue);
    }
    else if(tokenClass == TYPE){
            if(svalue == "void"){savedType = 0; yylval.tokenData->nvalue = 0;}
            else if(strcmp(svalue, "int") == 0){savedType = 1; yylval.tokenData->nvalue = 1;}
            else if(strcmp(svalue, "bool") == 0){savedType = 2; yylval.tokenData->nvalue = 2;}
            else if(strcmp(svalue, "char") == 0){savedType = 3; yylval.tokenData->nvalue = 3;}
            else{savedType = 0;}
    }
    else if(tokenClass == STATIC){
        isStatic = true;
    }
    else if(tokenClass == BOOL){
	if(svalue[0] == 't'){
	    yylval.tokenData->nvalue = 1;
        }else{
            yylval.tokenData->nvalue = 0;
        }
    }
    return tokenClass;
}

%}

%option noyywrap

%%

"bool"|"int"|"char"|"void"           { return setValue(line, TYPE, yytext);}

"+="|"-="|"*="|"/="|"="              { return setValue(line, ASSNG, yytext);}

">="|"<="|"!="|"=="|[<>]             { return setValue(line, REL, yytext);} 

\*                                   { return setValue(line, MUL, yytext);}

\-                                   { return setValue(line, SUB, yytext);}

[/%]                                 { return setValue(line, MULOP, yytext);}

\?                                   { return setValue(line, UNARYOP, yytext);}

\+                                   { return setValue(line, SUMOP, yytext);}

"return"                             { return setValue(line, RETURN, yytext);}

"if"                                 { return setValue(line, IF, yytext);}

"then"                               { return setValue(line, THEN, yytext);}

"else"                               { return setValue(line, ELSE, yytext);}

"while"                              { return setValue(line, WHILE, yytext);}

"do"                                 { return setValue(line, DO, yytext);}

"for"                                { return setValue(line, FOR, yytext);}

"static"                             { return setValue(line, STATIC, yytext);}

"to"                                 { return setValue(line, TO, yytext);}

"by"                                 { return setValue(line, BY, yytext);}

"break"                              { return setValue(line, BREAK, yytext);}

"or"                                 { return setValue(line, OR, yytext);}

"and"                                 { return setValue(line, AND, yytext);}

"not"                                 { return setValue(line, NOT, yytext);}

"++"                                { return setValue(line, INC, yytext);}

"--"                                { return setValue(line, DEC, yytext);}

"false"|"true"                       { return setValue(line, BOOL, yytext); }

[A-Za-z][A-Za-z0-9]*                 { return setValue(line, ID, yytext); }

"//".*                               ;                                 

[0-9]+                               { return setValue(line, NUMCONST, yytext); }

\'([^\\\'\n]|\\.)*\'                 { return setValue(line, CHARCONST, yytext); }

\"([^\\\"\n]|\\.)*\"                 { return setValue(line, STRINGCONST, yytext); }

[ \t\f\v]                            ;

"{"                                  { return setValue(line, LPARAN, yytext); }

"}"                                  { return setValue(line, RPARAN, yytext); }

[;,:\[\]\(\)]            			 { return yytext[0]; }

\n                                   { line++; }

.                                    { printf("ERROR(%d): Invalid or misplaced input character: \'%c\'. Character Ignored.\n", line, yytext[0]); numErrors++; }
%%
