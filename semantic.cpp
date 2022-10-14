#include "semantic.h"
#include <string.h>
#include <string>
#include <unordered_set>

int errorCount = 0;
int warningCount = 0;

std::unordered_set<std::string> sameOps = {"==", "<=", ">=", "!=", "<", ">", "="};
std::unordered_set<std::string> intOps = {"sizeof","+=", "-=", "/=", "*=", "+", "*", "-", "/", "%", "?", "++", "--"};
std::unordered_set<std::string> unaryOps = {"?", "chsign", "not", "--", "++"};
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

bool nestedArr(TreeNode *arr)
{
    if (arr->child[1] != NULL)
    {
        if (arr->child[1]->subkind.exp == ConstantK)
        {
            proccessID(arr->child[0], true);
            return true;
        }
        else if (arr->child[1]->subkind.exp == IdK && arr->child[1]->expType == Integer)
        {
            proccessID(arr->child[1], false);
            return true;
        }
        else if (arr->child[1]->subkind.exp == OpK)
        {
            proccessID(arr->child[0], true);
            return nestedArr(arr->child[1]);
        }
    }
    if (arr->child[0]->subkind.exp == IdK && arr->child[0]->expType == Integer)
    {
        proccessID(arr->child[0], true);
        return true;
    }
    else if (arr->child[0]->expType == Integer)
    {
        return true;
    }

    return false;
}
void semanticAnalysis(TreeNode *tree, bool debug)
{
    symtab.debug(debug);
    symtabTraverse(tree, false);

    TreeNode *entry = (TreeNode *)symtab.lookup("main");

    int count = 0;

    if (entry == NULL)
    {
        printf("ERROR(LINKER): A function named 'main()' must be defined.\n");
        errorCount++;
    }
    else if (entry->subkind.decl != FuncK)
    {
        printf("ERROR(LINKER): A function named 'main()' must be defined.\n");
        errorCount++;
    }
    else
    {
        for (int i = 0; i < MAXCHILDREN; i++)
        {
            if (entry->child[i] != NULL)
                count++;
        }

        if (count > 1)
        {
            printf("ERROR(LINKER): A function named 'main()' must be defined.\n");
            errorCount++;
        }
    }
}

int scope_count = 0;

