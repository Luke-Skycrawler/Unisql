#include "pch.h"
#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include <list>
#include <string>
#include "BufferManager.h"
#include "DBfile.h"
struct recordData {
  char *record;
  int recordsize;
  int blocknum;
  int offset;
};
class RecordManager {
 public:
  RecordManager(char *filename);
  ~RecordManager();
  recordData *insertRecord(char *filename, const int blocknum, char *record);
  std::list<recordData> selectRecord(char *filename, const int blocknum);
  void deleteRecord(
      char *filename, const int headblocknum,
      recordData record);  // delete record in the buffer and then disk
  int createTable(char *filename, const int recordsize);  // return the blocknum
  void dropTable(char *filename, const int blocknum);
  void startExecMode();
  void shutExecMode();
  BufferManager bufmanager;

 private:
  bool isexec;
  int curexecblock;
};
#endif
