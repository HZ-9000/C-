#include "scanType.h"
#include "symbolTable.h"
#include "semantic.h"
#include "yyerror.h"
#include <stdio.h>
#include <string.h>

extern TreeNode *tree;
extern int yyparse();
extern FILE *yyin;
extern int yydebug;
extern int numErrors;
extern int warningCount;

int main(int argc, char *argv[])
{
    int i;
    bool printAST = false;
    bool printASTAnnotated = false;
    bool symTabDebug = false;
    bool printMemory = false;
    initErrorProcessing();
    if (argc > 1)
    {
        if (argc >= 2)
        {
            for (i = 0; i < argc; i++)
            {
                if (strcmp(argv[i], "-d") == 0)
                {
                    yydebug = 1;
                }
                else if (strcmp(argv[i], "-D") == 0)
                {
                    symTabDebug = true;
                }
                else if (strcmp(argv[i], "-h") == 0)
                {
                    printf("usage: -c [options] [sourcefile]\n");
                    printf("options:\n");
                    printf("-d          - turn on parser debugging\n");
                    printf("-D          - turn on symbol table debugging\n");
                    printf("-h          - print this usage message\n");
                    printf("-p          - print the abstract syntax tree\n");
                    printf("-P          - print the abstract syntax tree plus type information\n");
                    printf("-M          - print the abstract syntax tree plus memory information\n");
                    return 0;
                }
                else if (strcmp(argv[i], "-p") == 0)
                {
                    printAST = true;
                }
                else if (strcmp(argv[i], "-P") == 0)
                {
                    printASTAnnotated = true;
                    printAST = true;
                }
                else if (strcmp(argv[i], "-M") == 0)
                {
                    printMemory = true;
                    printASTAnnotated = true;
                    printAST = true;
                }
            }
        }
        if ((yyin = fopen(argv[argc - 1], "r")))
        {
            printf("====================================\n");
            printf("FILE: %s\n", argv[argc - 1]);
            yyparse();

            if(numErrors == 0)
            {
                
                semanticAnalysis(tree, symTabDebug);

                if (printAST && numErrors == 0)
                    printTree(tree, 0, printASTAnnotated, printMemory);
            }
        }
        else
        {
            // fail
            printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", argv[argc-1]);
            numErrors++;
        }
    }

    printf("Number of warnings: %d\n", warningCount);
    printf("Number of errors: %d\n", numErrors);
    return 0;
}