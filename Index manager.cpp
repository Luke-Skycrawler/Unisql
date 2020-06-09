#define _CRT_SECURE_NO_DEPRECATE
#include "DBfile.h"
//#include"API_Module.h"
#include "BufferManager.h"
//#include"Catalog_Manager.h"
#include "pch.h"
#include "Index manager.h"
#include "RecordManager.h"
//#include"Interpreter.h"

BufferManager buffermanager((char*)"index.txt");

CString Char_CString(char* value, int length) {
  CString result;
  for (int k = 0; k < length; k++) {
    // cout << m[k] << endl;
    if (value[k] != '\0')
      result.Insert(k, value[k]);
    else {
      result.Insert(k, '0');
      result.SetAt(k, 0);
    }
  }
  return result;
}

void before_Head_Block_Insert(int headblock_num) {
  blocknode* Head_block =
      buffermanager.getBlock((char*)"index.txt", headblock_num);
  CString value;
  value.Insert(0, '#');
  value.Insert(0, (CString) "0000");
  value.Insert(0, '!');
  // Head_block->data = new char[value.GetLength()];
  char* m = (char*)malloc(value.GetLength() + 1);
  for (int i = 0; i < value.GetLength(); i++) {
    m[i] = value.GetAt(i);
  }
  m[value.GetLength()] = '\0';
  /*m = new char[value.GetLength()];
  strcpy(m, (char*)value.GetString());*/
  memcpy(Head_block->data, m, value.GetLength() + 1);
  buffermanager.setDirty((blocknode*)Head_block);
}

CString double2CString(double value) {
  char* m = new char[8];
  CString result;
  memcpy(m, &value, 8);

  for (int k = 0; k < 8; k++) {
    // cout << m[k] << endl;
    if (m[k] != '\0')
      result.Insert(k, m[k]);
    else {
      result.Insert(k, '0');
      result.SetAt(k, 0);
    }
  }
  return result;
}

double CString2double(CString value_cstring) {
  /*int num = _ttoi(value_cstring.Mid(0, 8));
  double a = double(num);*/
  // double a = _ttof(value_cstring);
  char* m = (char*)malloc(value_cstring.GetLength() + 1);
  for (int i = 0; i < value_cstring.GetLength(); i++) {
    m[i] = value_cstring.GetAt(i);
  }
  m[value_cstring.GetLength()] = '\0';
  double result;
  memcpy(&result, m, 8);
  return result;
}

CString Int2str(int value) {
  char* m = new char[4];
  CString result;
  memcpy(m, &value, 4);

  for (int k = 0; k < 4; k++) {
    // cout << m[k] << endl;
    if (m[k] != '\0')
      result.Insert(k, m[k]);
    else {
      result.Insert(k, '0');
      result.SetAt(k, 0);
    }
  }
  return result;
}

CString Value_CString(char* value, index inform) {
  CString result;
  int type_size = inform.length;
  int LENGTH;
  int NUM = (value[1] - '0') * 1000 + (value[2] - '0') * 100 +
            (value[3] - '0') * 10 + (value[4] - '0');
  if (value[0] == '!') {
    if (value[5 + (8 + type_size) * NUM] == '#')
      LENGTH = 5 + (8 + type_size) * NUM + 1;
    else
      LENGTH = 5 + (8 + type_size) * NUM + 4;
  } else
    LENGTH = 5 + (4 + type_size) * NUM + 4;

  for (int k = 0; k < LENGTH; k++) {
    // cout << m[k] << endl;
    if (value[k] != '\0')
      result.Insert(k, value[k]);
    else {
      result.Insert(k, '0');
      result.SetAt(k, 0);
    }
  }
  return result;
}

int Str2int(CString value_cstring) {
  char* m = new char[value_cstring.GetLength() + 1];
  for (int i = 0; i < value_cstring.GetLength(); i++) {
    m[i] = value_cstring.GetAt(i);
  }
  m[value_cstring.GetLength()] = '\0';
  int result;
  memcpy(&result, m, 4);
  return result;
}

