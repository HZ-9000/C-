%{
#define YYERROR_VERBOSE
#include "yyerror.h"
#include  "scanType.h"
#include  "AST.h"
#include  <stdio.h> 
#include  <stdlib.h> 
#include  <string.h> 
#include  <stdbool.h>

extern int yylex();
extern int savedType;
extern bool isStatic;
TreeNode *tree;

%}
 
%union {
    TokenData *tokenData;
    TreeNode *treeNode;
}

%token  <tokenData>  NUMCONST STRINGCONST CHARCONST ID BOOLCONST
%token  <tokenData>  LPARAN RPARAN REL RETURN IF THEN ELSE 
%token  <tokenData>  FOR WHILE DO STATIC TO BY BREAK INC DEC ASSNG
%token  <tokenData>  OR AND NOT SUMOP MULOP UNARYOP MUL SUB
%token  <tokenData>  INT CHAR VOID BOOL ADDASS MULASS DIVASS SUBASS
%token  <tokenData>  GEQ LEQ NEQ EQ

%type <treeNode>  declList decl varDeclList varDeclInit parmIdList
%type <treeNode>  parmList parmId stmt non_if compoundStmt open_stmt 
%type <treeNode>  closed_stmt iterRange returnStmt breakStmt exp forName
%type <treeNode>  simpleExp andExp unaryExp relExp unaryRelExp name
%type <treeNode>  mulExp sumExp factor immutable constant mutable
%type <treeNode>  localDecls varDecl scopedVarDecl call args parms 
%type <treeNode>  parmTypeList argList funDecl expStmt stmtList
%type <tokenData>  typeSpec assignOp relOp

%%
program   :  declList              
                {tree = $1;}
           ;
declList   :  declList   decl    { $$ = addSibling($1, $2); }
            |  decl   {$$ = $1;}                
            ;
decl   :  varDecl   {$$ = $1;}     
        |  funDecl  {$$ = $1;}
        |  error    {$$ = NULL;}
        ;
varDecl   :  typeSpec  varDeclList  ';' {$$ = $2; yyerrok;}
          |  error varDeclList ';'  {$$ = NULL; yyerrok;}
          |  typeSpec  error ';'        {$$ = NULL; yyerrok;}
           ;
typeSpec  : BOOL { $$ = $1; }
          | CHAR { $$ = $1; }
          | INT  { $$ = $1; }
          | VOID { $$ = $1; }
          ;
scopedVarDecl   :  STATIC  typeSpec   varDeclList  ';' 
                        {$$ = $3; yyerrok;}
                 |  typeSpec   varDeclList ';' 
                        {$$ = $2; yyerrok;}   
                 ;
varDeclList   :  varDeclList  ','  varDeclInit  { $$ = addSibling($1, $3); yyerrok; }
              | varDeclInit   {$$ = $1;}
              |  varDeclList ',' error  {$$ = NULL; } 
               | error  {$$ = NULL; }
               ;
varDeclInit   :  ID 
                {$$ = addDeclNode(VarK, $1->linenum, 1 , 1);
                 $$->attr.name = strdup($1->strvalue);
                 $$->isStatic = isStatic;
                 $$->expType = setType(savedType);}
              |  ID '[' NUMCONST ']'
                {$$ = addDeclNode(VarK, $1->linenum, $3->nvalue, 1);
                 $$->expType = setType(savedType);
                 $$->isArray = true;
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->attr.value = $3->nvalue;
                 $$->attr.name = strdup($1->strvalue);}
              |  ID  ':'  simpleExp 
                {$$ = addDeclNode(VarK, $1->linenum, 1, 1);
                 $$->expType = setType(savedType);
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->child[0] = $3;
                 $$->attr.name = strdup($1->strvalue);}
              |  ID '[' NUMCONST ']'  ':'  simpleExp 
                {$$ = addDeclNode(VarK, $1->linenum, $3->nvalue, 1);
                 $$->expType = setType(savedType);
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->child[0] = $6;
                 $$->attr.value = $3->nvalue;
                 $$->attr.name = strdup($1->strvalue);
                 $$->isArray = true;}
              | ID '[' error  { $$ = NULL; }
              | error ']'   { $$ = NULL; yyerrok; }
              | error ':' simpleExp { $$ = NULL; yyerrok; }
              ;
