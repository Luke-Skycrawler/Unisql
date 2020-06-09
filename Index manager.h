#pragma once
#include "pch.h"
#include <atlstr.h>
#include <stdio.h>
#include <iostream>
#include <string>

#define BLOCK_SIZE 4000

using namespace std;

// ����ȡ�����Ϣ

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

//�����������ڵ���Ϣ

struct index {
  int length;  // the length of the value
  int type;    // the type of the value  //0---int,1---double,2----char(n)
  int record_blocknum;  // the record offset in the table file
  int blocknum_offset;
  // long offset;
  CString value;  // the value
};

CString double2CString(double value);          // doubleת��ΪCString
double CString2double(CString value_cstring);  // CStringת��Ϊdouble
CString Int2str(int value);  // int ����ֵת����4�ֽڵ�CString����
int Str2int(CString value_cstring);  // CString ����ֵת����4�ֽڵ�int����

CString Char_CString(char* value, int length);  //��char*ת��Ϊ�����ַ���
CString Int_CString(int value);

CString Value_CString(char* value, index inform);  //��Buffer��char*ת��Ϊ�ַ���

int Cstring_isLarger(CString x, CString y, struct index inform);  //�Ƚ���ֵ��С

void before_Head_Block_Insert(
    int headblock_num);  // B+�������һ��indexʱ���ã����ڳ�ʼ��headblock

int search_one(int headblock_num,
               struct index& inform); /*
����һ��ֵ����inform.value(informΪinfo�ṹ��)�ļ�¼��table���еļ�¼�ţ�
������������ʱ����¼�ű�����inform.offset�У���û�У�inform.offset��0��
����ֵ��
- 7    inform.offset = 0    �����ļ�Ϊ��
- 8    inform.offset = 0    �����޼�¼
- 1    inform.offset = 0    ����ֵ�����ʹ���int��float��char(n)��
- 3    inform.offset = 0    �����쳣��
����  Ҷ�ӿ�ţ� ��inform.offset > 0����¼���ڱ��ļ��У���¼��Ϊinform.offset
����  Ҷ�ӿ��
����inform.offset��0����¼���ڱ��ļ��У����ص�Ҷ�ӿ���ں���Index_insert��Ҫ�ã���Ϊ��ֵ����Ŀ�͹��ˣ�
*/

void search_part(int headblock_num, int& start, int& end, int type,
                 struct index& inform); /*
����һ��>, >= , <, <=
inform.value(informΪinfo�ṹ��)�ļ�¼��table���еļ�¼�ţ� type�� 3 �� �ҳ�����
< inform.value �ļ�¼; 4 �� �ҳ����� <= inform.value �ļ�¼; 1 �� �ҳ����� >
inform.value; �ļ�¼�� 2 �� �ҳ����� >= inform.value �ļ�¼;
*/

int Index_insert(
    int headblock_num,
    struct index inform);  //����һ��ֵ�ĺ�����ֵ�������ڲ���inform�ṹ����

void Insert_divide(int headblock_num, struct index inform, int leaf1, int leaf2,
                   CString leafpoint2);  //��Ҷ�ӿ���ʱ���ú���

int find_prev_leaf_sibling(int headblock_numk, struct index inform,
                           int nodenum); /*
�ҳ�����Ҷ�ӿ��ǰһ���ֵ�Ҷ�ӿ飺
- 1      �����ж�������飻
0       û��ǰһ���ֵ�Ҷ�ӿ飨���˿�Ϊ�����ӣ�
>0      ǰһ���ֵ�Ҷ�ӿ�Ŀ��
*/

int find_next_leaf_sibling(struct index inform, int blocknum); /*
 �ҳ�����Ҷ�ӿ�ĺ�һ���ֵ�Ҷ�ӿ飺
 - 1      �����ж�������飻
 0       û�к�һ���ֵ�Ҷ�ӿ飨���˿�Ϊ���Һ��ӣ�
 >0      ��һ���ֵ�Ҷ�ӿ�Ŀ��
 */

int find_leftmost_child(int headblock_num, index inform); /*
�ҳ�����Ҷ�ӿ�Ŀ�ţ�
- 1      �����ж�������飻
0       �����ļ���
>0      ����Ҷ�ӿ�Ŀ��
*/

int find_rightmost_child(int headblock_num, index inform); /*
 �ҳ�����Ҷ�ӿ�Ŀ�ţ�
 - 1      �����ж�������飻
 0       �����ļ���
 >0      ����Ҷ�ӿ�Ŀ��
 */

int ask_new_freeblocknum(); /*
 ��ȡһ���տ�Ŀ�ţ���ʹ�ܿ�����һ
 0      �����쳣�飬���߿���>256��
 >0     �տ��
 ˼·���жϿ����Ƿ�>256����û�У��жϿտ������Ƿ�Ϊ�գ�Ϊ����õ��������ܿ����
 ��1���Ŀտ飬���򣬶�ȡ�տ������ף��޸�������ʹ�ܿ�����1��
 */

int get_father_blocknum(int headblock_num, index inform, int num); /*
 �ҿ�num�ĸ��׿�Ŀ�ţ��ڲ���inform.value��·���ϣ�
 ����ֵ��
 - 1      �����ж�������飻
 0       û�и��׿飨���˿�Ϊ����
 >0      ���׿�Ŀ��
 */

void Index_delete(int headblock_num, struct index inform);  //ɾ��index��¼

int find_pos(CString value, CString des_value, int type_size, index inform); /*
���Ҽ�¼��block�е�λ�� �����ڿ��е�λ��offset
�����ڷ���-1
*/

int find_insert(CString value, CString des_value, int type_size,
                index inform); /*
���ز���λ�� �������� 1~n
�쳣���� 0
����Ҷ�� ����Ѿ����ڱ��д��� -pos -1
*/

struct index find_offset_in_block(int blocknum, int index_offset, int type_size,
                                  char type);
//���ҷ���block�ж�Ӧ���index�ṹ�壬������ص�index.length == -1�򳬹����ݼ�¼
