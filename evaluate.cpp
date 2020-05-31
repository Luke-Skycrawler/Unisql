#include "pch.h"
#include <iostream>
#include <stack>
#include "expr.h"
#define _DEBUG_0
using namespace std;
extern int cursor;
int r(const char c);
extern block syntax_tree[MAX_QUERY_LENGTH];
stack<block> workslab;
extern map<string, TABLE> tables;
extern map<string, LOOK_UP_TABLE> attribute_lookup;
template <typename T>
block::block(const T &a) : p(new T(a)) {
  if (is_same<T, string>::value)
    content = "cs";
  else if (is_same<T, double>::value)
    content = "cd";
  else if (is_same<T, int>::value)
    content = "ci";
}
void *current_record = NULL;

void replace_with_const(block &a) {
  a.content.replace(1, 1, "c");
  string att = *static_cast<string *>(a.p);
  a.p = static_cast<char *>(current_record) +
        tables[attribute_lookup[att].parent].collum[att].offset;
}

int evaluate() {
  // return 1 if satisfied; -1 error
  while (!workslab.empty()) workslab.pop();
  // clean the workslab
  for (int i = 0; i <= cursor; i++) {
    if (r(syntax_tree[i].content[0]) >= 0) {
      // operator
      string &a = syntax_tree[i].content;
      int order;
      if (a[1] > '0' && a[1] <= '2')
        order = a[1] - '0';
      else if (a[0] == 'b')
        order = 3;
      else
        order = 2;
      block b[3];
      for (int k = 0; k < order; k++) {
        b[k] = workslab.top();
        if (b[k].content[0] == 'v') replace_with_const(b[k]);
        workslab.pop();
      }
      if (a == "-1") {
        // workslab.push(-b[0]->p);
        tmp_class<int> tmp(b[0].p);
        workslab.push(block(-tmp.data));
        continue;
		// FIXME: int not sure
      }
      if (b[1].content[1] == 'd') {
        tmp_class<double> t1(b[1].p);
        if (b[0].content[1] == 'd') {
          tmp_class<double> t2(b[0].p);
          switch (a[0]) {
            case '+':
              workslab.push(t1.data + t2.data);
              break;
            case '-':
              workslab.push(t1.data - t2.data);
              break;
            case '*':
              workslab.push(t1.data * t2.data);
              break;
            case '/':
              workslab.push(t1.data / t2.data);
              break;
            case 'a':
              workslab.push(t1.data && t2.data);
              break;
            case 'o':
              workslab.push(t1.data || t2.data);
              break;
            case '=':
              workslab.push(t1.data == t2.data);
              break;
            case '!':
              workslab.push(t1.data != t2.data);
              break;
            case '<':
              if (a[1] == '=')
                workslab.push(t1.data <= t2.data);
              else
                workslab.push(t1.data < t2.data);
              break;
            case '>':
              if (a[1] == '=')
                workslab.push(t1.data >= t2.data);
              else
                workslab.push(t1.data > t2.data);
              break;
          }
        } else if (b[0].content[1] == 'i') {
          tmp_class<int> t2(b[0].p);
          switch (a[0]) {
            case '+':
              workslab.push(t1.data + t2.data);
              break;
            case '-':
              workslab.push(t1.data - t2.data);
              break;
            case '*':
              workslab.push(t1.data * t2.data);
              break;
            case '/':
              workslab.push(t1.data / t2.data);
              break;
            case 'a':
              workslab.push(t1.data && t2.data);
              break;
            case 'o':
              workslab.push(t1.data || t2.data);
              break;
            case '=':
              workslab.push(t1.data == t2.data);
              break;
            case '!':
              workslab.push(t1.data != t2.data);
              break;
            case '<':
              if (a[1] == '=')
                workslab.push(t1.data <= t2.data);
              else
                workslab.push(t1.data < t2.data);
              break;
            case '>':
              if (a[1] == '=')
                workslab.push(t1.data >= t2.data);
              else
                workslab.push(t1.data > t2.data);
              break;
          }
        }
      } else if (b[1].content[1] == 'i') {
        tmp_class<int> t1(b[1].p);
        if (b[0].content[1] == 'd') {
          tmp_class<double> t2(b[0].p);
          switch (a[0]) {
            case '+':
              workslab.push(t1.data + t2.data);
              break;
            case '-':
              workslab.push(t1.data - t2.data);
              break;
            case '*':
              workslab.push(t1.data * t2.data);
              break;
            case '/':
              workslab.push(t1.data / t2.data);
              break;
            case 'a':
              workslab.push(t1.data && t2.data);
              break;
            case 'o':
              workslab.push(t1.data || t2.data);
              break;
            case '=':
              workslab.push(t1.data == t2.data);
              break;
            case '!':
              workslab.push(t1.data != t2.data);
              break;
            case '<':
              if (a[1] == '=')
                workslab.push(t1.data <= t2.data);
              else
                workslab.push(t1.data < t2.data);
              break;
            case '>':
              if (a[1] == '=')
                workslab.push(t1.data >= t2.data);
              else
                workslab.push(t1.data > t2.data);
              break;
          }
        } else if (b[0].content[1] == 'i') {
          tmp_class<int> t2(b[0].p);
          switch (a[0]) {
            case '+':
              workslab.push(t1.data + t2.data);
              break;
            case '-':
              workslab.push(t1.data - t2.data);
              break;
            case '*':
              workslab.push(t1.data * t2.data);
              break;
            case '/':
              workslab.push(t1.data / t2.data);
              break;
            case 'a':
              workslab.push(t1.data && t2.data);
              break;
            case 'o':
              workslab.push(t1.data || t2.data);
              break;
            case '=':
              workslab.push(t1.data == t2.data);
              break;
            case '!':
              workslab.push(t1.data != t2.data);
              break;
            case '<':
              if (a[1] == '=')
                workslab.push(t1.data <= t2.data);
              else
                workslab.push(t1.data < t2.data);
              break;
            case '>':
              if (a[1] == '=')
                workslab.push(t1.data >= t2.data);
              else
                workslab.push(t1.data > t2.data);
              break;
          }
        }
      } else if (b[1].content[1] == 's') {
        tmp_class<string> t1(b[1].p);

        if (b[0].content[1] == 's') {
          tmp_class<string> t2(b[0].p);
          switch (a[0]) {
            case '+':
              workslab.push(t1.data + t2.data);
              break;
            case '=':
              workslab.push(t1.data == t2.data);
              break;
            case '!':
              workslab.push(t1.data != t2.data);
              break;
            case '<':
              if (a[1] == '=')
                workslab.push(t1.data <= t2.data);
              else
                workslab.push(t1.data < t2.data);
              break;
            case '>':
              if (a[1] == '=')
                workslab.push(t1.data >= t2.data);
              else
                workslab.push(t1.data > t2.data);
              break;
          }
        }
		// TODO: between
      }
#ifdef _DEBUG_0
      block &top = workslab.top();
      char c = top.content[1];
      cout << "intermediate result(" + a + ")=";
      switch (c) {
        case 'i':
          cout << tmp_class<int>(top.p).data << endl;
          break;
        case 'd':
          cout << tmp_class<double>(top.p).data << endl;
          break;
        case 's':
          cout << tmp_class<string>(top.p).data << endl;
          break;
        default:
          break;
      }

#endif
    } else
      workslab.push(syntax_tree[i]);
  }
  return tmp_class<int>(workslab.top().p).data;
}
