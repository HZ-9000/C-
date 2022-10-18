#include "semantic.h"
#include <string.h>
#include <string>
#include <unordered_set>

int errorCount = 0;
int warningCount = 0;

std::unordered_set<std::string> intOps = {"++", "--", "+", "-", "*", "/"};
std::unordered_set<std::string> boolOps = {"==", "!=", "<", ">", "<=", ">=", "not"};
std::unordered_set<std::string> unaryOps = {"?", "chsign", "not", "--", "++"};
std::unordered_set<std::string> sameOps = {"<", ">", ">=", "<=", "!=", "=="};
std::unordered_set<std::string> arrayOps = {"sizeof"};

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

void checkUsed(std::string sym, void *ptr)
{
    TreeNode *entry = (TreeNode *)ptr;

    if (!entry->isUsed)
    {
        printf("WARNING(%d): The variable '%s' seems not to be used.\n", entry->lineno, entry->attr.name);
        warningCount++;
    }
}

void semanticAnalysis(TreeNode *tree, bool debug)
{
    symtab.debug(debug);
    symtabTraverse(tree, false, true);
}

int scope_count = 0;

void symtabTraverse(TreeNode *tree, bool isFunc, bool warn)
{
    while (tree != NULL)
    {
        bool isScope = false;
        if (tree->nodekind == DeclK)
        {
            if (tree->subkind.decl == VarK && tree->child[0] != NULL)
                tree->isInitialized = true;

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
                    isFunc = true;
                    symtab.enter(std::string(tree->attr.name));
                }
            }
            for (int i = 0; i < MAXCHILDREN; i++)
            {
                symtabTraverse(tree->child[i], isFunc, warn);
            }
            if(symtab.depth() > 1 && isScope)
            {
                symtab.applyToAll(checkUsed);
                symtab.leave();
            }
        }
        else if (tree->nodekind == StmtK)
        {
            switch (tree->subkind.stmt)
            {
            case IfK:
                symtab.enter(std::string("If"));
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc, warn);
                }
                symtab.applyToAll(checkUsed);
                symtab.leave();
                break;
            case ReturnK:
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc, warn);
                }
                break;
            case BreakK:
                break;
            case RangeK:
                break;
            case ForK:
                symtab.enter(std::string("For"));
                tree->child[0]->isInitialized = true;
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc, warn);
                }
                symtab.applyToAll(checkUsed);
                symtab.leave();
                break;
            case WhileK:
                symtab.enter(std::string("While"));
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc, warn);
                }
                symtab.applyToAll(checkUsed);
                symtab.leave();
                break;
            case CompoundK:
                if (!isFunc)
                {
                    symtab.enter(std::string("Compound"));
                }
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], false, warn);
                }
                if (!isFunc)
                {
                    symtab.applyToAll(checkUsed);
                    symtab.leave();
                }
                isFunc = false;
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
                symtabTraverse(tree->child[0], isFunc, warn);
                symtabTraverse(tree->child[1], isFunc, warn);
                proccessOP(tree);
                break;
            case IdK:
                proccessID(tree, warn);
                break;
            case CallK:
                proccessCALL(tree);
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc, warn);
                }
                break;
            case AssignK:
                proccessASSNG(tree, isFunc);
                break;
            case InitK:
                break;
            case ConstantK:
                break;
            default:
                break;
            }
        }

        tree = tree->sibling;
    }
}

bool proccessLandR(TreeNode *tree, bool left, bool right, ExpType type)
{
    if (right && tree->expType != type)
    {
        printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n",
               tree->lineno, tree->attr.name, printType(type), printType(tree->expType));
        errorCount++;
        return false;
    }
    if (left && tree->expType != type)
    {
        printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n",
               tree->lineno, tree->attr.name, printType(type), printType(tree->expType));
        errorCount++;
        return false;
    }

    return true;
}

void proccessID(TreeNode *tree, bool warn)
{
    TreeNode *entry = (TreeNode *)symtab.lookup(tree->attr.name);

    if (entry == NULL)
    {
        printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
        errorCount++;
        tree->expType = UndefinedType;
    }
    else
    {
        if (entry->subkind.decl == FuncK)
        {
            printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tree->lineno, tree->attr.name);
            errorCount++;
        }
        else
        {
            entry->isUsed = true;
            tree->expType = entry->expType;
            tree->isArray = entry->isArray;

            if (tree->isInitialized)
                entry->isInitialized = true;

            if (!entry->isInitialized && !entry->warned && warn)
            {
                printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->attr.name);
                warningCount++;
                entry->warned = true;
            }
        }
    }
}

void proccessCALL(TreeNode *tree)
{
    TreeNode *entry = (TreeNode *)symtab.lookup(tree->attr.name);

    if (entry == NULL)
    {
        printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
        errorCount++;
    }
    else
    {
        entry->isUsed = true;
        tree->expType = entry->expType;
    }
}

void proccessASSNG(TreeNode *tree, bool isFunc)
{
    symtabTraverse(tree->child[0], isFunc, false);
    symtabTraverse(tree->child[1], isFunc, true);

    tree->expType = tree->child[0]->expType;

    if (!(tree->child[0]->expType == UndefinedType || tree->child[1]->expType == UndefinedType))
    {
        if (tree->child[0]->expType != tree->child[1]->expType)
        {
            printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n",
                   tree->lineno, tree->attr.name, printType(tree->child[0]->expType), printType(tree->child[1]->expType));
            errorCount++;
        }
        if (tree->child[0]->isArray != tree->child[1]->isArray)
        {
            printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is %s and rhs is %s.\n",
                   tree->lineno, tree->attr.name, (tree->child[0]->isArray ? "an array" : "not an array"), (tree->child[1]->isArray ? "an array" : "not an array"));
            errorCount++;
        }
    }

    tree->child[0]->isInitialized = true;
}

void proccessOP(TreeNode *tree)
{
    if (boolOps.find(tree->attr.name) != boolOps.end())
        tree->expType = Boolean;
    else if (intOps.find(tree->attr.name) != intOps.end())
        tree->expType = Integer;
    else
        tree->expType = tree->child[0]->expType;

    if (tree->attr.name[0] == '[')
    {
        if (!tree->child[0]->isArray)
        {
            printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, tree->child[0]->attr.name);
            errorCount++;
        }
        proccessLandR(tree, false, true, tree->expType);
        if (tree->child[1]->isArray)
        {
            printf("ERROR(%d): Array index is the unindexed array '%s'.\n", tree->lineno, tree->child[1]->attr.name);
            errorCount++;
        }
    }

    if (unaryOps.find(tree->attr.name) != unaryOps.end())
        proccessLandR(tree, true, false, tree->expType);
    else if (sameOps.find(tree->attr.name) != sameOps.end())
    {
        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n",
               tree->lineno, tree->attr.name, printType(tree->child[0]->expType), printType(tree->child[1]->expType));
        errorCount++;
    }
    else
    {
        proccessLandR(tree, true, false, tree->expType);
        proccessLandR(tree, false, true, tree->expType);
    }
}