CString Int_CString(int value) {
  CString result;
  result.Format(_T("%.4d"), value);
  return result;
}

int search_one(int headblock_num, struct index& inform) {
  blocknode* Head_block =
      buffermanager.getBlock((char*)"index.txt", headblock_num);
  inform.record_blocknum = 0;
  inform.blocknum_offset = 0;
  int type_kind = inform.type;
  int type_size = inform.length;
  CString des_value = inform.value;
  blocknode* Block_temp = Head_block;

  // check if 所查值的类型错误（int，float，char(n)）
  if (type_kind > 2 || type_kind < 0) {
    return -1;
  } else if (type_kind == 0 && type_size != 4) {
    type_size = 4;
#ifdef _DEBUG_0
    cout << "search_one::type int should have length of 4 bit" << endl;
#endif

  } else if (type_kind == 1 && type_size != 8) {
    type_size = 8;
#ifdef _DEBUG_0
    cout << "search_one::type int should have length of 8 bit" << endl;
#endif
  }

  if (NULL == Block_temp) {
    return -7;
  }

  CString node_value = Value_CString(Block_temp->data, inform);

  while (1) {
    if ('!' == node_value.GetAt(0))  // leaf node
    {
      int pos = find_pos(node_value, des_value, type_size, inform);
      if (pos >= 0) {
        inform.record_blocknum = _ttoi(
            node_value.Mid(5 + (8 + type_size) * pos, 4));  // find the value
        inform.blocknum_offset = _ttoi(
            node_value.Mid(9 + (8 + type_size) * pos, 4));  // find the value
        if (0 == inform.record_blocknum) {
#ifdef _DEBUG_0
          cout << "索引文件为空" << endl;
#endif
        }
      }
      return Block_temp->blocknum;
    } else if ('?' == node_value.GetAt(0))  // normal node
    {
      // int a = node_value.GetLength();
      int pos = find_pos(node_value, des_value, type_size, inform);
      int next_block_num = _ttoi(node_value.Mid(5 + (4 + type_size) * pos, 4));
      blocknode* ptr_temp =
          buffermanager.getBlock((char*)"index.txt", next_block_num);
      node_value = Value_CString(ptr_temp->data, inform);
      Block_temp = ptr_temp;
    } else {
      // 读到异常块
#ifdef _DEBUG_0
      cout << "search_one::read abnormal block" << endl;
#endif
      return -3;
    }
  }
  //读到异常块 on normal case will not reach there?
#ifdef _DEBUG_0
  cout << "function search_one has undefined state" << endl;
#endif
  return -3;
}