void symtabTraverse(TreeNode *tree, bool isFunc)
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
                symtabTraverse(tree->child[i], isFunc);
            }
        }
        else if (tree->nodekind == StmtK)
        {
            switch (tree->subkind.stmt)
            {
            case IfK:
                break;
            case ReturnK:
                if (tree->child[0] != NULL && tree->child[0]->subkind.exp == IdK)
                {
                    TreeNode *entry = (TreeNode *)symtab.lookup(tree->child[0]->attr.name);
                    if (entry != NULL)
                    {
                        entry->isUsed = true;
                        if (entry->isArray)
                        {
                            printf("ERROR(%d): Cannot return an array.\n", tree->lineno);
                            errorCount++;
                        }
                    }
                }
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc);
                }
                break;
            case BreakK:
                break;
            case RangeK:
                //isScope = true;
                //symtab.enter(std::string("Range-" + std::to_string(scope_count++)));
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    if (tree->child[i] != NULL && tree->child[i]->subkind.exp == IdK)
                    {
                        tree->child[i]->expType = UndefinedType;
                    }
                }
                break;
            case ForK:
                //isScope = true;
                //symtab.enter(std::string("For_Loop-" + std::to_string(scope_count++)));
                // branch
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc);
                }
                break;
            case WhileK:
                //isScope = true;
                //symtab.enter(std::string("While_Loop-" + std::to_string(scope_count++)));
                // branch
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc);
                }
                break;
            case CompoundK:
                if (!isFunc)
                {
                    isScope = true;
                    symtab.enter(std::string("Compound_Statment-" + std::to_string(scope_count++)));
                }
                else
                {
                    isFunc = false;
                }
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc);
                }
                // branch
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
            {
                TreeNode *lhs = tree->child[0];
                TreeNode *rhs = tree->child[1];
                bool undeclared = false;
                bool array_indexed = false;

                if (intOps.find(tree->attr.name) != intOps.end())
                    tree->expType = Integer;

                if (tree->child[0] != NULL)
                {
                    // leaf
                    if (tree->child[0]->subkind.exp == IdK || tree->child[0]->subkind.exp == CallK)
                    {
                        if (!proccessID(tree->child[0], true))
                        {
                            proccessOP(tree, 0);
                            TreeNode *entry = (TreeNode *)symtab.lookup(tree->child[0]->attr.name);
                            lhs = entry;

                            if (!lhs->isArray && arrayOps.find(tree->attr.name) != arrayOps.end())
                            {
                                printf("ERROR(%d): The operation '%s' only works with arrays.\n", tree->lineno, tree->attr.name);
                                errorCount++;
                            }
                        }
                        else
                            undeclared = true;

                        if (tree->attr.name[0] == '[' && undeclared)
                        {
                            printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, tree->child[0]->attr.name);
                            errorCount++;
                        }
                    }
                    // is an array
                    else if (tree->child[0]->subkind.exp == OpK && tree->child[0]->attr.name[0] == '[')
                    {
                        if (!proccessID(tree->child[0]->child[0], true))
                        {
                            proccessOP(tree->child[0], 0);
                            lhs = (TreeNode *)symtab.lookup(tree->child[0]->child[0]->attr.name);
                            tree->expType = lhs->expType;
                            if (tree->child[0]->child[1]->expType == Integer)
                                array_indexed = true;   
                        }                
                        else
                            undeclared = true;
                    }
                    else if (tree->child[0]->subkind.exp == ConstantK)
                    {
                        if (tree->child[0]->expType != tree->expType && unaryOps.find(tree->attr.name) != unaryOps.end())
                        {
                            printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(tree->child[0]->expType));
                            errorCount++;
                        }
                        else if (tree->child[0]->expType != tree->expType && sameOps.find(tree->attr.name) == sameOps.end())
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(tree->child[0]->expType));
                            errorCount++;
                        }
                    }
                    else
                        symtabTraverse(tree->child[0], isFunc);
                    // possible branch
                }
                // rhs
                if (tree->child[1] != NULL)
                {
                    // leaf
                    if (tree->child[1]->subkind.exp == IdK)
                    {
                        if (!proccessID(tree->child[1], true))
                        {
                            proccessOP(tree, 1);
                            rhs = (TreeNode *)symtab.lookup(tree->child[1]->attr.name);
                        }
                        else
                            undeclared = true;
                    }
                    // is an array
                    else if (tree->child[1]->subkind.exp == OpK && tree->child[1]->attr.name[0] == '[')
                    {
                        if (!proccessID(tree->child[1]->child[0], false))
                        {
                            proccessOP(tree->child[1], 0);
                            rhs = (TreeNode *)symtab.lookup(tree->child[1]->child[0]->attr.name);
                            if (tree->child[1]->child[1]->expType == Integer)
                                array_indexed = true;
                        }
                        else
                            undeclared = true;
                    }
                    else
                        symtabTraverse(tree->child[1], isFunc);
                    // possible branch
                }
                if (lhs != NULL && rhs != NULL)
                {
                    if (sameOps.find(tree->attr.name) != sameOps.end())
                    {
                        if (lhs->isArray != rhs->isArray && !array_indexed)
                        {
                            printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is %s and rhs is %s.\n",
                                   tree->lineno, tree->attr.name, (lhs->isArray ? "an array" : "not an array"), (rhs->isArray ? "an array" : "not an array"));
                            errorCount++;
                        }
                        if (lhs->expType != rhs->expType && !undeclared)
                        {
                            printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n",
                                   tree->lineno, tree->attr.name, printType(lhs->expType), printType(rhs->expType));
                            errorCount++;
                        }
                    }
                    else if ((lhs->isArray || rhs->isArray) && !array_indexed && arrayOps.find(tree->attr.name) == arrayOps.end() && tree->attr.name[0] != '[')
                    {
                        printf("ERROR(%d): The operation '%s' does not work with arrays.\n", tree->lineno, tree->attr.name);
                        errorCount++;
                    }
                }
            }
            break;
            case IdK:
                // leaf
                proccessID(tree, true);
                break;
            case CallK:
                // branch
                if (!proccessID(tree, false))
                {
                    TreeNode *entry = (TreeNode *)symtab.lookup(tree->attr.name);

                    if (entry->subkind.decl == VarK || entry->subkind.decl == ParamK)
                    {
                        printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", tree->lineno, tree->attr.name);
                        errorCount++;
                    }
                }
                for (int i = 0; i < MAXCHILDREN; i++)
                {
                    symtabTraverse(tree->child[i], isFunc);
                }
                break;
            case AssignK:
            {
                TreeNode *lhs = tree->child[0];
                TreeNode *rhs = tree->child[1];
                bool undeclared = false;
                bool array_indexed = false;

                if (intOps.find(tree->attr.name) != intOps.end())
                    tree->expType = Integer;

                if (tree->child[0] != NULL)
                {
                    if (tree->child[0]->subkind.exp == IdK)
                    {
                        if (!proccessID(tree->child[0], false))
                        {
                            lhs = (TreeNode *)symtab.lookup(tree->child[0]->attr.name);

                            if (sameOps.find(tree->attr.name) != sameOps.end())
                                tree->expType = lhs->expType;

                            if (lhs->subkind.decl == FuncK)
                            {
                                printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tree->lineno, lhs->attr.name);
                                undeclared = true;
                                errorCount++;
                            }
                            else if (tree->child[0]->expType != tree->expType && unaryOps.find(tree->attr.name) != unaryOps.end())
                            {
                                printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(tree->child[0]->expType));
                                undeclared = true;
                                errorCount++;
                            }
                            else
                            {
                                lhs->isInitialized = true;
                            }
                        }
                        else
                            undeclared = true;
                    }
                    // is an array
                    else if (tree->child[0]->subkind.exp == OpK && tree->child[0]->attr.name[0] == '[')
                    {
                        if (!proccessID(tree->child[0]->child[0], false))
                        {
                            lhs = (TreeNode *)symtab.lookup(tree->child[0]->child[0]->attr.name);
                            lhs->isInitialized = true;
                            if (sameOps.find(tree->attr.name) != sameOps.end())
                                tree->expType = lhs->expType;
                            array_indexed = nestedArr(tree->child[0]);
                        }
                        else
                        {
                            printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, tree->child[0]->child[0]->attr.name);
                            undeclared = true;
                            errorCount++;
                        }
                    }
                    else
                        symtabTraverse(tree->child[0], isFunc);

                    if (lhs->expType != tree->expType && sameOps.find(tree->attr.name) == sameOps.end() && !undeclared)
                    {
                        printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(lhs->expType));
                        errorCount++;
                    }
                }

                if (tree->child[1] != NULL)
                {
                    if (tree->child[1]->subkind.exp == IdK)
                    {
                        if (!proccessID(tree->child[1], true))
                        {
                            rhs = (TreeNode *)symtab.lookup(tree->child[1]->attr.name);
                        }
                        else
                            undeclared = true;
                    }
                    // is an array
                    else if (tree->child[1]->subkind.exp == OpK && tree->child[1]->attr.name[0] == '[')
                    {
                        if (!proccessID(tree->child[1]->child[0], true))
                        {
                            rhs = (TreeNode *)symtab.lookup(tree->child[1]->child[0]->attr.name);
                        }
                        else
                            undeclared = true;
                    }
                    else
                        symtabTraverse(tree->child[1], isFunc);

                    if (rhs->expType != tree->expType && sameOps.find(tree->attr.name) == sameOps.end())
                    {
                        printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(rhs->expType));
                        errorCount++;
                    }
                }
                if (lhs != NULL && rhs != NULL)
                {
                    if (sameOps.find(tree->attr.name) != sameOps.end())
                    {
                        if (lhs->isArray != rhs->isArray && !array_indexed)
                        {
                            printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is %s and rhs is %s.\n",
                                   tree->lineno, tree->attr.name, (lhs->isArray ? "an array" : "not an array"), (rhs->isArray ? "an array" : "not an array"));
                            errorCount++;
                        }
                        else if (lhs->expType != rhs->expType && !undeclared)
                        {
                            printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n",
                                   tree->lineno, tree->attr.name, printType(lhs->expType), printType(rhs->expType));
                            errorCount++;
                        }
                    }
                }
            }
            break;
            case InitK:
                break;
            case ConstantK:
                break;
            default:
                break;
            }
        }

        if (symtab.depth() > 1 && isScope)
        {
            symtab.applyToAll(checkUsed);
            symtab.leave();
        }

        tree = tree->sibling;
    }
}

