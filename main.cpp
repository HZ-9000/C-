#include  "scanType.h"
#include  "AST.h"
#include  <stdio.h> 
#include  <string.h> 

extern TreeNode *tree;
extern int yyparse();
extern FILE *yyin;
extern int yydebug;
extern int numErrors;

int main(int argc, char *argv[])
{
    int i;
    bool printAST = false;
    if(argc > 1){
        if(argc >= 2)
        {
            for(i=0; i < argc; i++)
            {
                if(strcmp(argv[i], "-d") == 0)
                {
                    yydebug = 1;
                }
                else if(strcmp(argv[i], "-h") == 0)
                {
                    printf("usage: -c [options] [sourcefile]\n");
                    printf("options:\n");
                    printf("-d          - turn on parser debugging\n");
                    printf("-h          - print this usage message\n");
                    printf("-p          - print the abstract syntax tree\n");
                    return 0;
                }
                else if(strcmp(argv[i], "-p") == 0)
                {
                    printAST = true;
                }
            }
        }
        if((yyin = fopen(argv[argc-1], "r"))){
                //success
        }
        else
        {
            //fail
            printf("ERROR: failed to open \'%s\'\n", argv[1]);
            return 0;
        }
    }

    numErrors = 0;
    yyparse();
    if(printAST)
        printTree(tree, 0);
    return 0;
}