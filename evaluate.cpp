#include "pch.h"
#include <iostream>
#include <stack>
#include "expr.h"
//#define _DEBUG_0
using namespace std;
extern int cursor, ucursor;
int r(const char c);
extern Block syntax_tree[MAX_QUERY_LENGTH], update_exclusive[20];
stack<Block> workslab;
extern map<string, TABLE> tables;
extern map<string, LOOK_UP_TABLE> attribute_lookup;
template <typename T>
Block::Block(const T &a) : p(new T(a)) {
  if (is_same<T, string>::value)
    content = "cs";
  else if (is_same<T, double>::value)
    content = "cd";
  else if (is_same<T, int>::value)
    content = "ci";
}
char *current_record = NULL;

void replace_with_const(Block &a, string *p) {
  a.content.replace(0, 1, "c");
  string att = *static_cast<string *>(a.p);
  int j = tables[attribute_lookup[att].parent].collum[att].offset;
  // cout<<*(int*)(current_record);
  if (a.content[1] == 's') {
    p = new string(current_record + j);
    a.p = p;
  } else
    a.p = current_record + j;
  // cout<<*(int *)(a.p)<<endl;
}

int evaluate(Block *tree,const int start,const int root) {
  // return 1 if satisfied; -1 error
  while (!workslab.empty()) workslab.pop();
  // clean the workslab
  for (int i = start; i <= root; i++) {
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
      Block b[3];
      string *p[3];
      for (int k = 0; k < order; k++) {
        b[k] = workslab.top();
        if (b[k].content[0] == 'v') replace_with_const(b[k], p[k]);
        workslab.pop();
      }
      if (a == "-1") {
        // workslab.push(-b[0]->p);
        if (b[0].content[1] == 'i') {
          tmp_class<int> tmp(b[0].p);
          workslab.push(Block(-tmp.data));
        } else if (b[0].content[1] == 'd') {
          tmp_class<double> tmp(b[0].p);
          workslab.push(Block(-tmp.data));
        }
        continue;
        // FIXME: int not sure, fixed
      }
      if (a[0] == 'b') {
        // TODO: between support
        // b[0] is the latest push
        if (b[0].content[1] == 's') {
          tmp_class<string> tmp[3] = {b[2].p, b[1].p, b[0].p};
          workslab.push(
              (tmp[0].data <= tmp[1].data && tmp[0].data >= tmp[2].data) ||
              (tmp[0].data <= tmp[2].data && tmp[0].data >= tmp[1].data));
        } else {
          double d[3];
          for (int k = 0; k < 3; k++)
            if (b[k].content[1] == 'i')
              d[k] = static_cast<double>(tmp_class<int>(b[2 - k].p).data);
            else
              d[k] = tmp_class<double>(b[2 - k].p).data;
          workslab.push((d[0] - d[1]) * (d[0] - d[2]) <= 0);
        }
        continue;
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
        // TODO: between, fixed
      }
#ifdef _DEBUG_0
      Block &top = workslab.top();
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
      workslab.push(tree[i]);
  }
  if(tree==syntax_tree)return tmp_class<int>(workslab.top().p).data;
  return 0;
}