bool proccessID(TreeNode *tree, bool warn)
{
    bool errored = false;
    TreeNode *entry = (TreeNode *)symtab.lookup(tree->attr.name);
    TreeNode *currEntry = (TreeNode *)symtab.lookupCurrent(tree->attr.name);
    if (entry == NULL)
    {
        printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->attr.name);
        errorCount++;
        errored = true;
    }
    else if (warn && currEntry != NULL && currEntry->subkind.decl == VarK && !currEntry->isInitialized && !currEntry->isStatic && !currEntry->isUsed)
    {
        printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->attr.name);
        warningCount++;
    }
    else
    {
        tree->expType = entry->expType;
    }

    if (entry != NULL)
        entry->isUsed = true;
    if (currEntry != NULL)
        currEntry->isUsed = true;

    return errored;
}

void proccessOP(TreeNode *tree, int child_no)
{
    TreeNode *child = (TreeNode *)symtab.lookup(tree->child[child_no]->attr.name);
    child->isUsed = true;
    if (tree->isInitialized)
        child->isInitialized;
    if (child->subkind.decl == FuncK && tree->child[0]->subkind.exp != CallK)
    {
        printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tree->lineno, child->attr.name);
        errorCount++;
    }
    if (sameOps.find(tree->attr.name) != sameOps.end())
    { /* skip, this is handled earlier */
        return;
    }
    else if (tree->attr.name[0] == '[')
    {
        if (!child->isArray && child_no == 0)
        {
            printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, child->attr.name);
            errorCount++;
        }
        if (child_no == 1 && child->expType != Integer)
        {
            printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", tree->lineno, child->attr.name, printType(child->expType));
            errorCount++;
        }
    }
    else if (child_no == 0)
    {
        if (child->expType != tree->expType && unaryOps.find(tree->attr.name) != unaryOps.end())
        {
            printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(child->expType));
            errorCount++;
        }
        else if (child->expType != tree->expType && arrayOps.find(tree->attr.name) == arrayOps.end() && unaryOps.find(tree->attr.name) == unaryOps.end() && sameOps.find(tree->attr.name) == sameOps.end())
        {
            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(child->expType));
            errorCount++;
        }
        if (child->isArray && unaryOps.find(tree->attr.name) != unaryOps.end())
        {
            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", tree->lineno, tree->attr.name);
            errorCount++;
        }
    }
    else if (child_no == 1)
    {
        if (child->expType != tree->expType)
        {
            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", tree->lineno, tree->attr.name, printType(tree->expType), printType(child->expType));
            errorCount++;
        }
    }
}