#ifndef SCANTYPE_H
#define SCANTYPE_H

struct TokenData {
    int tokenclass;
    int linenum;
    char *tokenstr;
    char cvalue;
    int nvalue;
    char *strvalue;
    char *svalue;
};

#endif
