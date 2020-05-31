#include "pch.h"
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#include "expr.h"
// #define _DEBUG_0
#define _DEBUG_1
using namespace std;
stack<string> ops;
extern map<string, TABLE> tables;
extern string new_table, new_attribute;
block syntax_tree[MAX_QUERY_LENGTH];
int cursor = -1;
// empty=-1
// framework like this
extern vector<string> table_list;
map<string, LOOK_UP_TABLE> attribute_lookup;
// comment : translate the alias to the real attribute name and parent table

int type_check();
int output(const string &s) {
  if (s[0] == '(' || s[0] == ')') return 0;
#ifdef _DEBUG_0
  cout << s << endl;
#else
  syntax_tree[++cursor].content = s;
// operators
#endif
  return 0;
}
#ifndef _DEBUG_0
template <typename T>
int output(const string &s, const T &data) {
  syntax_tree[++cursor].content = s;
  // FIXME: leaking
  syntax_tree[cursor].p = static_cast<T *>(new T(data));
  return 0;
}
#endif
inline int r(char c) {
  switch (c) {
    case '*':
    case '/':
      return 5;
    case '+':
    case '-':
      return 4;
    case '>':
    case '<':
    case '=':
    case '!':
      return 3;
    case 'a':
    case ',':
      return 0;
    case 'o':
      return 2;
    case 'b':
      return 1;
    case ')':
    case '(':
      return -1;
    default:
      return -1;
  }
}
inline void pop() {
  if (ops.top() == "b3") {
    ops.pop();
    ops.push("ba");
  }
  // else if(ops.top()=="ba")
  else {
    output(ops.top());
    ops.pop();
  }
}
inline void push(char x, char y) {
  char c[3];
  c[0] = x;
  c[1] = y;
  c[2] = 0;
  ops.push(string(c));
}
int fetch_type() {
  for (int i = 0; i < cursor; i++)
    if (syntax_tree[i].content == "v") {
      string &a = *static_cast<string*>(syntax_tree[i].p),&t=syntax_tree[i].content;
	  if(!attribute_lookup.count(a))return 0;
      t.append(tables[attribute_lookup[a].parent]
                   .collum[a]
                   .type);
    }
  return 1;
}

int put(char x, char y = 0) {
  if (x != '(') {
    string a = ops.top();
    // cout<<a+"$"<<endl;
    while (r(a[0]) >= r(x)) {
      pop();
      if (a[0] == '(') break;
      a = ops.top();
      if (a == "ba" && x == 'a') break;
    }
  }
  if (x != ')' && x != 'a')
    push(x, y);
  else if (x == 'a' && ops.top() != "ba")
    push(x, y);
  return 0;
}
int find_attributes();
int expr::convert() {
  int i = 0;
  cursor=-1;
  put('(', ' ');
  do {
    // read(+,-,'(');
    int end = 0, n = s.length();
    do {
      i = skip(i);
      switch (s[i]) {
        case '-':
          put('-', '1');
          i++;
          break;
        case '(':
          put('(');
          i++;
          break;
        case '!':
        case '+':
          i++;
          break;
        default:
          end = 1;
          // TODO: to be continued
      }
    } while (!end);
    // read_literal();
    i = skip(i);
    int j = s.find_first_of("+-*/<>!=) ", i);
    if (j == -1) j = n;
    if (s[i] == '"' || s[i] == '\'')
      output("cs", s.substr(i + 1, s.find(s[i], i + 1) - 1 - i));
    else if (s[i] >= '0' && s[i] <= '9') {
      if (exist_point(i, j - 1))
        // FIXME: efficiency
        output("cd", stod(s.substr(i, j - i)));
      else
        output("ci", stoi(s.substr(i, j - i)));
    } else
      output("v", s.substr(i, j - i));
    i = j;

    if (i >= n) break;
    while (s[i] == ')') {
      put(')');
      i = skip(++i);
      // read(char c);
    }
    if (i >= n) break;
    if (s[i] == ' ' || s[i] == 'a' || s[i] == 'b' || s[i] == 'o') {
      i = skip(i);
      if (i >= n) break;
      // read(between,and,or);
      if (s[i] == 'b')
        put('b', '3');
      else
        put(s[i], '2');
      i = j = s.find(' ', i);
    } else {
      if (s[i + 1] == '=')
        put(s[i++], '=');  // different
      else
        put(s[i], '2');
      i++;
    }
    if (i >= n) break;
  } while (1);
  put(')');
  if(!find_attributes()||!fetch_type())return 0;
  if(!type_check())return 0;
#ifdef _DEBUG_1
  cout << endl << "evaluation=" << evaluate() << endl;
#endif
  if (ops.empty())
    return 1;
  else
    return 0;
}
// TODO: simplify the expressions statically, compute the consts
void simplify() {}
stack<char> checkslab;
inline int isN(char c) { return (c == 'i' || c == 'd'); }
int type_check() {
	while(!checkslab.empty())checkslab.pop();
  for (int i = 0; i <= cursor; i++) {
    string &a = syntax_tree[i].content;
    if (r(a[0]) >= 0) {
      // operator
      int order;
      if (a[1] > '0' && a[0] <= '2')
        order = a[1] - '0';
      else if (a[0] == 'b')
        order = 3;
      else
        order = 2;
      char c[3];
      int allN = 1;
      for (int k = 0; k < order; k++) {
        c[k] = checkslab.top();
        if (!(isN(c[k]))) allN = 0;
        checkslab.pop();
      }
      if (allN) {
        checkslab.push('d');
        continue;
      } else if (a[0] == 'b') {
        if (c[0] == 's' && c[1] == 's' && c[2] == 's') {
          checkslab.push('i');
          continue;
        }
      }
      switch (a[0]) {
        case '*':
        case '/':
        case '-':
        case 'a':
        case 'o':
          return 0;
        case '+':
          checkslab.push('s');
          break;
        default:
          checkslab.push('i');
      }
    } else
      checkslab.push(a[1]);
  }
  return 1;
}
extern vector<string> Pi;
int find_attributes() {
	for(auto it =Pi.begin();it!=Pi.end();it++)attribute_lookup[*it];

  for (auto jt = attribute_lookup.begin(); jt != attribute_lookup.end(); jt++){
    for (auto it = table_list.begin(); it != table_list.end(); it++)
      if (tables.count(*it) && tables[*it].collum.count(jt->first)) {
        // if(jt->parent=="")
        jt->second.parent = *it;
        break;
      }
	if(jt->second.parent=="")return 0;
  }
  return 1;
}
