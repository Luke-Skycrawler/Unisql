#include "pch.h"
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "expr.h"
#define _DEBUG_1
#define EXECFILE
#ifdef EXECFILE
#include <fstream>
#endif //  EXECFILE
#define CREATEINDEX(x)
using namespace std;
extern RecordManager rm;
extern char *current_record;
extern map<string, LOOK_UP_TABLE> attribute_lookup;
extern map<string, Block> search_list;
extern Block syntax_tree[MAX_QUERY_LENGTH], update_exclusive[20];
extern int cursor, ucursor;
// filename = "data.txt";
static char FILENAME[] = "data.txt";
extern BufferManager buffermanager;
map<string, INDEX *> index_lookup;
vector<string> Pi;
list<recordData> &search();
map<string, TABLE> tables;
string new_table, new_attribute;
extern stack<Block> workslab;
inline int ispermitable(char c)
{
  switch (c)
  {
  case '*':
  case ',':
  case ')':
  case '(':
    return 1;
  default:
    return 0;
  }
}
int word::take(const string &s, int &offset)
{
  int j = skip(s, offset), i;
  for (i = 0; i < expected_text.length(); i++)
    if (expected_text[i] != s[i + j])
      return 0;
  offset = i + j;
  if (i + j >= s.length())
    return 1;
  if (!isblank(s[i + j]) && !ispermitable(s[i + j]))
    return 0;
  return 1;
}
word _natural_join("natural join"), _join("join"), _select("select"), _on("on"),
    _delete("delete"), _update("update"), _table("table"), _index("index"),
    _create("create"), _values("values"), _int("int"), _char("char"),
    _quit("quit"), _double("double"), _from("from"), _set("set"),
    _insert_into("insert into"), _unique("unique"), _primary_key("primary key"),
    _drop("drop"), _float("float"), _execfile("execfile");
