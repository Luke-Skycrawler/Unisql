#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include "pch.h"
#include <map>
#include <stack>
#include <vector>
#include "DBfile.h"
#define BUFFER_SIZE 32
class BufferManager {
 public:
  BufferManager(char *filename);
  ~BufferManager();
  void initBlockPool(char *filename);      // initialize block 0 and block 1
  blocknode *getNewBlock(char *filename);  // get a new block for recordmanager
  blocknode *getBlock(
      char *filename,
      int blocknum);  // get a known block for recordmanager, not check valid
  void freeBlock(char *filename,
                 int blocknum);  // free a block for recordmanager
  block *getNewBlockBufferUnused(
      const char *filename);  // get a new block without buffer,not check valid
  block *getHeadBlock(char *filename);  // get block 1
  block *getBlockBufferUnused(
      char *filename, int blocknum);  // get a known block without buffer
  void freeBlockBufferUnused(char *filename, int blocknum);  // free block
  void setDirty(blocknode *useblock);
  void writeBlockBacktoDisk(char *filename, block *bufblock);
  blocknode *getBufHeadBlock() const { return headblock; }
  void showBufBlockList();

 private:
  blocknode *buf[BUFFER_SIZE];  // block buffer table to find the block quickly
  std::stack<int> freebuf;      // free buffer in block buffer table
  blocknode *headblock;         // head block
  std::map<int, int>
      mapBuffer;  // key:blocknum in the disk, value:buffer table index
  char *file;
  int bufblocknum;
  void writeAllBlockBack(char *filename);
  int mallocBlock(const char *filename);
};
#endif
