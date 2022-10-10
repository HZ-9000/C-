#include "symbolTable.h"
#include "AST.h"

void semanticAnalysis(TreeNode *tree, bool debug);
void symtabTraverse(TreeNode *tree, bool isFunc);
bool proccessID(TreeNode *tree, bool warn);
void proccessOP(TreeNode *tree, int child_no);
void typeTraverse(TreeNode *tree);