void search_part(int headblock_num, int& start, int& end, int type,
                 struct index& inform) {
  int block_num = search_one(headblock_num, inform);
  if (block_num < 0) return;
  if (inform.record_blocknum > 0) {
    if (2 == type) {
      end = find_rightmost_child(headblock_num, inform);
      start = block_num;
      return;
    } else if (4 == type) {
      start = find_leftmost_child(headblock_num, inform);
      end = block_num;
      return;
    }
  }

  blocknode* ptr_temp = buffermanager.getBlock((char*)"index.txt", block_num);
  CString temp = Value_CString(ptr_temp->data, inform);
  int num = _ttoi(temp.Mid(1, 4));
  int offset = find_insert(temp, inform.value, inform.length, inform);
  if (1 == type || 2 == type) {
    if (offset > 0) {
      offset = offset - 1;
    } else if (offset < 0) {
      offset = -offset;
    }
    start = block_num;
    end = find_rightmost_child(headblock_num, inform);
    if (num <= offset) {
      start = find_next_leaf_sibling(inform, start);
      if (0 == start) return;
      ptr_temp = buffermanager.getBlock((char*)"index.txt", start);
      temp = Value_CString(ptr_temp->data, inform);
      offset = 0;
    }
    inform.record_blocknum =
        _ttoi(temp.Mid(5 + (8 + inform.length) * offset, 4));  // find the value
    inform.blocknum_offset =
        _ttoi(temp.Mid(9 + (8 + inform.length) * offset, 4));  // find the value
    return;
  }
  if (3 == type || 4 == type) {
    if (offset > 0) {
      offset = offset - 1;
    } else if (offset < 0) {
      offset = -offset - 1;
    }

    end = block_num;
    start = find_leftmost_child(headblock_num, inform);
    if (offset == 0) {
      end = find_prev_leaf_sibling(headblock_num, inform, end);
      if (0 == end) {
        start = 0;
        return;
      }
      ptr_temp = buffermanager.getBlock((char*)"index.txt", end);
      temp = Value_CString(ptr_temp->data, inform);
      offset = _ttoi(temp.Mid(1, 4));
    }
    offset = offset - 1;
    inform.record_blocknum =
        _ttoi(temp.Mid(5 + (8 + inform.length) * offset, 4));  // find the value
    inform.blocknum_offset =
        _ttoi(temp.Mid(9 + (8 + inform.length) * offset, 4));  // find the value
    return;
  }
#ifdef _DEBUG_0
  cout << "search_many::wrong type" << endl;
#endif
  return;
}

int Cstring_isLarger(CString x, CString y, struct index inform) {
  if (inform.type == 0)  // int
  {
    int i = Str2int(x);
    int j = Str2int(y);
    if (i > j)
      return 1;
    else if (i == j)
      return 0;
    else
      return -1;

  } else if (inform.type == 1)  // double
  {
    int i = CString2double(x);
    int j = CString2double(y);
    if (i > j)
      return 1;
    else if (i == j)
      return 0;
    else
      return -1;
  } else  // char
  {
    if (x > y)
      return 1;
    else if (x == y)
      return 0;
    else
      return -1;
  }
}

