#include <iostream>
#include <string>
#include <regex>
#include <stack>
#include <queue>
#include "expr.h"
#define _DEBUG_1
using namespace std;
stack<int> op_stack;
queue<int> expr_queue;
// not mature
inline int legimate_binary_operator(const std::string &s){
    for(int i=0;i<10;i++)if(s==binary_operator[i])
        return i+1;
    return 0;
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
    if((value.length()>1&&value.front()=='"'&&value.back()=='"')||(value.front()=='\''&&value.back()=='\''))
        return 1;
        //TODO:  type=string, bla
    regex lib("(([0-9])+(\.[0-9]+)?)");
    if(regex_match(value,lib))return 1;
    return 0;
}
int expr::balance(int u,int v)const{
    int l=0,r=0;
    for(int i=u;i<v;i++)
        if(s[i]=='(')l++;
        else if(s[i]==')')
            if(l>r)r++;
            else return 0;
    return (l==r);
}
    
    // search for binary
    // need to check if it starts with unary operators
    // only +,-
    // in the parser phase, ignore all the (, ), unary operators, just count them

    // principle: ignore the blank characters around operators
int expr::parse() const{
    // principles & priorities:
    // 1. ()
    // 2. unary
    // 3. binary, and
    // 4. between and
    // translate to postorder and push to the queue
    // for literals, push to queue without doubt
    // make sure there is no extra () pair at the outer scope
    // lose_extra(s);
    int start=0,end=s.length()-1,syntax_start;
    while(isblank(s[start])||s[start]=='-'||s[start]=='+'||s[start]=='(')start++;
    if(s[start]==')')return 0;
    if(s[start]=='"')syntax_start=s.find("\"",start+1);
    else if(s[start]=='\'')syntax_start=s.find("'",start+1);
    else syntax_start=start;

    int u=this->s.find(" between ",syntax_start);
    int v=u+9;
    do{
        v=this->s.find(" and ",v+4);
    }
    while(v!=string::npos&&!this->balance(u,v));
    // should find and after between
    // bug: "b between a>0 and 1"->"b between (a>0 and 1)"
    // should contain equal number between two key words
    if(u!=string::npos)
        if(v!=string::npos){
            #ifdef _DEBUG_1
                cout<<u<<" "<<v;
            #endif
            expr t0(s.substr(0,u)),t1(s.substr(u+9,v-u-9)),t2(s.substr(v+5));
            #ifdef _DEBUG_1
                cout<<"between"<<endl;
            #endif
            if(t0.parse()&&t1.parse()&&t2.parse())
                return 1;
            else return 0;
        }
        else return 0;

    int t=this->s.find_first_of(OPERATORS,syntax_start);
    if(t!=string::npos){
        int nextphrase=t+1;
        if(s[nextphrase]=='=')nextphrase++;
        // while(nextphrase<s.length()&&isoperator(s[nextphrase]))nextphrase++;
        cout<<s.substr(nextphrase);
        expr x0(s.substr(start,t-start));
        expr x1(s.substr(nextphrase));
        // TODO: push operators to stack 
        int xp0=x0.parse(),xp1=x1.parse();
        // push not required in this phase
        // TODO: throw operator errors 
        int j=legimate_binary_operator(s.substr(t,nextphrase-t));
        if(j==0)return 0;
        #ifdef _DEBUG_1
        cout<<"("<<binary_operator[j-1]<<")"<<endl;
        #endif
        if(xp0&&xp1)return 1;
        return 0;
    }
    // and, or
    for(int i=10;i<12;i++){
        t=s.find(binary_operator[i],syntax_start);
        if(t!=string::npos){
            int nextphrase=t+strlen(binary_operator[i])-1; 
            expr x0(s.substr(start,t-start)),x1(s.substr(nextphrase));
            int xp0=x0.parse(),xp1=x1.parse();
            #ifdef _DEBUG_1
            cout<<'('<<binary_operator[i]<<')'<<endl;
            #endif
            if(xp0&&xp1)return 1;
            return 0;
        }
    }
    while(isblank(s[end])||s[end]==')')end--;
    if(s[end]=='(')return 0;
    const string &s_noblank=s.substr(start,end-start+1);
    #ifdef _DEBUG_1
    cout<<endl<<"#"+s_noblank;
    #endif
    // possible mess-up
    if(legimate_const(s_noblank))
        expr_queue.push(0);
    else if(legimate_variable(s_noblank))
        expr_queue.push(1);
    else return 0;
    return 1;
}
int main(void){
    string s;
    getline(cin,s);
    expr testpaser(s);
    if(testpaser.parse()&&testpaser.balance())cout<<endl<<"great";
        else cout<<endl<<"bad";
    system("pause");
    return 0;
}