funDecl   :  typeSpec  ID '(' parms ')' compoundStmt 
                {$$ = addDeclNode(FuncK, $1->linenum, 1, 1);
                 $$->attr.name = strdup($2->strvalue);
                 $$->expType = setType($1->nvalue);
                 $$->child[0] = $4;
                 $$->child[1] = $6;}
           | ID '(' parms ')' compoundStmt
                {$$ = addDeclNode(FuncK, $1->linenum, 1, 1);
                 $$->child[0] = $3;
                 $$->child[1] = $5;
                 $$->attr.name = strdup($1->strvalue);}
            | typeSpec error                          { $$ = NULL; }
              | typeSpec ID '(' error                   { $$ = NULL; }
              | ID '(' error                            { $$ = NULL; }
              | ID '(' parms ')' error                  { $$ = NULL; }
           ;
parms   :  parmList  
            {$$ = $1;}
         |  %empty
            {$$=NULL;}
         ;
parmList   :  parmList  ';'  parmTypeList  { $$ = addSibling($1, $3); yyerrok; }
            | parmTypeList   {$$ = $1;}
            | parmList ';' error                      { $$ = NULL; }
              | error                                   { $$ = NULL; }
            ;
parmTypeList   :  typeSpec   parmIdList 
                    {$$ = $2;}
               | typeSpec error                          { $$ = NULL; }
                ; 
parmIdList   : parmIdList ',' parmId                   { $$ = addSibling($1, $3); yyerrok; }
              |  parmId {$$ = $1;}
              | parmIdList ',' error                    { $$ = NULL; }
              | error                                   { $$ = NULL; }
              ; 
parmId   : ID  
                {$$ = addDeclNode(ParamK, $1->linenum, 1, 1);
                 $$->attr.name = strdup($1->strvalue);
                 $$->expType = setType(savedType);}
          | ID '[' ']'
                {$$ = addDeclNode(ParamK, $1->linenum, 1, 1);
                 $$->attr.name = strdup($1->strvalue);
                 $$->expType = setType(savedType);
                 $$->isArray = true;}
          ;
//-----------dangling if correction-----------
stmt  : closed_stmt    
            {$$ = $1;}                    
      | open_stmt          
            {$$ = $1;}                  
      ;
non_if        :  expStmt  
                    {$$ = $1;}
               |  compoundStmt      
                    {$$ = $1;}
               |  returnStmt  
                    {$$ = $1;}
               |  breakStmt 
                    {$$ = $1;}
               ;
expStmt   :  exp  ';' 
            {$$ = $1;}
          | ';'
            {$$=NULL;}
          ;
compoundStmt   : LPARAN  localDecls   stmtList  RPARAN
                    {$$ = addStmtNode(CompoundK, $1->linenum, 1, 1);
                     $$->child[0] = $2;
                     $$->child[1] = $3;
                     yyerrok;}
               ;
localDecls   :  localDecls   scopedVarDecl  
                    {TreeNode *t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $2;
                        $$ = $1;
                    }
                    else 
                        $$ = $2;    
                    }
              |  %empty
                    {$$=NULL;}
              ;
stmtList   :  stmtList   stmt  
                {TreeNode *t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $2;
                        $$ = $1;
                    }
                    else 
                        $$ = $2;    
                }
            |  %empty
                    {$$=NULL;}
            ;
open_stmt    : IF  simpleExp  THEN  stmt  
                {$$ = addStmtNode(IfK, $1->linenum, 1, 1);
                 $$->child[0] = $2;
                 $$->child[1] = $4;}
             | IF  simpleExp  THEN  closed_stmt  ELSE  open_stmt
                {$$ = addStmtNode(IfK, $1->linenum, 1, 1);
                 $$->child[0] = $2;
                 $$->child[1] = $4;
                 $$->child[2] = $6;}
             | WHILE  simpleExp  DO  open_stmt  
                {$$ = addStmtNode(WhileK, $1->linenum, 1, 1);
                 $$->child[0] = $2;
                 $$->child[1] = $4;}
             | FOR forName assignOp iterRange  DO  open_stmt
                {$$ = addStmtNode(ForK, $1->linenum, 1, 1);
                 $$->child[0] = $2;
                 $$->child[1] = $4;
                 $$->child[2] = $6;}
            | IF error THEN stmt                 { $$ = NULL; yyerrok; }
            | IF error ELSE open_stmt             { $$ = NULL; yyerrok; }
            | IF error THEN closed_stmt ELSE open_stmt { $$ = NULL; yyerrok; }
            | WHILE error DO open_stmt            { $$ = NULL; yyerrok; }
            | FOR forName assignOp error DO open_stmt      { $$ = NULL; yyerrok; }
             ; 