int Index_insert(int headblock_num, struct index inform) {
  int record_blocknum = inform.record_blocknum;
  int blocknum_offset = inform.blocknum_offset;
  int leaf_node = search_one(headblock_num, inform);
  int type_size = inform.length;
  CString des_value = inform.value;
  blocknode* pleaf = buffermanager.getBlock(
      (char*)"index.txt", leaf_node);  // the leaf node that insert
  CString value = Value_CString(pleaf->data, inform);
  // int a = value.GetLength();

  /*CString value = pleaf->data;*/

  int pos = find_insert(value, des_value, type_size, inform);

  if (pos < 0) {
    pos = -pos - 1;
    int m = 5 + (8 + type_size) * pos;

    if ("00000000" == value.Mid(m, 8)) {
      value.Delete(m, 8);
      value.Insert(m, Int_CString(blocknum_offset));
      value.Insert(m, Int_CString(record_blocknum));

    } else {
      cout << "Index_insert::already exit in the file" << endl;
      return -1;
    }
  }
  pos--;
  CString record = Int_CString(record_blocknum) + Int_CString(blocknum_offset) +
                   des_value.Right(type_size);
  // int a = value.Insert(5 + (8 + type_size) * pos, record.Right(8 +
  // type_size));
  CString Value1 = value.Left(5 + (8 + type_size) * pos);
  CString Value2 = value.Right(value.GetLength() - 5 - (8 + type_size) * pos);
  value = Value1 + record + Value2;
  // a = value.GetLength();

  // change the total number of record
  int num = _ttoi(value.Mid(1, 4));  // the whole number of record
  num++;

  // if (0)
  if (value.GetLength() + type_size + 8 < BLOCK_SIZE) {
    CString num2 = Int_CString(num).Right(4);
    value.Delete(1, 4);
    int b = value.Insert(1, num2);

    // pleaf->data = new char[value.GetLength()];
    char* m = new char[value.GetLength() + 1];
    for (int i = 0; i < value.GetLength(); i++) {
      char k = value.GetAt(i);
      m[i] = value.GetAt(i);
    }
    m[value.GetLength()] = '\0';
    memcpy(pleaf->data, m, value.GetLength() + 1);
    buffermanager.setDirty((blocknode*)pleaf);

  } else {
#ifdef _DEBUG_0
    cout << "index::Index_insert:Divided" << endl;
#endif


    int new_leaf_node = ask_new_freeblocknum();
    blocknode* pleaf_new =
        buffermanager.getBlock((char*)"index.txt", new_leaf_node);
    CString value1, value2, temp;
    int num1 = num / 2;

    value1 = value.Mid(5, num1 * (8 + type_size));
    value2 =
        value.Mid(5 + num1 * (8 + type_size), (num - num1) * (8 + type_size));
    temp = value2.Mid(8, type_size);

    value1.Insert(0, Int_CString(num1).Right(4));
    value2.Insert(0, Int_CString(num - num1).Right(4));
    value1.Insert(0, '!');
    value2.Insert(0, '!');

    // insert the last 4 characters
    value1.Insert(value1.GetLength(), Int_CString(new_leaf_node).Right(4));

    if (value.Right(1) == "#")
      value2.Insert(value2.GetLength(), '#');
    else
      value2.Insert(value2.GetLength(), value.Right(4));

    // pleaf_new->data = new char[value2.GetLength()];
    char* m = new char[value2.GetLength() + 1];
    for (int i = 0; i < value2.GetLength(); i++) {
      m[i] = value2.GetAt(i);
    }
    m[value2.GetLength()] = '\0';
    memcpy(pleaf_new->data, m, value2.GetLength() + 1);
    buffermanager.setDirty((blocknode*)pleaf_new);

    int father = get_father_blocknum(headblock_num, inform, leaf_node);
    if (father == pleaf->blocknum) {
      int pleaf2_node = ask_new_freeblocknum();
      blocknode* pleaf2 =
          buffermanager.getBlock((char*)"index.txt", pleaf2_node);

      // pleaf2_node->blocknum = leaf_node;//确保头节点的标记
      // pleaf->blocknum = pleaf2;
      CString value_head = "?0001" + Int_CString(pleaf2_node).Right(4) + temp +
                           Int_CString(new_leaf_node).Right(4);
      int a = value_head.GetLength();
      // head_node->data = new char[value_head.GetLength()];
      char* m = new char[value_head.GetLength() + 1];
      for (int i = 0; i < value_head.GetLength(); i++) {
        char k = value_head.GetAt(i);
        m[i] = value_head.GetAt(i);
      }
      m[value_head.GetLength()] = '\0';
      memcpy(pleaf->data, m, value_head.GetLength() + 1);
      buffermanager.setDirty((blocknode*)pleaf);

      char* n = new char[value1.GetLength() + 1];
      for (int i = 0; i < value1.GetLength(); i++) {
        n[i] = value1.GetAt(i);
      }
      n[value1.GetLength()] = '\0';
      memcpy(pleaf2->data, n, value1.GetLength() + 1);
      buffermanager.setDirty((blocknode*)pleaf2);
    } else {
      // pleaf->data = new char[value1.GetLength()];
      m = (char*)malloc(value1.GetLength() + 1);
      for (int i = 0; i < value1.GetLength(); i++) {
        m[i] = value1.GetAt(i);
      }
      m[value1.GetLength()] = '\0';
      memcpy(pleaf->data, m, value1.GetLength() + 1);
      buffermanager.setDirty((blocknode*)pleaf);
      Insert_divide(headblock_num, inform, leaf_node, pleaf_new->blocknum,
                    temp);
    }
  }
  return 1;
}