inline int r_list(char c)
{
  switch (c)
  {
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
inline int readname(const string &input, int &offset,
                    void (*f)(const string &input) = NULL)
{
  // no space at front
  int x = input.find_first_of(", ();", offset);
#ifdef _DEBUG_0
  cout << "#" + input.substr(offset, x - offset) << endl;
#endif
  if (legimate_variable(input.substr(offset, x - offset)))
  {
    if (f)
      (*f)(input.substr(offset, x - offset));
    offset = x;
    return 1;
  }
  return 0;
}
inline void store_in_Pi(const string &input) { Pi.push_back(input); }
int readlist(const string &s, int &offset)
{
  Pi.clear();
  offset = skip(s, offset);
  if (!readname(s, offset, store_in_Pi))
    return 0;
  offset = skip(s, offset);
  while (s[offset] == ',')
  {
    offset = skip(s, offset + 1);
    if (!readname(s, offset, store_in_Pi))
      return 0;
    offset = skip(s, offset);
  }
  return 1;
}
int read_list_value_version(const string &s, int &offset, char *ptr)
{
  offset = skip(s, offset);
  int i, j = 0;
  auto order = tables[new_table].order;
  do
  {
    if (j == order.size())
      return 0;
    i = s.find_first_of(",)", offset);
    const string &value = s.substr(offset, i - offset);
    auto p = order.at(j);
    auto type = p->second.type;
    auto tuple_offset = p->second.offset;
    if (!legimate_const(value))
      return 0;
    if (type == "s" && (value[0] == '"' || value[0] == '\''))
    {
      auto size = p->second.size;
      if (value.length() > size + 2)
        return 0;
      for (int k = 0; k < size; k++)
        if (k < value.length() - 2)
          *(ptr + tuple_offset + k) = value[k + 1];
        else
          *(ptr + tuple_offset + k) = '\0';
      //TODO: notify if the string is too long to store
    }
    else if (type == "d")
    {
      *(double *)(ptr + tuple_offset) = stod(value);
      //tuple_offset += 8;
    }
    else if (type == "i")
    {
      *(int *)(ptr + tuple_offset) = stoi(value);
      // cout << *(int *)(ptr + tuple_offset);
    }
    else
      return 0;
// TODO: pass those values to api, change the contents in database
#ifdef _DEBUG_0
    cout << value << endl;
#endif
    offset = i + 1;
    j++;
    if (p->second.unique)
    {
      //int j = tables[new_table].table_address.block_number;
      // TODO: hash
    }
  } while (s[i] != ')');
  // TODO: review the offset see if it matches with the prototype
  offset = i;
  return 1;
}
// TODO: push those return 0 to error stack
stack<string> table_op;
vector<string> table_list;
stack<Block> Join;
void store_to_table_list(const string &s) { table_list.push_back(s); }
int parse_from_clause(const string &input, int offset)
{
  int i = offset;
  i = skip(input, offset);
  table_list.clear();
  while (!table_op.empty())
    table_op.pop();
  if (!readname(input, i, store_to_table_list))
    return 0;
  if (tables.count(table_list.back()) == 0)
  {
    cout << endl
         << "ERROR: table does not exist" << endl;
    // TODO: replace with queue
    return 0;
  }
  do
  {
    i = skip(input, i);
    if (_natural_join.take(input, i))
      table_op.push("nj");
    else if (_join.take(input, i))
      table_op.push("j");
    else if (input[i] == ',')
    {
      table_op.push(",");
      i++;
    }
    else
      break;
    i = skip(input, i);
    if (!readname(input, i, store_to_table_list))
      return 0;
    if (tables.count(table_list.back()) == 0)
    {
      cout << endl
           << "ERROR: table does not exist" << endl;
      return 0;
    }
    i = skip(input, i);
  } while (1);
  return 1;
}
void get_name(const string &input) { new_table = input; }
void attribute_get_name(const string &input) { new_attribute = input; }
int sql_parse(string &s)
{
  int i = 0, _where;
  attribute_lookup.clear();
  if (_select.take(s, i))
  {
    i = skip(s, i);
    int x1, x2, x3;
    if (s[i] == '*')
    {
      x1 = 1;
      i++;
      Pi.clear();
    }
    else
      x1 = readlist(s, i);
    if (!_from.take(s, i))
      return 0;
    // TODO: parse and return the syntax errors; return 0 if not found
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos)
      return 0;
    if (_where == string::npos)
    {
      x3 = -1;
      x2 = parse_from_clause(s.substr(i, _end - i), 0);
    }
    else
    {
      x2 = parse_from_clause(s.substr(i, _where - i), 0);
      expr where_clause(s.substr(_where + 7, _end - _where - 7));
      x3 = where_clause.parse() && where_clause.balance();
      if (x3)
      {
        x3 = where_clause.convert();
#ifdef _DEBUG_1
        if (x3)
          cout << endl
               << "passed type check" << endl;
        else
          cout << "type check failed" << endl;
      }
      else
        cout << "parsing failed, no typecheck or evaluation" << endl;
#else
      }
#endif
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
      cout << x3 << endl;
#endif
    }
    if (!(x1 && x2 && x3))
      return 0;
    // TODO: iterate through the whole table
    string &a = table_list.back();
    int j = tables[table_list.back()].table_address.block_number;
    list<recordData> l = rm.selectRecord(FILENAME, j);
    if (x3 != -1)
      j = simplify();
    else
      j = -1;
    int tot = 0;
    auto st = clock();
#ifdef _INDEX_MANAGER_ENABLE
    if (j == 1)
    {
      find_index();
      if (index_lookup[search_list.begin()->first])
        l = search();
      else
        j = 2;
    }
#endif
    for (auto jt = tables[a].collum.begin();
         jt != tables[a].collum.end(); jt++)
      if (Pi.empty())
        cout << jt->first + "\t";
      else
        for (int k = 0; k < Pi.size(); k++)
          if (Pi[k] == jt->first)
            cout << jt->first + "\t";
    cout << endl;
    for (auto it = l.begin(); it != l.end(); it++)
    {
      current_record = (it->record);
      if (x3 == -1 || j == 1 || evaluate(syntax_tree, 0, cursor))
      {
        tot++;
        for (auto jt = tables[a].collum.begin(); jt != tables[a].collum.end(); jt++)
          if (Pi.empty())
            jt->second.display(current_record);
          else
            for (int k = 0; k < Pi.size(); k++)
              if (Pi[k] == jt->first)
              {
                //TODO :replace with hash
                jt->second.display(current_record);
                break;
              }
        //batch_output+='\n';
        cout << endl;
      }
      //cout<<batch_output;
      if (j != 1)
        delete[] it->record;
    }
    auto et = clock();
    cout << endl
         << tot << " rows selected, time ="
         << (double)(et - st) / CLOCKS_PER_SEC << " sec" << endl;
    return 1;
  }
  else if (_delete.take(s, i))
  {
    // same as select but no projection
    if (!_from.take(s, i))
      return 0;
    int x1, x2;
    x1 = parse_from_clause(s, i);
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos)
      return 0;
    if (_where == string::npos)
      x2 = -1;
    else
    {
      expr where_clause(s.substr(_where + 7, _end - _where - 7));
      x2 = where_clause.parse() && where_clause.balance();
      if (x2)
      {
        x2 = where_clause.convert();
#ifdef _DEBUG_1
        if (x2)
          cout << endl
               << "passed type check" << endl;
        else
          cout << "type check failed" << endl;
      }
      else
        cout << "parsing failed, no type check or evaluation" << endl;
#else
        //TODO: replace with queue
      }
#endif
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
#endif
    }
    if (!(x1 && x2))
      return 0;
    TABLE &a = tables[table_list.back()];
    int j = a.table_address.block_number;
    list<recordData> l = rm.selectRecord(FILENAME, j);
    int tot = 0;
    auto st = clock();
    for (auto it = l.begin(); it != l.end(); it++)
    {
      current_record = (it->record);
      if (x2 == -1 || evaluate(syntax_tree, 0, cursor))
      {
        tot++;
        index inform;
        for (auto jt = a.index.begin(); jt != a.index.end(); jt++)
        {

          inform.length = a.collum[jt->second.indexed_attributes[0]].size;
          //inform.blocknum_offset=inform.record_blocknum=0;
          int pri_offset = a.collum[jt->second.indexed_attributes[0]].offset;
          char type = a.collum[jt->second.indexed_attributes[0]].type[0];
          typetrans(type, inform, it->record + pri_offset);
          Index_delete(jt->second.root_node_address.block_number, inform);
        }
        rm.deleteRecord(FILENAME, j, *it);
      }
      delete[] it->record;
    }
    auto et = clock();
    cout << tot
         << " rows affected, time=" << (double)(et - st) / CLOCKS_PER_SEC << " sec"
         << endl;
    return 1;
  }
  else if (_update.take(s, i))
  {
    i = skip(s, i);
    if (!readname(s, i, get_name))
      return 0;
    if (!_set.take(s, i))
      return 0;
    _where = s.find(" where ");
    int _end = s.find_last_of(";");
    if (_end == string::npos)
      return 0;
    int x1, x2;
    if (_where == string::npos)
    {
      expr set_clause(s.substr(i + 1, _end - i - 1));
#ifdef _DEBUG_0
      cout << s.substr(i + 1, _end - i - 1) << endl;
#endif
      x1 = set_clause.parse(0, 1) && set_clause.balance();
      set_clause.convert(1);
      x2 = -1;
    }
    else
    {
      expr where_clause(s.substr(_where + 7, _end - _where - 7)), set_clause(s.substr(i + 1, _where - i - 1));
      x1 = set_clause.parse(0, 1) && set_clause.balance();
      if (x1)
        x1 = set_clause.convert(1);
      x2 = where_clause.parse() && where_clause.balance();
      if (x2)
      {
        x2 = where_clause.convert();
#ifdef _DEBUG_1
        if (x2)
          cout << endl
               << "passed type check" << endl;
        else
          cout << endl
               << "type check failed" << endl;
      }
      else
        cout << "parsing failed, no evaluation" << endl;
#else
      }
#endif
#ifdef _DEBUG_0
      cout << endl
           << "where_clause:" + s.substr(_where + 7, _end - _where - 7) << endl;
      cout << "set_clause:" + s.substr(i + 1, _where - i - 1) << endl;
#endif
    }
    if (!(x1 && x2))
      return 0;
    int tot = 0;
    auto st = clock();
    TABLE &a = tables[new_table];
    ATTRIBUTE &to_be_updated = a.collum[*static_cast<string *>(update_exclusive[0].p)];
    int j = a.table_address.block_number;
    list<recordData> l = rm.selectRecord(FILENAME, j);
    for (auto it = l.begin(); it != l.end(); it++)
    {
      current_record = (it->record);
      if (x2 == -1 || evaluate(syntax_tree, 0, cursor))
      {
        tot++;
        index inform;
        // DELETE AND INSERT INDEX
        for (auto jt = a.index.begin(); jt != a.index.end(); jt++)
        {
          inform.length = a.collum[jt->second.indexed_attributes[0]].size;
          int pri_offset = a.collum[jt->second.indexed_attributes[0]].offset;
          char type = a.collum[jt->second.indexed_attributes[0]].type[0];
          typetrans(type, inform, it->record + pri_offset);
          Index_delete(jt->second.root_node_address.block_number, inform);
        }

        evaluate(update_exclusive, 1, ucursor - 1);
        // NOTICE carefully

        char *p = rm.bufmanager.getBlock(FILENAME, it->blocknum)->data + it->offset;
        if (to_be_updated.type[0] == workslab.top().content[1])
          memcpy(p + to_be_updated.offset, workslab.top().p, to_be_updated.size);
        else if (to_be_updated.type[0] == 'd')
          *(double *)(p + to_be_updated.offset) = (double)*(int *)(workslab.top().p);

        for (auto jt = a.index.begin(); jt != a.index.end(); jt++)
        {
          inform.length =
              a.collum[jt->second.indexed_attributes[0]].size;
          int pri_offset =
              a.collum[jt->second.indexed_attributes[0]].offset;
          char type =
              a.collum[jt->second.indexed_attributes[0]].type[0];
          inform.blocknum_offset = it->offset;
          inform.record_blocknum = it->blocknum;
          typetrans(type, inform, p + pri_offset);
          Index_insert(jt->second.root_node_address.block_number,
                       inform);
        }
      }
      delete[] it->record;
    }
    auto et = clock();
    cout << tot
         << " rows affected, time=" << (double)(et - st) / CLOCKS_PER_SEC << " sec"
         << endl;
    return 1;
  }
  else if (_create.take(s, i))
  {
    if (_table.take(s, i))
    {
      i = skip(s, i);
      if (!readname(s, i, get_name))
        return 0;
      i = skip(s, i);
      if (s[i] != '(')
        return 0;
      i = skip(s, i + 1);
      int tuple_offset = 0;
      do
      {
        ATTRIBUTE p;
        if (_primary_key.take(s, i))
        {
          i = skip(s, i);
          if (s[i++] != '(')
            return 0;
          readlist(s, i);
          // TODO: create index of new_table on <Pi>
          if (s[i++] != ')')
            return 0;
          // TODO: commit changes
          CREATEINDEX(Pi.back());
#ifdef _INDEX_MANAGER_ENABLE
          blocknode *m = buffermanager.getNewBlock((char *)"index.txt");
          before_Head_Block_Insert(m->blocknum);
          tables[new_table].primary_key = Pi;
          tables[new_table].index["0"] = INDEX(m->blocknum,Pi.back());
#endif
        }
        else
        {
          if (!readname(s, i, attribute_get_name))
            return 0;
          if (_int.take(s, i))
          {
            p.type = "i";
            p.offset = tuple_offset;
            tuple_offset += 4;
            p.size = 4;
          }
          else if (_char.take(s, i))
          {
            int size;
            p.type = "s";
            i = skip(s, i);
            if (s[i] != '(')
              return 0;
            int j;
            if ((j = s.find(')', i + 1)) != string::npos)
            {
              size = stoi(s.substr(i + 1, j - i - 1)) + 1;
              // TODO: 1 for '\0'
              i = j + 1;
            }
            else
              return 0;
            p.offset = tuple_offset;
            tuple_offset += size;
            p.size = size;
            // read(i)
          }
          else if (_double.take(s, i) || _float.take(s, i))
          {
            p.type = "d";
            p.offset = tuple_offset;
            tuple_offset += 8;
            p.size = 8;
          }
          else
            return 0;
          tables[new_table].collum.insert_or_assign(new_attribute, p);
          tables[new_table].collum[new_attribute].unique =
              _unique.take(s, i) ? 1 : 0;
          tables[new_table].order.push_back(
              tables[new_table].collum.find(new_attribute));
        }
        i = skip(s, i);
        if (s[i] != ',')
          break;
        i = skip(s, i + 1);
      } while (1);
      // TODO:
      // tables[new_table].table_address=create_table();
      tables[new_table].tuple_size = tuple_offset;
      i = skip(s, i);
      if (s[i] != ')')
        return 0;
      i = skip(s, i + 1);
      if (s[i] != ';')
        return 0;
      int j = rm.createTable(FILENAME, tuple_offset);
      tables[new_table].table_address =file_address(j);
      return 1;
    }
    else if (_index.take(s, i))
    {
      i = skip(s, i);
      if (!readname(s, i, attribute_get_name))
        return 0;
      i = skip(s, i);
      if (!_on.take(s, i))
        return 0;
      i = skip(s, i);
      if (!readname(s, i, get_name) || tables.count(new_table) == 0)
        return 0;
      if (s[i = skip(s, i)] != '(')
        return 0;
      i = skip(s, i + 1);
      readlist(s, i);
      if (tables[new_table].collum.count(Pi.back()) == 0)
        return 0;
      // find_attributes();
      if (s[skip(s, i + 1)] != ';')
        return 0;
      if (tables[new_table].collum[Pi.back()].unique == 0)
      {
        cout << endl
             << "ERROR: cannot index on non-unique attributes" << endl;
        return 0;
      }
#ifdef _INDEX_MANAGER_ENABLE
      CREATEINDEX(Pi);
      blocknode *memory_storage = buffermanager.getNewBlock((char *)"index.txt");
      int index_header = memory_storage->blocknum;
      tables[new_table].index[new_attribute] =INDEX(index_header,Pi.back());
      before_Head_Block_Insert(index_header);
      index inform;
      int j = tables[new_table].table_address.block_number;
      list<recordData> l = rm.selectRecord(FILENAME, j);
      ATTRIBUTE &a = tables[new_table].collum[Pi.back()];
      inform.length = a.size;
      for (auto it = l.begin(); it != l.end(); it++)
      {
        typetrans(a.type[0], inform, it->record + a.offset);
        inform.record_blocknum = it->blocknum;
        inform.blocknum_offset = it->offset;
        Index_insert(index_header, inform);
        delete[] it->record;
      }
#endif
      return 1;
    }
    else
      return 0;
  }
  else if (_insert_into.take(s, i))
  {
    i = skip(s, i);
    if (!readname(s, i, get_name))
      return 0;
    int tuple_size;
    if (tables.count(new_table))
      tuple_size = tables[new_table].tuple_size;
    else
      return 0;
    if (!_values.take(s, i))
      return 0;
    i = skip(s, i);
    if (s[i++] != '(')
      return 0;

    char *p = new char[tuple_size];
    read_list_value_version(s, i, p);
    i = skip(s, i + 1);
    if (s[i] != ';')
      return 0;
    // TODO: commit changes
    int j = tables[new_table].table_address.block_number;
    TABLE &a = tables[new_table];
    if (a.primary_key.empty())
    {
      rm.insertRecord(FILENAME, j, p);
      delete[] p;
      return 1;
    }
#ifdef _INDEX_MANAGER_ENABLE
    index inform;
    inform.length = a.collum[a.primary_key[0]].size;
    int pri_offset = a.collum[a.primary_key[0]].offset;
    char type = a.collum[a.primary_key[0]].type[0];
    typetrans(type, inform, p + pri_offset);
    search_one(a.index["0"].root_node_address.block_number, inform);
    if (inform.blocknum_offset)
    {
      cout << endl
           << "repetition in primary key" << endl;
      return 0;
    }
    recordData *pr = rm.insertRecord(FILENAME, j, p);
    inform.record_blocknum = pr->blocknum;
    inform.blocknum_offset = pr->offset;
    Index_insert(a.index["0"].root_node_address.block_number, inform);
    for (auto jt = a.index.begin(); jt != a.index.end(); jt++)
      if (jt->first != "0")
      {
        ATTRIBUTE &t = a.collum[jt->second.indexed_attributes[0]];
        inform.length = t.size;
        typetrans(t.type[0], inform, p + t.offset);
        try
        {
          Index_insert(jt->second.root_node_address.block_number, inform);
        }
        catch (exception &e)
        {
          cout << endl
               << "insertion in index failed" << endl;
        }
      }
#endif
    delete[] p;
    return 1;
  }
  else if (_drop.take(s, i))
  {
    if (_table.take(s, i))
    {
      i = skip(s, i);
      if (!readname(s, i, get_name))
        return 0;
      if (s[skip(s, i)] != ';')
        return 0;
      rm.dropTable(FILENAME, tables[new_table].table_address.block_number);
      tables.erase(new_table);
      return 1;
    }
    else if (_index.take(s, i))
    {
      i = skip(s, i);
      if (!readname(s, i, attribute_get_name) || !_on.take(s, i))
        return 0;
      i = skip(s, i);
      if (!readname(s, i, get_name) || s[skip(s, i)] != ';')
        return 0;
      if (tables.count(new_table))
      {
        // TODO: actual remove of index
        tables[new_table].index.erase(new_attribute);
      }
      return 1;
    }
  }
  else if (_quit.take(s, i))
  {
	non_volatile_flush();
    return -1;
  }
