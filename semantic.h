#include "symbolTable.h"
#include "AST.h"

void semanticAnalysis(TreeNode *tree, bool debug);
void symtabTraverse(TreeNode *tree);
void proccessID(TreeNode *tree);
void proccessOP(TreeNode *tree);
void typeTraverse(TreeNode *tree);
void printErrorMessage(int index, char *string1, char *string2, char *string3, int linenum);