void Insert_divide(int headblock_num, struct index inform, int leaf1, int leaf2,
                   CString leafpoint2) {
  int type_size = inform.length;
  int block1 = leaf1;
  int block2 = leaf2;

  int father;
  CString temp;
  blocknode* ptr_temp;
  CString value;
  temp = leafpoint2;

  father = get_father_blocknum(headblock_num, inform, block1);
  ptr_temp = buffermanager.getBlock((char*)"index.txt", father);
  value = Value_CString(ptr_temp->data, inform);
  int num = _ttoi(value.Mid(1, 4));  // the whole number of node

  if ('?' == value.GetAt(0))  // normal node
  {
    int pos = find_pos(value, temp, type_size, inform);

    num++;
    CString num2 = Int_CString(num).Right(4);
    value.Delete(1, 4);
    value.Insert(1, num2);

    CString value0 =
        value.Left(9 + (4 + type_size) * pos) + temp + Int_CString(block2) +
        value.Right(value.GetLength() - (9 + (4 + type_size) * pos));
    value = value0;

    // ptr_temp->data = new char[value.GetLength()];
    char* m = (char*)malloc(value.GetLength() + 1);
    for (int i = 0; i < value.GetLength(); i++) {
      m[i] = value.GetAt(i);
    }
    m[value.GetLength()] = '\0';
    memcpy(ptr_temp->data, m, value.GetLength() + 1);
    num++;
    // if(0)
    if (value.GetLength() + type_size + 8 <
        BLOCK_SIZE)  // if the node is not full
    {
      buffermanager.setDirty((blocknode*)ptr_temp);
      return;
    } else {  // if the node is full

      int new_node = ask_new_freeblocknum();
      blocknode* pleaf_new =
          buffermanager.getBlock((char*)"index.txt", new_node);
      CString value1, value2;
      int num1 = num / 2;
      value1 = value.Mid(5, num1 * (4 + type_size) - type_size);
      value2 = value.Mid(5 + num1 * (4 + type_size),
                         (num - num1) * (4 + type_size) - type_size);
      value1.Insert(0, Int_CString(num1 - 1).Right(4));
      value2.Insert(0, Int_CString(num - num1 - 1).Right(4));
      value1.Insert(0, '?');
      value2.Insert(0, '?');

      temp = value.Mid(5 + num1 * (4 + type_size) - type_size, type_size);

      // ptr_temp->data = new char[value1.GetLength()];
      /*char* m = (char*)malloc(value1.GetLength() + 1);
      for (int i = 0; i < value1.GetLength(); i++) {
              m[i] = value1.GetAt(i);
      }
      m[value1.GetLength()] = '\0';
      memcpy(ptr_temp->data, m, value1.GetLength() + 1);
      buffermanager.setDirty((blocknode*) ptr_temp);*/

      // pleaf_new->data = new char[value2.GetLength()];
      char* m = (char*)malloc(value2.GetLength() + 1);
      for (int i = 0; i < value2.GetLength(); i++) {
        m[i] = value2.GetAt(i);
      }
      m[value2.GetLength()] = '\0';
      memcpy(pleaf_new->data, m, value2.GetLength() + 1);
      buffermanager.setDirty((blocknode*)pleaf_new);

      if (father == headblock_num) {
        int pleaf2_node = ask_new_freeblocknum();
        blocknode* pleaf2 =
            buffermanager.getBlock((char*)"index.txt", pleaf2_node);

        // head_node->blocknum = ptr_temp->blocknum;//确保头节点的标记
        // ptr_temp->blocknum = head;
        CString value_head = "?0001" + Int_CString(pleaf2_node).Right(4) +
                             temp + Int_CString(new_node).Right(4);
        // head_node->data = new char[value_head.GetLength()];
        char* m = (char*)malloc(value_head.GetLength() + 1);
        for (int i = 0; i < value_head.GetLength(); i++) {
          m[i] = value_head.GetAt(i);
        }
        m[value_head.GetLength()] = '\0';
        memcpy(ptr_temp->data, m, value_head.GetLength() + 1);
        buffermanager.setDirty((blocknode*)ptr_temp);

        m = (char*)malloc(value1.GetLength() + 1);
        for (int i = 0; i < value1.GetLength(); i++) {
          m[i] = value1.GetAt(i);
        }
        m[value1.GetLength()] = '\0';
        memcpy(pleaf2->data, m, value1.GetLength() + 1);
        buffermanager.setDirty((blocknode*)pleaf2);
      } else {
        char* m = (char*)malloc(value1.GetLength() + 1);
        for (int i = 0; i < value1.GetLength(); i++) {
          m[i] = value1.GetAt(i);
        }
        m[value1.GetLength()] = '\0';
        memcpy(ptr_temp->data, m, value1.GetLength() + 1);
        buffermanager.setDirty((blocknode*)ptr_temp);

        Insert_divide(headblock_num, inform, father, new_node, temp);
      }
    }
  } else {
#ifdef _DEBUG_0
    cout << "buffer::Insert_divide::read unexpected block" << endl;
#endif

    return;
  }
}

