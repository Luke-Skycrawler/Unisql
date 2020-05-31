#include "pch.h"
#include <iostream>
#include <map>
#include <stack>
#include <vector>
#include "expr.h"
#define _DEBUG_0
using namespace std;
inline int ispermitable(char c) {
  switch (c) {
    case '*':
    case ',':
    case ')':
    case '(':
      return 1;
    default:
      return 0;
  }
}
int word::take(const string &s, int &offset) {
  int j = skip(s, offset), i;
  for (i = 0; i < expected_text.length(); i++)
    if (expected_text[i] != s[i + j]) return 0;
  if (!isblank(s[i + j]) && !ispermitable(s[i + j])) return 0;
  offset = i + j;
  return 1;
}
word _natural_join("natural join"), _join("join"), _select("select"),
    _delete("delete"), _update("update"), _table("table"), _index("index"),
    _create("create"), _values("values"), _int("int"), _char("char"),
    _double("double"), _from("from"), _set("set"), _insert_into("insert into");
inline int r_list(char c) {
  switch (c) {
    case ',':
      return 1;
    case 'j':
    case 'n':
      return 2;
    case '(':
    case ')':
      return -1;
    default:
      return 0;
  }
}
vector<string> Pi;
inline int readname(const string &input, int &offset,
                    void (*f)(const string &input) = NULL) {
  // no space at front
  int x = input.find_first_of(", (", offset);
#ifdef _DEBUG_0
  cout << "#" + input.substr(offset, x - offset) << endl;
#endif
  if (legimate_variable(input.substr(offset, x - offset))) {
    // if(store)Pi.push_back(input.substr(offset,x-offset));
    if (f) (*f)(input.substr(offset, x - offset));
    offset = x;
    return 1;
  }
  return 0;
}
inline void store_in_Pi(const string &input) { Pi.push_back(input); }
int readlist(const string &s, int &offset) {
	Pi.clear();
  offset = skip(s, offset);
  if (!readname(s, offset, store_in_Pi)) return 0;
  offset = skip(s, offset);
  while (s[offset] == ',') {
    offset = skip(s, offset + 1);
    if (!readname(s, offset, store_in_Pi)) return 0;
    offset = skip(s, offset);
  }
  return 1;
}
int read_list_value_version(const string &s, int &offset) {
  offset = skip(s, offset);
  int i;
  do {
    i = s.find_first_of(",)", offset);
// TODO: pass those values to api, change the contents in database
#ifdef _DEBUG_0
    cout << s.substr(offset, i - offset) << endl;
#endif
    offset = i + 1;
  } while (s[i] != ')');
  offset = i;
  return 1;
}
// TODO: push those return 0 to error stack
stack<string> table_op;
vector<string> table_list;
stack<block> Join;
void store_to_table_list(const string &s){
	table_list.push_back(s);
}
int parse_from_clause(const string &input, int offset) {
	table_list.clear();
  int i = offset;
  i = skip(input, offset);
  table_list.clear();
  while(!table_op.empty())table_op.pop();
  if (!readname(input, i,store_to_table_list)) return 0;
  do {
    i = skip(input, i);
    if (_natural_join.take(input, i)) {
      table_op.push("nj");
    } else if (_join.take(input, i)) {
      table_op.push("j");
    } else if (input[i] == ',') {
      table_op.push(",");
      i++;
    } else
      break;
    i = skip(input, i);
    if (!readname(input, i, store_to_table_list)) return 0;
    i = skip(input, i);
  } while (1);
  return 1;
}
// TODO: bracket support
// map<string,map<string,string>> tables;
map<string, TABLE> tables;
string new_table, new_attribute;
// or could do with a class with static members
void get_name(const string &input) { new_table = input; }
void attribute_get_name(const string &input) { new_attribute = input; }
int sql_parse(string &s) {
  int i = 0, _where;
  if (_select.take(s, i)) {
    i = skip(s, i);
    int x1, x2, x3;
    if (s[i] == '*') {
      x1 = 1;
      i++;
    } else
      x1 = readlist(s, i);
    // _from=s.find(" from ");
    if (!_from.take(s, i)) return 0;
    // TODO: parse and return the syntax errors; return 0 if not found
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos) return 0;
    if (_where == string::npos) {
      x3 = 1;
      x2 = parse_from_clause(s.substr(i, _end - i), 0);
    } else {
      x2 = parse_from_clause(s.substr(i, _where - i), 0);
      expr where_clause(s.substr(_where + 7, _end - _where - 7));
      x3 = where_clause.parse();
      where_clause.convert();
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
      cout << x3 << endl;
#endif
    }
    if (x1 && x2 && x3)
      return 1;
    else
      return 0;
  } else if (_delete.take(s, i)) {
    // same as select but no projection
    if (!_from.take(s, i)) return 0;
    int x1, x2;
    x1 = parse_from_clause(s, i);
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos) return 0;
    if (_where == string::npos)
      x2 = 1;
    else {
      expr where_clause(s.substr(_where + 7, _end - _where - 7));
      x2 = where_clause.parse();
      where_clause.convert();
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
#endif
    }
    if (x1 && x2)
      return 1;
    else
      return 0;
  } else if (_update.take(s, i)) {
    i = skip(s, i);
    readname(s, i, get_name);
    if (!_set.take(s, i)) return 0;
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos) return 0;
    int x1, x2;
    if (_where == string::npos) {
      expr set_clause(s.substr(i + 1, _end - i - 1));
#ifdef _DEBUG_0
      cout << s.substr(i + 1, _end - i - 1) << endl;
#endif
      x1 = set_clause.parse();
      x2 = 1;
    } else {
      expr where_clause(s.substr(_where + 7, _end - _where - 7)),
          set_clause(s.substr(i + 1, _where - i - 1));
      x1 = set_clause.parse();
      x2 = where_clause.parse();
      where_clause.convert();
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
      cout << "set_clause:" + s.substr(i + 1, _where - i - 1) << endl;
#endif
    }
    if (x1 && x2)
      return 1;
    else
      return 0;

  } else if (_create.take(s, i)) {
    if (_table.take(s, i)) {
      i = skip(s, i);
      readname(s, i, get_name);
      i = skip(s, i);
      if (s[i] != '(') return 0;
      i = skip(s, i + 1);
      int tuple_offset = 0;
      do {
        readname(s, i, attribute_get_name);
		ATTRIBUTE p;
        if (_int.take(s, i)) {
          p.type = "i";
          p.offset = tuple_offset;
          tuple_offset += 4;
        } else if (_char.take(s, i)) {
          p.type = "s";
          // read(i)
        } else if (_double.take(s, i)) {
          p.type = "d";
          p.offset = tuple_offset;
          tuple_offset += 8;
        } else
          return 0;
		tables[new_table].collum.insert_or_assign(new_attribute,p);
        i = skip(s, i);
        if (s[i] != ',') break;
        i = skip(s, i + 1);
      } while (1);
      // TODO:
      // tables[new_table].table_address=create_table();
	  tables[new_table].tuple_size=tuple_offset;
      i = skip(s, i);
      if (s[i] != ')') return 0;
      i = skip(s, i + 1);
      if (s[i] != ';') return 0;
      return 1;
    } else if (_index.take(s, i)) {
    } else
      return 0;
    // TODO: store in vector
  } else if (_insert_into.take(s, i)) {
    i = skip(s, i);
    readname(s, i, get_name);
	int tuple_size;
	if(tables.count[new_table])tuple_size=tables[new_table].tuple_size;
		else return 0;
	
    if (!_values.take(s, i)) return 0;
    i = skip(s, i);
    if (s[i++] != '(') return 0;
    read_list_value_version(s, i);
    i = skip(s, i + 1);
    if (s[i] != ';') return 0;
    return 1;
  } else
    return 0;
}
