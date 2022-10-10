#include <stdio.h>
#include "semantic.h"

extern int errorCount;
extern int warningCount;
extern int line;

void printErrorMessage(int index, char *string1, char *string2, char *string3, int linenum)
{
    switch (index)
    {
    case 0:
        errorCount++;
        printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", linenum, string1);
        break;
    case 1:
        errorCount++;
        printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is %s an array and rhs is %s an array.\n", linenum, string1, string2, string3);
        break;
    case 2:
        errorCount++;
        printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", linenum, string1, string2, string3);
        break;
    case 3:
        errorCount++;
        printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", linenum, string1, string2, string3);
        break;
    case 4:
        errorCount++;
        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", linenum, string1, string2, string3);
        break;
    case 5:
        errorCount++;
        printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", linenum, string1, string2);
        break;
    case 6:
        errorCount++;
        printf("ERROR(%d): Array index is the unindexed array '%s'.\n", linenum, string1);
        break;
    case 7:
        errorCount++;
        printf("ERROR(%d): Cannot index nonarray '%s'.\n", linenum, string1);
        break;
    case 8:
        errorCount++;
        printf("ERROR(%d): Cannot return an array.\n", linenum);
        break;
    case 9:
        errorCount++;
        printf("ERROR(%d): Cannot use function '%s' as a variable.\n", linenum, string1);
        break;
    case 10:
        errorCount++;
        printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", line, string1, linenum);
        break;
    case 11:
        errorCount++;
        printf("ERROR(%d): Symbol '%s' is not declared.\n", linenum, string1);
        break;
    case 12:
        errorCount++;
        printf("ERROR(%d): The operation '%s' does not work with arrays.\n", linenum, string1);
        break;
    case 13:
        errorCount++;
        printf("ERROR(%d): The operation '%s' only works with arrays.\n", linenum, string1);
        break;
    case 14:
        errorCount++;
        printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", linenum, string1, string2, string3);
        break;
    case 15:
        errorCount++;
        printf("ERROR(LINKER): A function named 'main()' must be defined.\n");
        break;
    case 16:
        warningCount++;
        printf("WARNING(%d): The variable '%s' seems not to be used.\n", linenum, string1);
        break;
    case 17:
        warningCount++;
        printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", linenum, string1);
        break;
    default:
        break;
    }
}