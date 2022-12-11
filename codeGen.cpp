#include "semantic.h"
#include <stdlib.h>

void code_generation(TreeNode *tree) {
  int i;
  while (tree != NULL)
  {
    if (tree->nodekind == StmtK)
    {
      switch (tree->subkind.stmt)
      {
      case IfK:
        break;
      case ReturnK:
        break;
      case BreakK:
        break;
      case RangeK:
        break;
      case ForK:
        break;
      case WhileK:
        break;
      case CompoundK:
        break;
      default:
        break;
      }
    }
    else if (tree->nodekind == ExpK)
    {
      switch (tree->subkind.exp)
      {
      case OpK:
        break;
      case ConstantK:
        break;
      case IdK:
        break;
      case AssignK:
        break;
      case CallK:
        break;
      case InitK:
        break;
      default:
        break;
      }
    }
    else if (tree->nodekind == DeclK)
    {
      switch (tree->subkind.decl)
      {
      case VarK:
        break;
      case FuncK:
        break;
      case ParamK:
        break;
      default:
        break;
      }
    }

    for (i = 0; i < MAXCHILDREN; i++)
    {
      code_generation(tree->child[i]);
    }
    tree = tree->sibling;
  }
}