int find_prev_leaf_sibling(int headblock_num, struct index inform,
                           int nodenum) {
  int most_left_child = find_leftmost_child(headblock_num, inform);
  if (most_left_child == nodenum) {
    return 0;
  }
  blocknode* ptr_temp =
      buffermanager.getBlock((char*)"index.txt", most_left_child);
  CString node_value = Value_CString(ptr_temp->data, inform);
  int blocknum_rem = ptr_temp->blocknum;

  while (node_value.Right(1) != "#") {
    ptr_temp =
        buffermanager.getBlock((char*)"index.txt", _ttoi(node_value.Right(3)));
    node_value = Value_CString(ptr_temp->data, inform);
    if (ptr_temp->blocknum == nodenum) return blocknum_rem;
    blocknum_rem = ptr_temp->blocknum;
  }
  return -1;
}

int find_next_leaf_sibling(struct index inform, int blocknum) {
  blocknode* ptr_temp = buffermanager.getBlock((char*)"index.txt", blocknum);
  CString node_value = Value_CString(ptr_temp->data, inform);
  if (node_value.Right(1) == "#") {
    return 0;
  }
  return _ttoi(node_value.Right(3));
}

int find_leftmost_child(int headblock_num, index inform) {
  int type_kind = inform.type;
  int type_size = inform.length;
  blocknode* Head_block =
      buffermanager.getBlock((char*)"index.txt", headblock_num);
  blocknode* Block_temp = Head_block;
  CString node_value = Value_CString(Block_temp->data, inform);

  while (1) {
    if ('!' == node_value.GetAt(0))  // leaf node
    {
      return Block_temp->blocknum;
    } else if ('?' == node_value.GetAt(0))  // normal node
    {
      int num = _ttoi(node_value.Mid(1, 4));  // the whole number of node
      if (0 == num) return -1;
      blocknode* ptr_temp = buffermanager.getBlock((char*)"index.txt",
                                                   _ttoi(node_value.Mid(5, 4)));
      node_value = Value_CString(ptr_temp->data, inform);
      ;
      Block_temp = ptr_temp;
    } else {
      // 读到异常块
#ifdef _DEBUG_0
      cout << "find_leftmost_child::read unnormal block" << endl;
#endif

      return -1;
    }
  }

  //读到异常块 on normal case will not reach there
#ifdef _DEBUG_0
  cout << "function find_leftmost_child has undefined state" << endl;
#endif

  return -1;
}

