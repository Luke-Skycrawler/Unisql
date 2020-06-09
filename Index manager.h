#pragma once
#include "pch.h"
#include <atlstr.h>
#include <stdio.h>
#include <iostream>
#include <string>

#define BLOCK_SIZE 4000

using namespace std;

// 所读取块的信息

// struct block
//{
//	int blockNum;                     // the blocknumber of the block, which
// indicate it when it be newed 	bool dirty;                       // 0
// ->flase
////1 -> indicate dirty, write back 	bool lock; //
// prevent theblock from replacing 	block* nextblock; 	block* preblock;
// char* data;
//// the arrayspace for storing the records in the block in buffer
//};

//定义表的索引节点信息

struct index {
  int length;  // the length of the value
  int type;    // the type of the value  //0---int,1---double,2----char(n)
  int record_blocknum;  // the record offset in the table file
  int blocknum_offset;
  // long offset;
  CString value;  // the value
};

CString double2CString(double value);          // double转换为CString
double CString2double(CString value_cstring);  // CString转换为double
CString Int2str(int value);  // int 型数值转化成4字节的CString类型
int Str2int(CString value_cstring);  // CString 型数值转化成4字节的int类型

CString Char_CString(char* value, int length);  //将char*转换为定长字符串
CString Int_CString(int value);

CString Value_CString(char* value, index inform);  //将Buffer中char*转换为字符串

int Cstring_isLarger(CString x, CString y, struct index inform);  //比较数值大小

void before_Head_Block_Insert(
    int headblock_num);  // B+树插入第一个index时调用，用于初始化headblock

int search_one(int headblock_num,
               struct index& inform); /*
查找一个值等于inform.value(inform为info结构体)的记录在table表中的记录号：
函数结束返回时，记录号保存在inform.offset中，若没有，inform.offset＝0。
返回值：
- 7    inform.offset = 0    索引文件为空
- 8    inform.offset = 0    表中无记录
- 1    inform.offset = 0    所查值的类型错误（int，float，char(n)）
- 3    inform.offset = 0    读到异常块
正数  叶子块号， 若inform.offset > 0，记录存在表文件中，记录号为inform.offset
正数  叶子块号
，若inform.offset＝0，记录不在表文件中，返回的叶子块号在函数Index_insert中要用（因为将值插入改块就够了）
*/

void search_part(int headblock_num, int& start, int& end, int type,
                 struct index& inform); /*
查找一批>, >= , <, <=
inform.value(inform为info结构体)的记录在table表中的记录号： type： 3 是 找出所有
< inform.value 的记录; 4 是 找出所有 <= inform.value 的记录; 1 是 找出所有 >
inform.value; 的记录； 2 是 找出所有 >= inform.value 的记录;
*/

int Index_insert(
    int headblock_num,
    struct index inform);  //插入一个值的函数，值和类型在参数inform结构体中

void Insert_divide(int headblock_num, struct index inform, int leaf1, int leaf2,
                   CString leafpoint2);  //到叶子块满时调用函数

int find_prev_leaf_sibling(int headblock_numk, struct index inform,
                           int nodenum); /*
找出所在叶子块的前一个兄弟叶子块：
- 1      函数中读到错误块；
0       没有前一个兄弟叶子块（即此块为最左孩子）
>0      前一个兄弟叶子块的块号
*/

int find_next_leaf_sibling(struct index inform, int blocknum); /*
 找出所在叶子块的后一个兄弟叶子块：
 - 1      函数中读到错误块；
 0       没有后一个兄弟叶子块（即此块为最右孩子）
 >0      后一个兄弟叶子块的块号
 */

int find_leftmost_child(int headblock_num, index inform); /*
找出最左叶子块的块号：
- 1      函数中读到错误块；
0       索引文件空
>0      最左叶子块的块号
*/

int find_rightmost_child(int headblock_num, index inform); /*
 找出最右叶子块的块号：
 - 1      函数中读到错误块；
 0       索引文件空
 >0      最右叶子块的块号
 */

int ask_new_freeblocknum(); /*
 获取一个空块的块号，并使总块数增一
 0      读到异常块，或者块数>256块
 >0     空块号
 思路：判断块数是否>256，若没有，判断空块链表是否为空，为空则得到（现有总块号数
 ＋1）的空块，否则，读取空块链表首，修改链表；并使总块数＋1。
 */

int get_father_blocknum(int headblock_num, index inform, int num); /*
 找块num的父亲块的块号（在查找inform.value的路径上）
 返回值：
 - 1      函数中读到错误块；
 0       没有父亲块（即此块为根）
 >0      父亲块的块号
 */

void Index_delete(int headblock_num, struct index inform);  //删除index记录

int find_pos(CString value, CString des_value, int type_size, index inform); /*
查找记录在block中的位置 返回在块中的位置offset
不存在返回-1
*/

int find_insert(CString value, CString des_value, int type_size,
                index inform); /*
返回插入位置 所在区间 1~n
异常返回 0
对于叶子 如果已经存在表中存在 -pos -1
*/

struct index find_offset_in_block(int blocknum, int index_offset, int type_size,
                                  char type);
//查找返回block中对应项的index结构体，如果返回的index.length == -1则超过数据记录