closed_stmt   : non_if
              | IF  simpleExp  THEN  closed_stmt  ELSE  closed_stmt
                    {$$ = addStmtNode(IfK, $1->linenum, 1, 1);
                     $$->child[0] = $2;
                     $$->child[1] = $4;
                     $$->child[2] = $6;}
              | WHILE  simpleExp  DO  closed_stmt  
                    {$$ = addStmtNode(WhileK, $1->linenum, 1, 1);
                     $$->child[0] = $2;
                     $$->child[1] = $4;}
              | FOR forName assignOp iterRange  DO  closed_stmt
                    {$$ = addStmtNode(ForK, $1->linenum, 1, 1);
                     $$->child[0] = $2;
                     $$->child[1] = $4;
                     $$->child[2] = $6;}
              | IF error                            { $$ = NULL; }
	      | IF error ELSE closed_stmt               { $$ = NULL; yyerrok; }
	      | IF error THEN closed_stmt ELSE closed_stmt  { $$ = NULL; yyerrok; }
	      | WHILE error DO closed_stmt              { $$ = NULL; yyerrok; }
	      | WHILE error                         { $$ = NULL; }
	      | FOR forName assignOp error DO closed_stmt        { $$ = NULL; yyerrok; }
	      | FOR error                           { $$ = NULL; }
              ;
forName     : ID 
                {$$ = addDeclNode(VarK, $1->linenum, 1, 1);
                 $$->expType = Integer;
                 $$->attr.name = strdup($1->strvalue);}
            ;
iterRange   :  simpleExp  TO  simpleExp  
                {$$ = addStmtNode(RangeK, $2->linenum, 1, 1);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
            |  simpleExp  TO  simpleExp  BY  simpleExp 
                {$$ = addStmtNode(RangeK, $2->linenum, 1, 1);
                 $$->child[0] = $1;
                 $$->child[1] = $3;
                 $$->child[2] = $5;}
            | simpleExp TO error                      { $$ = NULL; }
              | error BY error                          { $$ = NULL; yyerrok; }
              | simpleExp TO simpleExp BY error         { $$ = NULL; }
            ;
returnStmt   : RETURN ';' 
                {$$ = addStmtNode(ReturnK, $1->linenum, 1, 1);}
             | RETURN  exp  ';'
                {$$ = addStmtNode(ReturnK, $1->linenum, 1, 1);
                 $$->child[0] = $2;
                 yyerrok;}
             | RETURN error ';'          { $$ = NULL; yyerrok; }
             ;
breakStmt   : BREAK ';'
                {$$ = addStmtNode(BreakK, $1->linenum, 1, 1);}
            ;
//-----------------------------------------------------
exp   :   mutable   assignOp exp  
            {$$ = addExpNode(AssignK, $2->linenum, Equal, 1, 1);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;
             $$->child[1] = $3;}
       |  mutable INC
            {$$ = addExpNode(AssignK, $2->linenum, Integer, 1, 1);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;}
       |  mutable DEC
            {$$ = addExpNode(AssignK, $2->linenum, Integer, 1, 1);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;}
       |  simpleExp
            {$$ = $1;}
        | error assignOp exp                      { $$ = NULL; yyerrok; }
              | mutable assignOp error                  { $$ = NULL; }
              | error INC                               { $$ = NULL; yyerrok; }
              | error DEC                               { $$ = NULL; yyerrok; }
       ; 
assignOp : MULASS { $$ = $1; }
         | ADDASS { $$ = $1; }
         | SUBASS { $$ = $1; }
         | DIVASS { $$ = $1; }
         | ASSNG { $$ = $1; }
         ;
simpleExp   :  simpleExp  OR  andExp  
                    {$$ = addExpNode(OpK, $2->linenum, Boolean, 1, 1);
                     $$->attr.name = strdup($2->tokenstr);
                     $$->child[0] = $1;
                     $$->child[1] = $3;}
             |  andExp
                    {$$ = $1;}
             | simpleExp OR error  { $$ = NULL; };
             ; 
andExp   :  andExp  AND  unaryRelExp 
                {$$ = addExpNode(OpK, $2->linenum, Boolean, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  unaryRelExp
                {$$ = $1;}
          | andExp AND error { $$ = NULL; }
          ; 
unaryRelExp   : NOT  unaryRelExp 
                    {$$ = addExpNode(OpK, $1->linenum, Boolean, 1, 1);
                     $$->attr.name = strdup($1->tokenstr);
                     $$->child[0] = $2;}
               |  relExp
                    {$$ = $1;}
                | NOT error  { $$ = NULL; }
               ; 
relExp   :  sumExp   relOp   sumExp 
                {$$ = addExpNode(OpK, $2->linenum, Boolean, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  sumExp
                {$$ = $1;}
          ; 
relOp    :  REL  { $$ = $1; }
         |  GEQ  { $$ = $1; }
         |  LEQ  { $$ = $1; }
         |  NEQ  { $$ = $1; }
         |  EQ   { $$ = $1; }
         ;
sumExp   :  sumExp   SUMOP   mulExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
         |  sumExp   SUB   mulExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  mulExp
                {$$ = $1;}
          | sumExp SUMOP error { $$ = NULL; }
          | sumExp SUB error { $$ = NULL; }
          ;
mulExp   :  mulExp   MULOP   unaryExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
         |  mulExp   MUL   unaryExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  unaryExp
                {$$ = $1;}
          | mulExp MULOP error { $$ = NULL; }
          | mulExp MUL error { $$ = NULL; }
          ;
unaryExp   :  UNARYOP unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($1->tokenstr);
                 $$->child[0] = $2;}
            |   MUL unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer, 1, 1);
                 $$->attr.name = strdup("sizeof");
                 $$->child[0] = $2;}
            |   SUB unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer, 1, 1);
                 $$->attr.name = strdup("chsign");
                 $$->child[0] = $2;}
            |  factor
                {$$ = $1;}
            | UNARYOP error { $$ = NULL; }
            | MUL error { $$ = NULL; }
            | SUB error { $$ = NULL; }
            ; 
