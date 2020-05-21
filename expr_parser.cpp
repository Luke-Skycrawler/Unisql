#include <iostream>
#include <string>
#include <regex>
#include <stack>
#define _DEBUG_1
using namespace std;
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
// class var{
//     int type;
//     string code_name;
// }
const char *preserved[]={"between","and","or","select","from","where","create","update","delete","drop"},
    *binary_operator[]={"+","-","*","/","<","=",">","<=",">=","!="};
inline int PRE_COUNT(){
    return sizeof(preserved)/sizeof(char *); 
}
int legimate_variable(const string &name){
    regex var("([_a-z][_a-z0-9]*)");
    if(regex_match(name,var)){
        for(int i=0;i<PRE_COUNT();i++)if(name==preserved[i])return 0;
        return 1;
    }
    return 0;
}
int legimate_const(const string &value){
    // no blank
    if((value.front()=='"'&&value.back()=='"')||(value.front()=='\''&&value.back()=='\''))
        return 1;
        //TODO:  type=string, bla
    regex lib("(([0-9])+(\.[0-9]+)?)");
    if(regex_match(value,lib))return 1;
    return 0;
}
class expr{
    public:
    expr(const string &_s):s(_s){}
    const string &s;
    int evaluate() const;
    int parse() const;
};
// regex lib("([_a-z][_a-z0-9]*)|(([0-9])+(\.[0-9]+)?)"),binary("\*|\+|-|/|=");
stack<string> op_stack;
stack<string> lib_stack;
// not mature
int expr::parse() const{
    // search for binary
    int t=this->s.find_first_of(OPERATORS,0);
    if(t!=string::npos){
        int nextphrase=t+1;
        while(nextphrase<s.length()&&isoperator(s[nextphrase]))nextphrase++;
        expr x0(s.substr(0,t)),x1(s.substr(nextphrase));
        // TODO: push operators to stack 
        op_stack.push(s.substr(t,nextphrase-t));
        cout<<op_stack.top();
        if(x0.parse()&&x1.parse())return 1;
        return 0;
    }
    int u=this->s.find(" between ");
    int v=this->s.find(" and ",u);
    // should find and after between
    if(u!=string::npos)
        if(v!=string::npos){
            #ifdef _DEBUG_1
                cout<<u<<" "<<v;
            #endif
            expr t0(s.substr(0,u)),t1(s.substr(u+9,v-u-9)),t2(s.substr(v+5));
            op_stack.push("between");
            #ifdef _DEBUG_1
                cout<<"between"<<endl;
            #endif
            if(t0.parse()&&t1.parse()&&t2.parse()){
                return 1;
            }
            else return 0;
        }
        else return 0;
    int start=0,end=s.length()-1;
    while(isblank(s[start]))start++;
    while(isblank(s[end]))end--;
    const string &s_noblank=s.substr(start,end-start+1);
    #ifdef _DEBUG_1
    cout<<endl<<"#"+s_noblank;
    #endif
    // possible mess-up
    if(legimate_const(s_noblank))
        lib_stack.push(s_noblank);
    else if(legimate_variable(s_noblank))
        lib_stack.push(s_noblank);
    else return 0;
    return 1;
    // TODO: unary operators not considered
    #ifdef _DEBUG_0
    for(int i=0;i<OP_COUNT;i++){
        int t=this->s.find_first_of(OPERATORS,0);

        // int t=this->s.find_first_of(OPERATORS[i],0);
        if(t!=string::npos)
            cout<<t<<endl; 
        else cout<<"#"<<endl;
        // send the previous and following to parser
    }
    TODO: cout>>
    #endif
}
int main(void){
    string s;
    getline(cin,s);
    expr testpaser(s);
    if(testpaser.parse())cout<<endl<<"great";
        else cout<<endl<<"bad";
    system("pause");
    return 0;
}
