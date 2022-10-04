%{
#include  "scanType.h"
#include  "AST.h"
#include  <stdio.h> 
#include  <string.h> 
#include  <stdbool.h>

extern int yylex();
extern int line;
extern int numErrors;
extern int savedType;
extern bool isStatic;
TreeNode *tree;

#define YYERROR_VERBOSE
void yyerror(const char *msg)
{
    printf("ERROR(%d): %s\n", line, msg);
    numErrors++;
}

%}
 
%union {
    TokenData *tokenData;
    TreeNode *treeNode;
}

%token  <tokenData>  NUMCONST STRINGCONST CHARCONST ID BOOL TYPE
%token  <tokenData>  LPARAN RPARAN REL RETURN IF THEN ELSE 
%token  <tokenData>  FOR WHILE DO STATIC TO BY BREAK INC DEC ASSNG
%token  <tokenData>  OR AND NOT SUMOP MULOP UNARYOP MUL SUB

%type <treeNode>  declList decl varDeclList varDeclInit parmIdList
%type <treeNode>  parmList parmId stmt non_if compoundStmt open_stmt 
%type <treeNode>  closed_stmt iterRange returnStmt breakStmt exp forName
%type <treeNode>  simpleExp andExp unaryExp relExp unaryRelExp name
%type <treeNode>  mulExp sumExp factor immutable constant mutable
%type <treeNode>  localDecls varDecl scopedVarDecl call args parms 
%type <treeNode>  parmTypeList argList funDecl expStmt stmtList

%%
program   :  declList              
                {tree = $1;}
           ;
declList   :  declList   decl      
                {TreeNode * t = $1;
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
            |  decl   {$$ = $1;}                
            ;
decl   :  varDecl   {$$ = $1;}     
        |  funDecl  {$$ = $1;}
        ;
varDecl   :  TYPE  varDeclList  ';' 
                {$$ = $2;}
           ;
scopedVarDecl   :  STATIC  TYPE   varDeclList  ';' 
                        {$$ = $3;}
                 |  TYPE   varDeclList ';' 
                        {$$ = $2;}   
                 ;
varDeclList   :  varDeclList  ','  varDeclInit  
                    {
                    TreeNode * t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL){
                            t = t->sibling;
                        }
                        t->sibling = $3;
                        $$ = $1;
                    }
                    else 
                        {$$ = $3;}    
                }
               | varDeclInit   {$$ = $1;}
               ;
varDeclInit   :  ID 
                {$$ = addDeclNode(VarK, $1->linenum);
                 $$->attr.name = strdup($1->strvalue);
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->expType = setType(savedType);}
              |  ID '[' NUMCONST ']'
                {$$ = addDeclNode(VarK, $1->linenum);
                 $$->expType = setType(savedType);
                 $$->isArray = true;
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->attr.value = $3->nvalue;
                 $$->attr.name = strdup($1->strvalue);}
              |  ID  ':'  simpleExp 
                {$$ = addDeclNode(VarK, $1->linenum);
                 $$->expType = setType(savedType);
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->child[0] = $3;
                 $$->attr.name = strdup($1->strvalue);}
              |  ID '[' NUMCONST ']'  ':'  simpleExp 
                {$$ = addDeclNode(VarK, $1->linenum);
                 $$->expType = setType(savedType);
                 $$->isStatic = isStatic;
                 isStatic = false;
                 $$->child[0] = $6;
                 $$->attr.value = $3->nvalue;
                 $$->attr.name = strdup($1->strvalue);
                 $$->isArray = true;}
              ;
funDecl   :  TYPE  ID '(' parms ')' compoundStmt 
                {$$ = addDeclNode(FuncK, $1->linenum);
                 $$->attr.name = strdup($2->strvalue);
                 $$->expType = setType($1->nvalue);
                 $$->child[0] = $4;
                 $$->child[1] = $6;}
           | ID '(' parms ')' compoundStmt
                {$$ = addDeclNode(FuncK, $1->linenum);
                 $$->child[0] = $3;
                 $$->child[1] = $5;
                 $$->attr.name = strdup($1->strvalue);}
           ;
