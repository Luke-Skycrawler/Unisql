// #include "pch.h"
// #include <ctime>
// #include <iostream>
// #include "RecordManager.h"
// using namespace std;

// int main() {
//   using namespace std;
//   long begin, finish;
//   begin = clock();
//   char record[] = "123456789123456789123456789";
//   char filename[] = "data.txt";
//   RecordManager rm(filename);
//   //    rm.bufmanager.freeBlock(filename,44);
//   int num = rm.createTable(filename, 28);
//   for (int i = 0; i < 10000; ++i) {
//     rm.insertRecord(filename, num, record);
//   }
//   rm.bufmanager.showBufBlockList();
//   //    rm.dropTable(filename,44);
//   list<recordData> l = rm.selectRecord(filename, num);
//   //    list<recordData>::iterator iter;
//   //    for(iter = l.begin(); iter != l.end(); iter++)
//   //    {
//   //        cout << iter->record << endl;
//   //    }
//   cout << l.size() << endl;
//   //    recordData records = {string(record),5,num,28};
//   //    rm.deleteRecord(filename,num,records);
//   finish = clock();
//   cout << (double)(finish - begin) / CLOCKS_PER_SEC << "s";
//   return 0;
// }
// Unisql.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <queue>
#include <string>
#include "expr.h"
using namespace std;
extern queue<pair<int, int>> error_queue;
const char* ERRORS[] = {
    "ACCEPTED",        "BRACKET_LEFT",        "BRACKET_RIGHT",
    "INVALID_LITERAL", "BETWEEN_NOT_MATCHED", "EXTRA_BRACKET",
    "INVALID_OPERATOR"};
extern map<string, TABLE> tables;
void print_tables(void) {
  cout << endl;
  for (auto it = tables.begin(); it != tables.end(); it++) {
    cout << it->first << endl;
    for (auto jt = it->second.collum.begin(); jt != it->second.collum.end();
         jt++)
      cout << "\t" + jt->first + "\t" + jt->second.type << endl;
  }
}
char filename[] = "data.txt";
RecordManager rm(filename);
int main(void) {
  string s;
  non_volatile_init();
  // for (int i = 0; i < 10000; ++i) {
  std::ios::sync_with_stdio(false);
  do {
    getline(cin, s);
    // expr a(s);
    // int i=a.parse(0,1)&&a.balance();
    // while(!error_queue.empty())error_queue.pop();
    int i = sql_parse(s);
    if (i == -1) break;
    if (i) {
      // cout << endl << "parsing complete"<<endl;
      // print_tables();
    } else {
      cout << endl << "bad" << endl;
      while (!error_queue.empty()) {
        pair<int, int>& a = error_queue.front();
        cout << ERRORS[a.first - 1] << " @" << a.second << endl;
        error_queue.pop();
      }
      // break;
    }

  } while (1);
  // system("pause");
  return 0;
}
//  ACCEPTED = 1,
//  BRACKET_LEFT,
//  BRACKET_RIGHT,
//  INVALID_LITERAL,
//  BETWEEN_NOT_MATCHED,
//  EXTRA_BRACKET,
//  INVALID_OPERATOR

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5.
//   转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