int find_rightmost_child(int headblock_num, index inform) {
  blocknode* Head_block =
      buffermanager.getBlock((char*)"index.txt", headblock_num);
  int type_kind = inform.type;
  int type_size = inform.length;
  blocknode* Block_temp = Head_block;
  CString node_value = Value_CString(Block_temp->data, inform);

  while (1) {
    if ('!' == node_value.GetAt(0))  // leaf node
    {
      return Block_temp->blocknum;
    } else if ('?' == node_value.GetAt(0))  // normal node
    {
      int num = _ttoi(node_value.Mid(1, 4));  // the whole number of node
      if (0 == num) return -1;

      blocknode* ptr_temp = buffermanager.getBlock(
          (char*)"index.txt",
          _ttoi(node_value.Mid(5 + (4 + type_size) * num, 4)));
      node_value = Value_CString(ptr_temp->data, inform);
      Block_temp = ptr_temp;
    } else {
      // 读到异常块
#ifdef _DEBUG_0
      cout << "find_rightmost_child::read unnormal block" << endl;
#endif

      return -1;
    }
  }
  //读到异常块 on normal case will not reach there?
#ifdef _DEBUG_0
  cout << "function find_rightmost_child has undefined state" << endl;
#endif

  return -1;
}

int ask_new_freeblocknum() {
  /*char* file_name;
  strcpy(file_name, "index.txt");*/
  blocknode* temp = buffermanager.getNewBlock((char*)"index.txt");
  return temp->blocknum;
}

int get_father_blocknum(int headblock_num, index inform, int num) {
  blocknode* Head_block =
      buffermanager.getBlock((char*)"index.txt", headblock_num);
  inform.record_blocknum = 0;
  inform.blocknum_offset = 0;
  int type_size = inform.length;
  CString des_value = inform.value;
  blocknode* Block_temp = Head_block;

  //索引文件为空
  if (NULL == Block_temp) return -8;
  int block_num = Block_temp->blocknum;

  CString node_value = Value_CString(Block_temp->data, inform);

  while (1) {
    if (num == Block_temp->blocknum)
      return block_num;
    else
      block_num = Block_temp->blocknum;

    if ('!' == node_value.GetAt(0))  // leaf node
    {
      int pos = find_pos(node_value, des_value, type_size, inform);
      if (pos < 0)
        return -1;
      else {
        return 0;
      }
    } else if ('?' == node_value.GetAt(0))  // normal node
    {
      int pos = find_pos(node_value, des_value, type_size, inform);
      blocknode* ptr_temp = buffermanager.getBlock(
          (char*)"index.txt",
          _ttoi(node_value.Mid(5 + (4 + type_size) * pos, 4)));
      node_value = Value_CString(ptr_temp->data, inform);
      Block_temp = ptr_temp;
    } else {
      // 读到异常块
#ifdef _DEBUG_0
      cout << "get_father_blocknum :: read abnormal block" << endl;
#endif

      return -1;
    }
  }
  //读到异常块 on normal case will not reach there?
#ifdef _DEBUG_0
  cout << "get_father_blocknum :: function search_one has undefined state"
       << endl;
#endif

  return 0;
}

void Index_delete(
    int headblock_num,
    struct index
        inform) { /*
                         fileInfo* ptrDatabase;
                         ptrDatabase = get_file_info(database, table_name, 1);*/
  int Block_num = search_one(headblock_num, inform);
  if (inform.record_blocknum <= 0) {
#ifdef _DEBUG_0
    cout << "Index_delete::删除的数据不存在表中" << endl;
#endif

    return;
  }
  blocknode* Block = buffermanager.getBlock((char*)"index.txt", Block_num);
  CString value = Value_CString(Block->data, inform);
  int pos = find_pos(value, inform.value, inform.length, inform);
  pos = 5 + (8 + inform.length) * pos;
  CString Zero = "00000000";
  value.Delete(pos, 8);
  value.Insert(pos, Zero);
  //(char*)(LPCTSTR)num2;
  // Block->data = new char[value.GetLength()];
  char* m = (char*)malloc(value.GetLength() + 1);
  for (int i = 0; i < value.GetLength(); i++) {
    m[i] = value.GetAt(i);
  }
  m[value.GetLength()] = '\0';
  memcpy(Block->data, m, value.GetLength() + 1);
  buffermanager.setDirty((blocknode*)Block);
}

