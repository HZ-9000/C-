typedef int OpKind;
enum NodeKind {DeclK, StmtK, ExpK};
enum DeclKind {VarK, FuncK, ParamK};
enum StmtKind {NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK};
enum ExpKind {OpK, ConstantK, IdK, AssignK, InitK, CallK};
enum ExpType {Void, Integer, Boolean, Char, CharInt, Equal, UndefinedType};
enum VarKind {None, Local, Global, Parameter, LocalStatic};

#define MAXCHILDREN 3

typedef struct treeNode
{
    // connectivity in the tree
    struct treeNode *child[MAXCHILDREN]; // children of the node
    struct treeNode *sibling; // siblings for the node
    // what kind of node
    int lineno; // linenum relevant to this node
    NodeKind nodekind; // type of this node
    union // subtype of type
    {
        DeclKind decl; // used when DeclK
        StmtKind stmt; // used when StmtK
        ExpKind exp; // used when ExpK
    } subkind;
    // extra properties about the node depending on type of the node
    union // relevant data to type -> attr
    {
        OpKind op; // type of token (same as in bison)
        int value; // used when an integer constant or boolean
        unsigned char cvalue; // used when a character
        char *string; // used when a string constant
        char *name; // used when IdK
    } attr;
    ExpType expType; // used when ExpK for type checking
    bool isArray = false; // is this an array
    bool isStatic = false; // is staticly allocated?
    bool isInitialized = false;
    bool isUsed = false;
    bool warned = false;
    VarKind var = None;
} TreeNode;

TreeNode * addStmtNode(StmtKind kind, int lineno);

TreeNode * addDeclNode(DeclKind kind, int lineno);

TreeNode * addExpNode(ExpKind kind, int lineno, ExpType type);
    
// Adds a TreeNode to a list of siblings.
// Adding a NULL node to the sibling list is probably a programming error!
TreeNode *addSibling(TreeNode *t, TreeNode *s);

// Passes the isStatic and type attributes down the sibling list.
ExpType setType(int count);

void printTree(TreeNode *t, int count, bool types);