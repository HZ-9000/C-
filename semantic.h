#include "symbolTable.h"
#include "AST.h"
#include "emitcode.h"

void code_generation(TreeNode *tree);

void semanticAnalysis(TreeNode *tree, bool debug);
void symtabTraverse(TreeNode *tree, bool isFunc, bool warn);
void proccessID(TreeNode *tree, bool warn);
void proccessCALL(TreeNode *tree);
void proccessOP(TreeNode *tree);
void proccessASSNG(TreeNode *tree, bool isFunc);
void proccessLandR(TreeNode *tree);
void typeTraverse(TreeNode *tree);