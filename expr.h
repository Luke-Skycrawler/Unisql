#ifndef EXPR_H
#define EXPR_H
#include "pch.h"
#include <map>
#include <string>
#include <vector>
#include "BufferManager.h"
#include "RecordManager.h"
#define _INDEX_MANAGER_ENABLE
#ifdef _INDEX_MANAGER_ENABLE
#include "Index manager.h"
#define INDEX_FILE std::string("index.txt")
int find_index();
inline void typetrans(const char type, index &inform, void *data) {
  switch (type) {
    case 's':
      inform.type = 2;
      inform.value = Char_CString(static_cast<char *>(data),inform.length);
      break;
    case 'd':
      inform.type = 1;
      inform.value = double2CString(*static_cast<double *>(data));
      break;
    case 'i':
      inform.type = 0;
      inform.value = Int2str(*static_cast<int *>(data));
      break;
  }
}
#endif
#define MAX_QUERY_LENGTH 100
inline int proper_name(char c) {
  return ((c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c == '_'));
}
inline int isblank(char c) {
  if (c != ' ' && c != '\t' && c != '\n') return 0;
  return 1;
}
static const char OPERATORS[] = "+-*/<>!=";
#define OP_COUNT sizeof(OPERATORS) - 1
inline int isoperator(char c) {
  for (int i = 0; i < OP_COUNT; i++)
    if (c == OPERATORS[i]) return 1;
  return 0;
}
int legimate_variable(const std::string &name);
int legimate_binary_operator(const std::string &s);
int legimate_const(const std::string &value);
int sql_parse(std::string &s);
int find_attributes();
int simplify();
struct Block {
 public:
  Block(const bool b) : p(new int(b)), content("ci") {}
  template <typename T>
  Block(const T &data);
  Block() : p(NULL), content("") {}
  std::string content;
  void *p;
  void display();
};
int evaluate(Block *tree,const int start,const int root);

class expr {
 public:
  expr(const std::string &_s) : s(_s) {}
  const std::string s;
  int parse(int offset = 0, int update_clause = 0) const;
  int balance(int start = 0, int end = std::string::npos) const;
  int convert(int update_clause = 0) const;

 private:
  inline int skip(int i) const {
    while (i <s.length() && isblank(s[i])) i++;
    return i;
  }
  inline int exist_point(int _i, int j) const {
    for (int i = _i; i <= j; i++)
      if (s[i] == '.') return 1;
    return 0;
  }
};

enum {
  ACCEPTED = 1,
  BRACKET_LEFT,
  BRACKET_RIGHT,
  INVALID_LITERAL,
  BETWEEN_NOT_MATCHED,
  EXTRA_BRACKET,
  INVALID_OPERATOR
};
class word {
 public:
  word(std::string &s) : expected_text(s) {}
  word(const char *s) : expected_text(std::string(s)) {}
  const std::string expected_text;
  int take(const std::string &s, int &offset);
};
inline int skip(const std::string &s, int offset) {
  int i = offset;
  if (i > s.length()) return s.length();
  while (isblank(s[i])) i++;
  return i;
}
struct file_address {
  file_address(int block_number=0,const char * file="data.txt"):block_number(block_number),file_name(file){}
  std::string file_name;
  int block_number;
  int block_offset;
};
struct ATTRIBUTE {
  ATTRIBUTE(){}
  ATTRIBUTE(std::string type,int offset,int size,int unique):
    type(type),offset(offset),size(size),unique(unique){}
  std::string type;
  int offset;
  int size;
  int display(char *ptr_to_record);
  int unique;
};
struct INDEX {
  INDEX(){}
  INDEX(int block_number,std::string _ia):root_node_address(block_number,"index.txt"){
    indexed_attributes.push_back(_ia);
  }
  file_address root_node_address;
  std::vector<std::string> indexed_attributes;
};
class TABLE {
 public:
  TABLE(){}
  TABLE(int block_number,int tuple_size):table_address(block_number),tuple_size(tuple_size){}
  file_address table_address;
  void *current_pos;
  int tuple_size;
  std::map<std::string, ATTRIBUTE> collum;
  std::map<std::string, INDEX> index;
  std::vector<std::string> primary_key;
  std::vector<std::map<std::string, ATTRIBUTE>::iterator> order;
 private:
};
struct LOOK_UP_TABLE {
  std::string attribute, parent;
};
template <class T>
struct tmp_class {
  tmp_class(void *p) : data(*static_cast<T *>(p)) {}
  T data;
};
void non_volatile_flush(void);
void non_volatile_init(void);
// PART II: API
void *get_blank_block(file_address &p);
int flush_to_disk(file_address &p);
int delete_record(void *ptr_to_record);
int insert(file_address &ptr_to_table, void *insert_data);
void *next(void *ptr_to_table, void *ptr_to_record);
template <typename T>
int update(const void *ptr_to_record, const int offset, const T &data);
template <typename T>
int fetch_attribute(const void *ptr_to_record, const int offset, T &escort);
// pass the desired attribute to "escort";
// char is particularly hard; expect to have string as output
void *get_first_record(void *ptr_to_table);
file_address &create_table(int _size);
file_address &create_index(const std::vector<ATTRIBUTE> &a);
int &insert_index(file_address &root,void *ptr_to_record);
void drop_table(file_address &table_address);
// return ptr_to_table; the Block number and other booking signs can be placed
// at the begining; then use get_first_record() to get the pointer to first
// record; and then use next() to fetch the rest

#endif
