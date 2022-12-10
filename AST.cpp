#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_set>
#include "AST.h"
using namespace std;

extern int numErrors;
std::unordered_set<ExpKind> exps = {AssignK};
std::unordered_set<DeclKind> decls = {};

TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
  if (s == NULL && numErrors == 0)
  {
    printf("ERROR(SYSTEM): never add a NULL to a sibling list.\n");
    exit(1);
  }
  if (t != NULL)
  {
    TreeNode *tmp;

    tmp = t;
    while (tmp->sibling != NULL)
      tmp = tmp->sibling;
    tmp->sibling = s;
    return t;
  }
  return s;
}

TreeNode *addStmtNode(StmtKind kind, int lineno, int size, int loc)
{
  TreeNode *t = new TreeNode;
  int i;
  if (t == NULL)
    printf("Out of memory error!\n");
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->subkind.stmt = kind;
    t->lineno = lineno;
    t->size = size;
  }
  return t;
}

TreeNode *addDeclNode(DeclKind kind, int lineno, int size, int loc)
{
  TreeNode *t = new TreeNode;
  int i;
  if (t == NULL)
    printf("Out of memory error!\n");
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->subkind.decl = kind;
    t->lineno = lineno;
    t->size = size;
  }
  return t;
}

TreeNode *addExpNode(ExpKind kind, int lineno, ExpType type, int size, int loc)
{
  TreeNode *t = new TreeNode;
  int i;
  if (t == NULL)
    printf("Out of memory error!\n");
  else
  {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->subkind.exp = kind;
    t->lineno = lineno;
    t->expType = type;
    t->size = size;
  }
  return t;
}

// Passes the isStatic and type attributes down the sibling list.
ExpType setType(int count)
{
  switch (count)
  {
  case 0:
    return Void;
    break;
  case 1:
    return Integer;
    break;
  case 2:
    return Boolean;
    break;
  case 3:
    return Char;
    break;
  default:
    return UndefinedType;
    break;
  }
}

static char *printVarKind(VarKind kind)
{
  char *string;
  switch (kind)
  {
  case Local:
    string = strdup("Local");
    break;
  case LocalStatic:
    string = strdup("LocalStatic");
    break;
  case Global:
    string = strdup("Global");
    break;
  case Parameter:
    string = strdup("Parameter");
    break;
  default:
    string = strdup("None");
    break;
  }
  return string;
}

static int indentno = -3;

#define INDENT indentno += 3
#define UNINDENT indentno -= 3

static void printSpaces(void)
{
  int i;
  for (i = 0; i < indentno; i++)
  {
    if (i % 3 == 0)
      printf(".");
    printf(" ");
  }
}

static char *printType(ExpType type)
{
  char *types;
  if (type == Void)
    types = strdup("void");
  else if (type == Integer)
    types = strdup("int");
  else if (type == Char)
    types = strdup("char");
  else if (type == Boolean)
    types = strdup("bool");
  else if (type == CharInt)
    types = strdup("charint");
  else if (type == Equal)
    types = strdup("equal");
  else
    types = strdup("Undefined Type");

  return types;
}

static char *printTypeString(ExpType type, bool ret)
{
  char *types;
  char *temp = strdup("");
  if (type == Void)
    types = strdup("of type void ");
  else if (type == Integer)
    types = strdup("of type int ");
  else if (type == Char)
    types = strdup("of type char ");
  else if (type == Boolean)
    types = strdup("of type bool ");
  else if (type == CharInt)
    types = strdup("of type charint ");
  else if (type == Equal)
    types = strdup("of type equal ");
  else
    types = strdup("of undefined type ");

  return (ret ? types : temp);
}

int childCount = 0;
bool isChild = false;
bool isSibling = false;

void printTree(TreeNode *tree, int count, bool types, bool memory)
{
  int i;
  INDENT;
  while (tree != NULL)
  {
    printSpaces();
    if (isChild)
    {
      printf("Child: %d ", childCount);
      isChild = false;
      count = 0;
    }
    if (isSibling)
    {
      printf("Sibling: %d ", count);
      isSibling = false;
    }
    if (tree->nodekind == StmtK)
    {
      switch (tree->subkind.stmt)
      {
      case IfK:
        printf("If ");
        break;
      case ReturnK:
        printf("Return ");
        break;
      case BreakK:
        printf("Break ");
        break;
      case RangeK:
        printf("Range ");
        break;
      case ForK:
        printf("For ");
        break;
      case WhileK:
        printf("While ");
        break;
      case CompoundK:
        printf("Compound ");
        break;
      default:
        printf("Unknown ExpNode kind\n");
        break;
      }
    }
    else if (tree->nodekind == ExpK)
    {
      switch (tree->subkind.exp)
      {
      case OpK:
        printf("Op: %s %s", tree->attr.name, printTypeString(tree->expType, types));
        break;
      case ConstantK:
        if (tree->expType == Integer)
          printf("Const %d %s", tree->attr.value, printTypeString(tree->expType, types));
        else if (tree->expType == Boolean)
          if (tree->attr.value == 1)
            printf("Const %s %s", "true", printTypeString(tree->expType, types));
          else
            printf("Const %s %s", "false", printTypeString(tree->expType, types));
        else
          printf("Const %s %s", tree->attr.string, printTypeString(tree->expType, types));
        break;
      case IdK:
        if (tree->isArray)
          printf("Id: %s is array of type %s ", tree->attr.name, printType(tree->expType));
        else
          printf("Id: %s %s", tree->attr.name, printTypeString(tree->expType, types));
        break;
      case AssignK:
        printf("Assign: %s %s", tree->attr.name, printTypeString(tree->expType, types));
        break;
      case CallK:
        printf("Call: %s ", tree->attr.name);
        break;
      case InitK:
        printf("Init: ");
        break;
      default:
        printf("Unknown ExpNode kind\n");
        break;
      }
    }
    else if (tree->nodekind == DeclK)
    {
      switch (tree->subkind.decl)
      {
      case VarK:
        if (tree->isArray)
          printf("Var: %s is array of type %s ", tree->attr.name, printType(tree->expType));
        else
          printf("Var: %s of type %s ", tree->attr.name, printType(tree->expType));
        break;
      case FuncK:
        printf("Func: %s returns type %s ", tree->attr.name, printType(tree->expType));
        break;
      case ParamK:
        if (tree->isArray)
          printf("Parm: %s of array of type %s ", tree->attr.name, printType(tree->expType));
        else
          printf("Parm: %s of type %s ", tree->attr.name, printType(tree->expType));
        break;
      default:
        printf("Unknown ExpNode kind\n");
        break;
      }
    }
    else
      printf("Unknown node kind\n");

    if (memory)
    {
      if (exps.find(tree->subkind.exp) == exps.end() && decls.find(tree->subkind.decl) == decls.end() && exps.find(tree->subkind.exp) == exps.end())
        printf("[mem: %s loc: %d size: %d]", printVarKind(tree->var), tree->loc, tree->size);
    }

    printf("[line: %d]\n", tree->lineno);
    for (i = 0; i < MAXCHILDREN; i++)
    {
      isChild = true;
      isSibling = false;
      childCount = i;
      printTree(tree->child[i], count, types, memory);
      isChild = false;
    }
    tree = tree->sibling;
    count++;
    isSibling = true;
  }
  UNINDENT;
}