parms   :  parmList  
            {$$ = $1;}
         |  %empty
            {$$=NULL;}
         ;
parmList   :  parmList  ';'  parmTypeList  
                    {TreeNode *t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $3;
                        $$ = $1;
                    }
                    else 
                        $$ = $3;    
                }
            | parmTypeList
                    {$$ = $1;}
            ;
parmTypeList   :  TYPE   parmIdList 
                    {$$ = $2;}
                ; 
parmIdList   :  parmIdList  ','  parmId  
                    {TreeNode *t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $3;
                        $$ = $1;
                    }
                    else 
                        $$ = $3;    
                }
              |  parmId
                    {$$ = $1;}
              ; 
parmId   : ID  
                {$$ = addDeclNode(ParamK, $1->linenum);
                 $$->attr.name = strdup($1->strvalue);
                 $$->expType = setType(savedType);}
          | ID '[' ']'
                {$$ = addDeclNode(ParamK, $1->linenum);
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
                    {$$ = addStmtNode(CompoundK, $1->linenum);
                     $$->child[0] = $2;
                     $$->child[1] = $3;}
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
                {$$ = addStmtNode(IfK, $1->linenum);
                 $$->child[0] = $2;
                 $$->child[1] = $4;}
             | IF  simpleExp  THEN  closed_stmt  ELSE  open_stmt
                {$$ = addStmtNode(IfK, $1->linenum);
                 $$->child[0] = $2;
                 $$->child[1] = $4;
                 $$->child[2] = $6;}
             | WHILE  simpleExp  DO  open_stmt  
                {$$ = addStmtNode(WhileK, $1->linenum);
                 $$->child[0] = $2;
                 $$->child[1] = $4;}
             | FOR forName ASSNG iterRange  DO  open_stmt
                {$$ = addStmtNode(ForK, $1->linenum);
                 $$->child[0] = $2;
                 $$->child[1] = $4;
                 $$->child[2] = $6;}
             ; 
closed_stmt   : non_if
              | IF  simpleExp  THEN  closed_stmt  ELSE  closed_stmt
                    {$$ = addStmtNode(IfK, $1->linenum);
                     $$->child[0] = $2;
                     $$->child[1] = $4;
                     $$->child[2] = $6;}
              | WHILE  simpleExp  DO  closed_stmt  
                    {$$ = addStmtNode(WhileK, $1->linenum);
                     $$->child[0] = $2;
                     $$->child[1] = $4;}
              | FOR forName ASSNG iterRange  DO  closed_stmt
                    {$$ = addStmtNode(ForK, $1->linenum);
                     $$->child[0] = $2;
                     $$->child[1] = $4;
                     $$->child[2] = $6;}
              ;
forName     : ID 
                {$$ = addDeclNode(VarK, $1->linenum);
                 $$->expType = Integer;
                 $$->attr.name = strdup($1->strvalue);}
            ;
iterRange   :  simpleExp  TO  simpleExp  
                {$$ = addStmtNode(RangeK, $2->linenum);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
            |  simpleExp  TO  simpleExp  BY  simpleExp 
                {$$ = addStmtNode(RangeK, $2->linenum);
                 $$->child[0] = $1;
                 $$->child[1] = $3;
                 $$->child[2] = $5;}
            ;
returnStmt   : RETURN ';' 
                {$$ = addStmtNode(ReturnK, $1->linenum);}
             | RETURN  exp  ';'
                {$$ = addStmtNode(ReturnK, $1->linenum);
                 $$->child[0] = $2;}
             ;
breakStmt   : BREAK ';'
                {$$ = addStmtNode(BreakK, $1->linenum);}
            ;
//-----------------------------------------------------
exp   :   mutable   ASSNG exp  
            {$$ = addExpNode(AssignK, $2->linenum, Equal);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;
             $$->child[1] = $3;}
       |  mutable INC
            {$$ = addExpNode(AssignK, $2->linenum, Void);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;}
       |  mutable DEC
            {$$ = addExpNode(AssignK, $2->linenum, Void);
             $$->attr.name = strdup($2->tokenstr);
             $$->child[0] = $1;}
       |  simpleExp
            {$$ = $1;}
       ; 
simpleExp   :  simpleExp  OR  andExp  
                    {$$ = addExpNode(OpK, $2->linenum, Boolean);
                     $$->attr.name = strdup($2->tokenstr);
                     $$->child[0] = $1;
                     $$->child[1] = $3;}
             |  andExp
                    {$$ = $1;}
             ; 
andExp   :  andExp  AND  unaryRelExp 
                {$$ = addExpNode(OpK, $2->linenum, Boolean);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  unaryRelExp
                {$$ = $1;}
          ; 
unaryRelExp   : NOT  unaryRelExp 
                    {$$ = addExpNode(OpK, $1->linenum, Boolean);
                     $$->attr.name = strdup($1->tokenstr);
                     $$->child[0] = $2;}
               |  relExp
                    {$$ = $1;}
               ; 
relExp   :  sumExp   REL   sumExp 
                {$$ = addExpNode(OpK, $2->linenum, Boolean);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  sumExp
                {$$ = $1;}
          ; 
sumExp   :  sumExp   SUMOP   mulExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
         |  sumExp   SUB   mulExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  mulExp
                {$$ = $1;}
          ;
mulExp   :  mulExp   MULOP   unaryExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
         |  mulExp   MUL   unaryExp 
                {$$ = addExpNode(OpK, $2->linenum, Integer);
                 $$->attr.name = strdup($2->tokenstr);
                 $$->child[0] = $1;
                 $$->child[1] = $3;}
          |  unaryExp
                {$$ = $1;}
          ;
unaryExp   :  UNARYOP unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer);
                 $$->attr.name = strdup($1->tokenstr);
                 $$->child[0] = $2;}
            |   MUL unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer);
                 $$->attr.name = strdup("sizeof");
                 $$->child[0] = $2;}
            |   SUB unaryExp
                {$$ = addExpNode(OpK, $1->linenum, Integer);
                 $$->attr.name = strdup("chsign");
                 $$->child[0] = $2;}
            |  factor
                {$$ = $1;}
            ; 
