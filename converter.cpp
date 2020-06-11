#include "pch.h"
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include "expr.h"
// #define _DEBUG_0
// #define _DEBUG_1
using namespace std;
stack<string> ops;
extern map<string, TABLE> tables;
extern string new_table, new_attribute;
extern stack<Block> workslab;
map<string, Block> search_list;
Block syntax_tree[MAX_QUERY_LENGTH], update_exclusive[20];
int cursor = -1, ucursor = -1;
// empty=-1
// framework like this
extern vector<string> table_list;
map<string, LOOK_UP_TABLE> attribute_lookup;
// comment : translate the alias to the real attribute name and parent table
int update_statement = 0;

int type_check();

int output(const string &s) {
  if (s[0] == '(' || s[0] == ')') return 0;
#ifdef _DEBUG_0
  cout << s << endl;
#else
  if(update_statement)
	  update_exclusive[++ucursor]=s;
  else 
  syntax_tree[++cursor].content = s;
// operators
#endif
  return 0;
}
#ifndef _DEBUG_0
template <typename T>
int output(const string &s, const T &data) {
  Block *tree;
  int *cur;
  if (!update_statement) {
    tree = syntax_tree;
    cur = &cursor;
  } else {
    cur = &ucursor;
    tree = update_exclusive;
  }
  tree[++(*cur)].content = s;
  // FIXME: leaking
  tree[*cur].p = static_cast<T *>(new T(data));
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
  if (ops.empty()) return;
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
  Block *tree;
  int n;
  if (!update_statement) {
    tree = syntax_tree;
    n = cursor;
  } else {
    tree = update_exclusive;
    n = ucursor;
  }
  for (int i = 0; i <= n; i++)
    if (tree[i].content == "v") {
      string &a = *static_cast<string *>(tree[i].p), &t = tree[i].content;
      if (!attribute_lookup.count(a)) return 0;
      t.append(tables[attribute_lookup[a].parent].collum[a].type);
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
      // elegent but confusing, means already caused a pop on "b3" and got "ba"
      // on top, thus needs to break
    }
  }
  if (x != ')' && x != 'a')
    push(x, y);
  else if (x == 'a' && ops.top() != "ba")
    push(x, y);
  return 0;
}
int expr::convert(int update) const {
  int i = 0;
  while (!ops.empty()) ops.pop();
  if (update) {
    update_statement = 1;
    ucursor = -1;
  } else
    cursor = -1;
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
    unsigned j = s.find_first_of("+-*/<>!=) ", i);
    if (j == string::npos) j = n;
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
    i = skip(i);
    if (i >= n) break;
    if (s[i] == 'a' || s[i] == 'b' || s[i] == 'o') {
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
  } while (1);
  put(')');
  if (!ops.empty()) return 0;
  if (!find_attributes() || !fetch_type()) {
    update_statement = 0;
    return 0;
  }
  if (!type_check()) {
    update_statement = 0;
    return 0;
  }
#ifdef _DEBUG_1
  cout << endl << "evaluation=" << evaluate(update) << endl;
#endif
  update_statement = 0;
  return 1;
}
// TODO: simplify the expressions statically, compute the consts
int rooted_at(const int i) {
  int order;
  string &a = syntax_tree[i].content;
  if (a[0] == 'v' || a[0] == 'c') return i;
  if (a[0] == 'b')
    order = 3;
  else if (a[1] == '1' || a[1] == '2')
    order = a[1] - '0';
  else
    order = 2;
  int j = i - 1;
  while (order--) {
    j = rooted_at(j) - 1;
  }
  return j + 1;
}
int simplify() {
  // must be careful because integer divide isn't the reverse of multiply
  // the true reverse of x/b=a is ab<=x<(a+1)b
  // steps:1. cut into child trees
  // 2. count the variables recursively, if >=2 we are done for
  // 3. move the siblings (and the parent) of the variable node to the side of
  // the parent's sibling (which shall be a tree made up of consts) base
  // base scienarios first
  if (cursor == -1) return 0;
  search_list.clear();
  int i=0;
  for(int k=0;k<cursor;k++){
	if(syntax_tree[k].content[0]=='v')i++;
  }
  if(i==0){
	if(evaluate(syntax_tree,0,cursor))return -1;
	else return -2;
  }
  if (syntax_tree[cursor].content[0] == '=') {
    int r = rooted_at(cursor - 1), lc = 0, rc = 0;
    for (i = 0; i < r; i++)
      if (syntax_tree[i].content[0] == 'v')
        if (lc < 2) lc++;
    for (i = r; i < cursor; i++)
      if (syntax_tree[i].content[0] == 'v' && rc < 2) rc++;
    // return -1 for const
	char type;
    if (lc + rc < 2) {
      void *p;
      if (lc == 0) {
        p = syntax_tree[r].p;
		type=syntax_tree[r].content[1];
        evaluate(syntax_tree, 0, r - 1);
      } else {
        evaluate(syntax_tree, r, cursor - 1);
        p = syntax_tree[r - 1].p;
		type=syntax_tree[r-1].content[1];
      }
      Block &result = workslab.top();
	  if(type!=result.content[1]){
		if(type=='d')result.p=new double(*static_cast<int*>(result.p));
		else return -2;
		// straight
	  }
      if (lc == 0) {
        syntax_tree[0] = result;
        for (int k = 1; k <= cursor + 1 - r; k++)
          syntax_tree[k] = syntax_tree[k + r - 1];
        cursor -= r - 1;
      } else {
        syntax_tree[r] = result;
        syntax_tree[r + 1] = syntax_tree[cursor];
        cursor = r + 1;
      }
      if ((lc == 0 && syntax_tree[i - 1].content[0] == 'v') ||
          (rc == 0 && r == 1)) {
        search_list[*static_cast<string *>(p)] = result;
        return 1;
        // return 1 if it is base situation
      }
      return 2;
    } else
      return 0;
  }
  return 0;
}
stack<char> checkslab;
inline int isN(char c) { return (c == 'i' || c == 'd'); }
int type_check() {
  int n;
  Block *tree;
  if (!update_statement) {
    tree = syntax_tree;
    n = cursor;
  } else {
    n = ucursor;
    tree = update_exclusive;
  }
  while (!checkslab.empty()) checkslab.pop();
  for (int i = 0; i <= n; i++) {
    string &a = tree[i].content;
    if (r(a[0]) >= 0) {
      // operator
      int order;
      if (a[1] > '0' && a[1] <= '2')
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
        } else
          return 0;
      } else
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
  for (auto it = Pi.begin(); it != Pi.end(); it++) attribute_lookup[*it];

  for (auto jt = attribute_lookup.begin(); jt != attribute_lookup.end(); jt++) {
    for (auto it = table_list.begin(); it != table_list.end(); it++)
      if (tables.count(*it) && tables[*it].collum.count(jt->first)) {
        // if(jt->parent=="")
        jt->second.parent = *it;
        break;
      }
    if (jt->second.parent == "") return 0;
  }
  return 1;
}
