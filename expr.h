#ifndef EXPR_H
#define EXPR_H
#include "pch.h"
#include <map>
#include <string>
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
int evaluate();
struct block {
 public:
  block(const bool b) : p(new int(b)),content("ci") {}
  template <typename T>
  block(const T &data);
  block() : p(NULL), content("") {}
  std::string content;
  void *p;
};

class expr {
 public:
  expr(const std::string &_s) : s(_s) {}
  const std::string s;
  int parse(int offset = 0) const;
  int balance(int start = 0, int end = std::string::npos) const;
  int convert();

 private:
  inline int skip(int i) {
    while (i != std::string::npos && isblank(s[i])) i++;
    return i;
  }
  inline int exist_point(int _i, int j) {
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
  if(i>s.length())return s.length();
  while (isblank(s[i])) i++;
  return i;
}
struct file_address {
  std::string file_name;
  int block_number;
  int block_offset;
};
struct ATTRIBUTE {
  std::string type;
  int offset;
};
class TABLE {
 public:
  file_address table_address;
  void *current_pos;
  std::map<std::string, ATTRIBUTE> collum;
  int tuple_size;
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

// PART II: API
void *get_blank_block(file_address &p);
int flush_to_disk(file_address &p);
int delete_record(void *ptr_to_record);
int insert(void *ptr_to_table, void *insert_data);
void *next(void *ptr_to_table, void *ptr_to_record);
template <typename T>
int update(const void *ptr_to_record, const int offset, const T &data);
template <typename T>
int fetch_attribute(const void *ptr_to_record, const int offset, T &escort);
// pass the desired attribute to "escort";
// char is particularly hard; expect to have string as output
void *get_first_record(void *ptr_to_table);
file_address &create_table(int _size);
void drop_table(file_address &table_address);
// return ptr_to_table; the block number and other booking signs can be placed
// at the begining; then use get_first_record() to get the pointer to first
// record; and then use next() to fetch the rest

#endif
