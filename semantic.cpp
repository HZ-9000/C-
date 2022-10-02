#include "semantic.h"
#include <string.h>
#include <string>

extern int errorCount;
extern int warningCount;

SymbolTable symtab = SymbolTable();

static char *printType(ExpType type)
{
    char *types;
    if (type == Void)
        types = strdup("type void");
    else if (type == Integer)
        types = strdup("type int");
    else if (type == Char)
        types = strdup("type char");
    else if (type == Boolean)
        types = strdup("type bool");
    else if (type == CharInt)
        types = strdup("type charint");
    else if (type == Equal)
        types = strdup("type equal");
    else
        types = strdup("type Undefined Type");

    return types;
}

void semanticAnalysis(TreeNode *tree, bool debug)
{
    symtabTraverse(tree);

    if (symtab.lookup("main") == NULL)
        printErrorMessage(15, (char *)"", (char *)"", (char *)"", tree->lineno);

    printf("Number of warnings: %d\n", warningCount);
    printf("Number of errors: %d\n", errorCount);
    symtab.print(pointerPrintStr);
}

int scope_count = 0;

void symtabTraverse(TreeNode *tree)
{
    while (tree != NULL)
    {
        bool isScope = false;
        if (tree->nodekind == DeclK)
        {
            if(symtab.depth() > 1)
                tree->var = Local;
            else
                tree->var = Global;

            TreeNode *point = (TreeNode *)symtab.lookup(tree->attr.name);
            if (!symtab.insert(tree->attr.name, tree))
            {
                printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->attr.name, point->lineno);
                errorCount++;
            }
            else
            {
                if (tree->subkind.decl == FuncK)
                {
                    isScope = true;
                    symtab.enter(std::string(tree->attr.name));
                }
            }
        }
        else if (tree->subkind.stmt == CompoundK)
        {
            isScope = true;
            symtab.enter(std::string("Compound_Statment-" + std::to_string(scope_count++)));
        }
        else if (tree->nodekind == ExpK)
        {
            switch (tree->subkind.exp)
            {
            case OpK:
            {
                TreeNode *point = (TreeNode *)symtab.lookup(tree->child[0]->attr.name);
                if(point != NULL && point->var == Local)
                {
                    printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->child[0]->attr.name);
                    warningCount++;
                }
                if(tree->child[0]->expType != tree->expType)
                {
                    printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(tree->child[0]->expType));
                    errorCount++;
                }
                if(tree->child[1]->expType != tree->expType)
                {
                    printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(tree->child[1]->expType));
                    errorCount++;
                }
            }
                break;
            case IdK:
            {
                TreeNode *point = (TreeNode *)symtab.lookup(tree->attr.name);
                if(point == NULL)
                {
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
                    errorCount++;
                }
                // else if(point->subkind.decl == VarK)
                // {
                //     printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->attr.name);
                // }
            }
                break;
            case CallK:
                if(symtab.lookup(tree->attr.name) == NULL)
                {
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
                    errorCount++;
                }
                break;
            case AssignK:
                tree->child[0]->isInitialized = true;
                break;
            case InitK:
                break;
            case ConstantK:
                break;
            default:
                printf("Unknown ExpNode kind: %d\n", tree->subkind.exp);
                break;
            }
        }

        for (int i = 0; i < MAXCHILDREN; i++)
        {
            symtabTraverse(tree->child[i]);
        }

        if (symtab.depth() > 1 && isScope)
        {
            symtab.leave();
        }

        tree = tree->sibling;
    }
}