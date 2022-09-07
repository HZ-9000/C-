%{
#include "scanType.h"
#include "parser.tab.h"
#include <string.h>
int line = 1;
int numErrors;

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
    
    if(tokenClass == CHARCONST){ 
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
    else if(tokenClass == BOOL){
	if(svalue[0] == 't'){
	    yylval.tokenData->nvalue = 1;
        }else{
            yylval.tokenData->nvalue = 0;
        }
    }
    else if(tokenClass == INCREMENT){
        if(svalue[1] == '+'){
            char w[4] = "INC";
	        yylval.tokenData->strvalue = strdup(w);
        }else{
            char w[4] = "DEC";
            yylval.tokenData->strvalue = strdup(w);
        }
    }
    else if(tokenClass == TYPE || tokenClass == LOOP || tokenClass == LOGIC || tokenClass == KEYWORD || tokenClass == FLOW){
	    upperCase(&svalue);
        yylval.tokenData->strvalue = strdup(svalue);
    }
    else if(tokenClass == NUMCONST){
        yylval.tokenData->nvalue = atoi(svalue);
    }

    return tokenClass;
}

%}

%option noyywrap

%%
"int"|"bool"|"char"|"static"         { return setValue(line, TYPE, yytext); }

"while"|"do"|"for"|"to"|"by"|"then"  { return setValue(line, LOOP, yytext); }

"and"|"or"|"not"                     { return setValue(line, LOGIC, yytext); }

"false"|"true"                       { return setValue(line, BOOL, yytext); }

"break"|"return"|"addasgn"           { return setValue(line, KEYWORD, yytext); }

"begin"|"end"                        { return setValue(line, KEYWORD, yytext); }

"if"|"else"                          { return setValue(line, FLOW, yytext); }

[A-Za-z][A-Za-z0-9]*                 { return setValue(line, ID, yytext); }

"=="                                 { return setValue(line, EQ, yytext); }

">="                                 { return setValue(line, GEQ, yytext); }

"<="                                 { return setValue(line, LEQ, yytext); }

"!="                                 { return setValue(line, NEQ, yytext); }

"<"|">"                              { return setValue(line, RELATION, yytext); }

"<-"|"-="|"*="|"/="|"="              { return setValue(line, ASSIGN, yytext); }

"+="                                 { return setValue(line, ADDASS, yytext); }

"++"|"--"                            { return setValue(line, INCREMENT, yytext); }

"##".*                               ;                                 

[\%\*\(\)\+\-\[\]\:\;\<\>\?\,\/{}]   { return setValue(line, EXTRA, yytext); }

[0-9]+                               { return setValue(line, NUMCONST, yytext); }

\'([^\\\'\n]|\\.)*\'                 { return setValue(line, CHARCONST, yytext); }

\"([^\\\"\n]|\\.)*\"                 { return setValue(line, STRINGCONST, yytext); }

[ \t\f\v]                            ;

\/\/.*                               ;

\n                                   { line++; }

.                                    { printf("ERROR(%d): Invalid or misplaced input character: \'%c\'. Character Ignored.\n", line, yytext[0]); numErrors++; }
%%