int find_pos(CString value, CString des_value, int type_size, index inform) {
  CString node_value = value;
  int num = _ttoi(node_value.Mid(1, 4));  // the whole number of record

  if ('!' == node_value.GetAt(0))  // leaf node
  {
    int left, right, middle;
    left = 0;
    right = num - 1;
    // the following loop is a Binary Search
    while (left <= right) {
      middle = (left + right) / 2;
      if (Cstring_isLarger(
              node_value.Mid(13 + (8 + type_size) * middle, type_size),
              des_value, inform) > 0) {
        right = middle - 1;
      } else if (Cstring_isLarger(
                     node_value.Mid(13 + (8 + type_size) * middle, type_size),
                     des_value, inform) < 0) {
        left = middle + 1;
      } else {
        return middle;
      }
    }
    return -1;
  } else if ('?' == node_value.GetAt(0))  // normal node
  {
    int left, right, middle;
    left = 0;
    right = num;
    // the following loop is a Binary Search
    while (left < right) {
      middle = (left + right) / 2;
      if (Cstring_isLarger(
              node_value.Mid(9 + (4 + type_size) * middle, type_size),
              des_value, inform) > 0) {
        right = middle;
      } else if (Cstring_isLarger(
                     node_value.Mid(9 + (4 + type_size) * middle, type_size),
                     des_value, inform) < 0) {
        left = middle + 1;
      } else {
        left = right = middle + 1;
        break;
      }
    }

    return left;
  } else {
#ifdef _DEBUG_0
    cout << " find_pos::读到异常块 " << endl;
#endif

    return -3;
  }
}

int find_insert(CString value, CString des_value, int type_size, index inform) {
  CString node_value = value;

  int num = _ttoi(node_value.Mid(1, 4));  // the whole number of record
  if (0 == num) return 1;
  int left, right, middle;
  if ('!' == node_value.GetAt(0))  // leaf node
  {
    left = 0;
    right = num;
    // the following loop is a Binary Search
    while (left < right) {
      middle = (left + right) / 2;
      if (Cstring_isLarger(
              node_value.Mid(13 + (8 + type_size) * middle, type_size),
              des_value, inform) > 0) {
        right = middle;
      } else if (Cstring_isLarger(
                     node_value.Mid(13 + (8 + type_size) * middle, type_size),
                     des_value, inform) < 0) {
        left = middle + 1;
      } else {
#ifdef _DEBUG_0
        cout << "Index_insert::input already exit in the table" << endl;
#endif

        return -middle - 1;
      }
    }
    return left + 1;
  } else {
#ifdef _DEBUG_0
    cout << " find_pos::读到异常块 " << endl;
#endif

    return 0;
  }
}

struct index find_offset_in_block(int blocknum, int index_offset, int type_size,
                                  char type) {
  struct index inform;
  blocknode* block_temp = buffermanager.getBlock((char*)"index.txt", blocknum);
  CString value = Value_CString(block_temp->data, inform);
  if (index_offset > _ttoi(value.Mid(1, 4)) || index_offset < 1) {
    inform.length = -1;
  } else {
    inform.length = type_size;
    inform.type = type;
    inform.record_blocknum =
        _ttoi(value.Mid(5 + (8 + type_size) * (index_offset - 1), 4));
    inform.blocknum_offset =
        _ttoi(value.Mid(9 + (8 + type_size) * (index_offset - 1), 4));
    inform.value =
        value.Mid(13 + (8 + type_size) * (index_offset - 1), type_size);
  }
  return inform;
}
//
// int next_block(int blocknum, int index_offset, int type_size) {
//	blocknode* block_temp = getBlock((char*)"index.txt", blocknum);
//	CString value = block_temp->data;
//	int num =  _ttoi(value.Mid(1, 4));
//	return _ttoi(value.Mid(5 + (8 + type_size) * num, 4));
//}
