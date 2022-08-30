%{
#include "scanType.h"
#include <stdio.h>
#include <string.h>

extern int yylex();
extern FILE *yyin;
extern int line;
extern int numErrors;

#define YYERROR_VERBOSE
void yyerror(const char *msg)
{
    printf("ERROR(%d): %s\n", line, msg);
    numErrors++;
}

%}
 
%union {
    TokenData *tokenData;
}

%token <tokenData> NUMCONST STRINGCONST CHARCONST ID TYPE EXTRA
%token <tokenData> LOOP LOGIC BOOL ASSIGN RELATION KEYWORD INCREMENT FLOW

%%
S : T S
  | T
  ;

T : NUMCONST     {printf("Line %d Token: NUMCONST Value: %d Input: %s\n", line, $1->nvalue, $1->tokenstr);}
  | CHARCONST    {printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", line, $1->cvalue, $1->tokenstr);}
  | STRINGCONST  {printf("Line %d Token: STRINGCONST Value: %s Len: %d Input: %s\n", line, $1->strvalue, int(strlen($1->strvalue) - 2), $1->tokenstr);}
  | ID           {printf("Line %d Token: ID Value: %s\n", line, $1->strvalue);}
  | TYPE         {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | EXTRA        {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | LOOP         {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | LOGIC        {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | BOOL         {printf("Line %d Token: BOOLCONST Value: %d Input: %s\n", line, $1->nvalue, $1->tokenstr);}
  | ASSIGN       {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | RELATION     {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | KEYWORD      {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | INCREMENT    {printf("Line %d Token: %s\n", line, $1->strvalue);}
  | FLOW         {printf("Line %d Token: %s\n", line, $1->strvalue);}
  ;

%%
extern int yydebug;
int main(int argc, char *argv[])
{
    if(argc > 1){
        if((yyin = fopen(argv[1], "r"))){
            //success
            numErrors = 0;
            yyparse();
        }
        else
        {
            //fail
            printf("ERROR: failed to open \'%s\'\n", argv[1]);
            exit(1);
        }
    }

    return 0;
}
