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
        else if (tree->nodekind == StmtK)
        {
            switch (tree->subkind.exp)
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
                isScope = true;
                symtab.enter(std::string("For_Loop-" + std::to_string(scope_count++)));
                break;
            case WhileK:
                isScope = true;
                symtab.enter(std::string("While_Loop-" + std::to_string(scope_count++)));
                break;
            case CompoundK:
                isScope = true;
                symtab.enter(std::string("Compound_Statment-" + std::to_string(scope_count++)));
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
                proccessOP(tree);
                tree->child[0] = NULL;
                tree->child[1] = NULL;
                break;
            case IdK:
                break;
            case CallK:
                if (symtab.lookup(tree->attr.name) == NULL)
                {
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
                    errorCount++;
                }
                break;
            case AssignK:
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

void proccessOP(TreeNode *tree)
{
    if (tree->child[0] == NULL)
    {
        proccessID(tree);
    }
    else if(tree->child[1] != NULL)
    {
        TreeNode *lhs = (TreeNode *)symtab.lookup(tree->child[0]->attr.name);
        TreeNode *rhs = (TreeNode *)symtab.lookup(tree->child[1]->attr.name);
        if (tree->attr.name[0] == '[')
        {
            if ((lhs != NULL && !lhs->isArray) || lhs == NULL)
            {
                printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, lhs->attr.name);
                errorCount++;
            }
        }
        if (tree->attr.name[0] == ('*' || '+'))
        {
            if (lhs->expType != tree->expType)
            {
                printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(lhs->expType));
                errorCount++;
            }
            if (rhs->expType != tree->expType)
            {
                printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(rhs->expType));
                errorCount++;
            }
        }
    }
}

void proccessID(TreeNode *tree)
{
}