factor   :  mutable  
                {$$ = $1;}
          |  immutable
                {$$ = $1;}
          ; 
mutable   : ID 
                {$$ = addExpNode(IdK, $1->linenum, setType(savedType), 1, 1);
                 $$->attr.name = strdup($1->strvalue);}
           | name '['  exp  ']'
                {$$ = addExpNode(OpK, line, Void, 1, 1);
                 $$->child[0] = $1;
                 $$->child[1] = $3;
                 $$->attr.name = strdup("[");}
           ;
name       : ID 
                {$$ = addExpNode(IdK, $1->linenum, Integer, 1, 1);
                 $$->attr.name = strdup($1->strvalue);}
           ;
immutable   : '('  exp  ')' 
                {$$ = $2; yyerrok; }
             |  call  
                    {$$ = $1;}
             |  constant
                    {$$ = $1;}
              | '(' error  { $$ = NULL; }
             ; 
call   : ID '('  args  ')'
            {$$ = addExpNode(CallK, $1->linenum, Void, 1, 1);
             $$->attr.name = strdup($1->strvalue);
             $$->child[0] = $3;}
        | error '('   { $$ = NULL; yyerrok; }
        ;
args   : argList  
            {$$ = $1;}
        | %empty
            {$$=NULL;}
        ;
argList   :  argList  ','  exp  { $$ = addSibling($1, $3); yyerrok; }
          | argList ',' error  { $$=NULL; }
           |  exp
                {$$ = $1;}
           ; 
constant   : NUMCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Integer, 1, 1);
                 $$->attr.value = $1->nvalue;}
            | CHARCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Char, 1, 1);
                 $$->attr.name = strdup($1->tokenstr);}
            | STRINGCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Char, 1, 1);
                 $$->attr.string = strdup($1->strvalue);
                 $$->isArray = true;}
            | BOOLCONST
                {$$ = addExpNode(ConstantK, $1->linenum, Boolean, 1, 1);
                 $$->attr.value = $1->nvalue;}
            ; 
%%