#ifdef EXECFILE
  else if (_execfile.take(s, i))
  {
    i = skip(s, i);
    unsigned j = s.find(";", i);
    if (j == string::npos)
      return 0;
    while (isblank(s[--j]))
      ;
    auto st = clock();
	rm.startExecMode();
    try
    {
      ifstream fin(s.substr(i, j + 1 - i));
      string fstr;
      while (!fin.eof())
      {
        getline(fin, fstr);
        cout << fstr << endl;
        if (sql_parse(fstr) == -1)
          break;
      }
      fin.close();
    }
    catch (exception &e)
    {
      cout << "file exception" << endl;
    }
    auto et = clock();
    cout << "file execution time=" << (double)(et - st) / CLOCKS_PER_SEC << " sec" << endl;
	rm.shutExecMode();
    return 1;
  }
#endif
  return 0;
}
int ATTRIBUTE::display(char *ptr_to_record)
{
  if (type == "d")
    cout << *(double *)(ptr_to_record + offset) << "\t";
  //batch_output+=to_string(*(double *)(ptr_to_record+offset))+'\t';
  else if (type == "i")
    cout << *(int *)(ptr_to_record + offset) << "\t";
  //batch_output+=to_string(*(int *)(ptr_to_record+offset))+'\t';
  else if (type == "s")
    // {
    // for (int i = 0; i < size; i++)
    //   cout << *(ptr_to_record + offset + i);
    //batch_output+=(ptr_to_record+offset)+'\t';
    cout << ptr_to_record + offset << "\t";
  // TODO: modify this(probably slow)
  // }
  return 0;
}
void Block::display()
{
  if (content[1] == 's')
    cout << *static_cast<string *>(p);
  else if (content[1] == 'd')
    cout << *static_cast<double *>(p);
  else if (content[1] == 'i')
    cout << *static_cast<int *>(p);
}
list<recordData> &search()
{
  list<recordData> *l = new list<recordData>;
  for (auto it = search_list.begin(); it != search_list.end(); it++)
  {
    TABLE &t = tables[attribute_lookup[it->first].parent];
    ATTRIBUTE &a = t.collum[it->first];
    index inform;
    inform.length = a.size;
    if (a.type[0] == 's')
      it->second.p = (void *)(static_cast<string *>(it->second.p)->data());
    // FIXME: too violent
    typetrans(a.type[0], inform, it->second.p);
    int k = search_one(index_lookup[it->first]->root_node_address.block_number, inform);
    if (inform.blocknum_offset == 0)
      return *l;
    block *p = rm.bufmanager.getBlock(FILENAME, inform.record_blocknum);
    l->push_back({p->data + inform.blocknum_offset, 0, 0, 0});
  }
  return *l;
}
#ifdef _INDEX_MANAGER_ENABLE
int find_index()
{
  index_lookup.clear();
  for (auto it = search_list.begin(); it != search_list.end(); it++)
  {
    index_lookup[it->first] = NULL;
    TABLE &t = tables[attribute_lookup[it->first].parent];
    auto jt = t.index.begin();
    for (; jt != t.index.end(); jt++)
      if (jt->second.indexed_attributes[0] == it->first)
      {
        index_lookup[it->first] = &jt->second;
        break;
      }
  }
  return 0;
}
#endif