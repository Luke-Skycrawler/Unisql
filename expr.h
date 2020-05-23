#ifndef EXPR_H
#define EXPR_H
#include <string>
inline int proper_name(char c){
    return ((c<='z'&&c>='a')||(c<='Z'&&c>='A')||(c=='_'));
}
inline int isblank(char c){
    if(c!=' '&&c!='\t'&&c!='\n')return 0;
    return 1;
}
static const char OPERATORS[]="+-*/<>!=";
#define OP_COUNT sizeof(OPERATORS)-1
inline int isoperator(char c){
    for(int i=0;i<OP_COUNT;i++)if(c==OPERATORS[i])return 1;
    return 0;
}
const char *preserved[]={"between","and","or","select","from","where","create","update","delete","drop"},
    *binary_operator[]={"+","-","*","/","<","=",">","<=",">=","!="," and "," or "};
inline int PRE_COUNT(){
    return sizeof(preserved)/sizeof(char *); 
}
class expr{
    public:
    expr(const std::string &_s):s(_s){}
    const std::string s;
    int parse(int offset=0) const;
    int balance(int start=0,int end=std::string::npos) const;
};

enum{
    ACCEPTED=1,BRACKET_LEFT,BRACKET_RIGHT,INVALID_LITERAL,BETWEEN_NOT_MATCHED,EXTRA_BRACKET,INVALID_OPERATOR
};

#endif