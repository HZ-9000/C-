#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AST.h"
using namespace std;

TreeNode *addStmtNode(StmtKind kind, int lineno)
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
  }
  return t;
}

TreeNode *addDeclNode(DeclKind kind, int lineno)
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
  }
  return t;
}

TreeNode *addExpNode(ExpKind kind, int lineno, ExpType type)
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
  }
  return t;
}

// Adds a TreeNode to a list of siblings.
// Adding a NULL node to the sibling list is probably a programming error!
TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
  if (s == NULL)
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

int childCount = 0;
bool isChild = false;
bool isSibling = false;

void printTree(TreeNode *tree, int count)
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
        printf("If [line: %d]\n", tree->lineno);
        break;
      case ReturnK:
        printf("Return [line: %d]\n", tree->lineno);
        break;
      case BreakK:
        printf("Break [line: %d]\n", tree->lineno);
        break;
      case RangeK:
        printf("Range [line: %d]\n", tree->lineno);
        break;
      case ForK:
        printf("For [line: %d]\n", tree->lineno);
        break;
      case WhileK:
        printf("While [line: %d]\n", tree->lineno);
        break;
      case CompoundK:
        printf("Compound [line: %d]\n", tree->lineno);
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
        printf("Op: %s [line: %d]\n", tree->attr.name, tree->lineno);
        break;
      case ConstantK:
        if (tree->expType == Integer)
          printf("Const %d [line: %d]\n", tree->attr.value, tree->lineno);
        else if (tree->expType == Boolean)
          if (tree->attr.value == 1)
            printf("Const %s [line: %d]\n", "true", tree->lineno);
          else
            printf("Const %s [line: %d]\n", "false", tree->lineno);
        else
          printf("Const %s [line: %d]\n", tree->attr.string, tree->lineno);
        break;
      case IdK:
        printf("Id: %s [line: %d]\n", tree->attr.name, tree->lineno);
        break;
      case AssignK:
        tree->child[0]->isInitialized = true;
        printf("Assign: %s [line: %d]\n", tree->attr.name, tree->lineno);
        break;
      case CallK:
        printf("Call: %s [line: %d]\n", tree->attr.name, tree->lineno);
        break;
      case InitK:
        printf("Init: [line: %d]\n", tree->lineno);
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
          printf("Var: %s of array of type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        else if (tree->isStatic)
          printf("Var: %s of static type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        else
          printf("Var: %s of type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        break;
      case FuncK:
        printf("Func: %s returns type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        break;
      case ParamK:
        if (tree->isArray)
          printf("Parm: %s of array of type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        else
          printf("Parm: %s of type %s [line: %d]\n", tree->attr.name, printType(tree->expType), tree->lineno);
        break;
      default:
        printf("Unknown ExpNode kind\n");
        break;
      }
    }
    else
      printf("Unknown node kind\n");
    for (i = 0; i < MAXCHILDREN; i++)
    {
      isChild = true;
      isSibling = false;
      childCount = i;
      printTree(tree->child[i], count);
      isChild = false;
    }
    tree = tree->sibling;
    count++;
    isSibling = true;
  }
  UNINDENT;
}