factor   :  mutable  
                {$$ = $1;}
          |  immutable
                {$$ = $1;}
          ; 
mutable   : ID 
                {$$ = addExpNode(IdK, $1->linenum, setType(savedType));
                 $$->attr.name = strdup($1->strvalue);}
           | name '['  exp  ']'
                {$$ = addExpNode(OpK, line, Void);
                 $$->child[0] = $1;
                 $$->child[1] = $3;
                 $$->attr.name = strdup("[");}
           ;
name       : ID 
                {$$ = addExpNode(IdK, $1->linenum, Integer);
                 $$->attr.name = strdup($1->strvalue);}
           ;
immutable   : '('  exp  ')' 
                {$$ = $2;}
             |  call  
                    {$$ = $1;}
             |  constant
                    {$$ = $1;}
             ; 
call   : ID '('  args  ')'
            {$$ = addExpNode(CallK, $1->linenum, Void);
             $$->attr.name = strdup($1->strvalue);
             $$->child[0] = $3;}
        ;
args   : argList  
            {$$ = $1;}
        | %empty
            {$$=NULL;}
        ;
argList   :  argList  ','  exp 
                {TreeNode *t = $1;
                    if (t != NULL)
                    {
                        while(t->sibling != NULL)
                            t = t->sibling;
                        t->sibling = $3;
                        $$ = $1;
                    }
                    else 
                        $$ = $3;    
                }
           |  exp
                {$$ = $1;}
           ; 
constant   : NUMCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Integer);
                 $$->attr.value = $1->nvalue;}
            | CHARCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Char);
                 $$->attr.name = strdup($1->tokenstr);}
            | STRINGCONST 
                {$$ = addExpNode(ConstantK, $1->linenum, Char);
                 $$->attr.string = strdup($1->strvalue);
                 $$->isArray = true;}
            | BOOL
                {$$ = addExpNode(ConstantK, $1->linenum, Boolean);
                 $$->attr.value = $1->nvalue;}
            ; 
%%
