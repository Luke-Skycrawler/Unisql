#include "pch.h"
#include "BufferManager.h"
#include <cstdio>
#include <cstring>
#include <iostream>

#define insertDoubleList(insertblock, headblock, preblock, nextblock) \
  {                                                                   \
    insertblock->preblock = headblock->preblock;                      \
    headblock->preblock->nextblock = insertblock;                     \
    insertblock->nextblock = headblock;                               \
    headblock->preblock = insertblock;                                \
  }

#define deleteDoubleList(deleteblock, preblock, nextblock)     \
  {                                                            \
    deleteblock->preblock->nextblock = deleteblock->nextblock; \
    deleteblock->nextblock->preblock = deleteblock->preblock;  \
  }

BufferManager::BufferManager(char *filename) {
  memset(buf, 0, sizeof(blocknode *) * BUFFER_SIZE);
  headblock = NULL;
  bufblocknum = 0;
  file = new char[strlen(filename) + 1];
  strcpy(file, filename);
  initBlockPool(filename);
  for (int i = 0; i < BUFFER_SIZE; ++i) {
    freebuf.push(i);
  }
}

BufferManager::~BufferManager() {
  // writeback
  writeAllBlockBack(file);
  // free the block list
  blocknode *tempblock;
  while (bufblocknum--) {
    tempblock = headblock;
    headblock = headblock->nextblock;
    delete tempblock->data;
    delete tempblock;
  }
}

void BufferManager::initBlockPool(char *filename) {
  FILE *fp = fopen(filename, "rb+");
  if (fp == NULL) {
    std::cout << filename << " open failed when initBlockPool" << std::endl;
    exit(1);
  }
  fseek(fp, 0L, SEEK_END);
  int filesize = ftell(fp);
  if (filesize >= 2 * BLOCK_SIZE) return;
  fseek(fp, 0L, SEEK_SET);
  long blockpool = 2L;
  char diskblock[BLOCK_SIZE];
  memset(diskblock, 0, sizeof(char) * BLOCK_SIZE);
  memcpy(diskblock, &blockpool, 4);
  if (fwrite(diskblock, BLOCK_SIZE, 1, fp) == 1)  // block 0
  {
    std::cout << "write success" << std::endl;
  }
  memset(diskblock, 0, sizeof(char) * 4);
  fwrite(diskblock, BLOCK_SIZE, 1, fp);  // block 1
  fclose(fp);
}

block *BufferManager::getHeadBlock(char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    std::cout << filename << " open failed when getHeadBlock" << std::endl;
    exit(1);
  }
  fseek(fp, 0L, SEEK_END);
  int filesize = ftell(fp);
  if (filesize < 2 * BLOCK_SIZE) {
    std::cout << filename << " is not initialized" << std::endl;
    return NULL;
  }
  block *headerblock = new block;
  headerblock->blocknum = 1;
  headerblock->data = new char[BLOCK_SIZE];
  fseek(fp, BLOCK_SIZE, SEEK_SET);
  fread(headerblock->data, BLOCK_SIZE, 1, fp);
  return headerblock;
}

int BufferManager::mallocBlock(char *filename) {
  FILE *fp = fopen(filename, "rb+");
  if (fp == NULL) {
    std::cout << filename << " open failed when mallocBlock" << std::endl;
    exit(1);
  }
  fseek(fp, 0L, SEEK_SET);
  long blockpool;
  if (fread(&blockpool, 4, 1, fp) == 0) {
    std::cout << "read error" << std::endl;
  }
  int blocknum = blockpool;
  fseek(fp, blockpool * BLOCK_SIZE, SEEK_SET);
  fgetc(fp);
  if (feof(fp)) {
    fseek(fp, 0L, SEEK_END);
    blockpool += 1;
    // std::cout << "find the file end" << std::endl;
  } else {
    // fseek(fp,-1L,SEEK_CUR);
    fseek(fp, blockpool * BLOCK_SIZE, SEEK_SET);
    fread(&blockpool, 4, 1, fp);
  }
  char stuff[BLOCK_SIZE];  // if stack space is not enough, use heap
  memset(stuff, 0, sizeof(char) * BLOCK_SIZE);
  fwrite(stuff, BLOCK_SIZE, 1, fp);  // allocate a block
  fseek(fp, 0L, SEEK_SET);
  fwrite(&blockpool, 4, 1, fp);
  fflush(fp);
  fclose(fp);
  return blocknum;
}

block *BufferManager::getNewBlockBufferUnused(char *filename) {
  block *newblock = new block;
  newblock->blocknum = mallocBlock(filename);
  newblock->data = new char[BLOCK_SIZE];
  memset(newblock->data, 0, sizeof(char) * BLOCK_SIZE);
  return newblock;
}

blocknode *BufferManager::getNewBlock(char *filename) {
  int blocknum = mallocBlock(filename);
  return getBlock(
      filename,
      blocknum);  // read the data from disk and add the block into buffer
}

blocknode *BufferManager::getBlock(char *filename, int blocknum) {
  std::map<int, int>::iterator iter;
  iter = mapBuffer.find(blocknum);
  if (iter ==
      mapBuffer
          .end())  // can not find this block in the buffer and read from disk
  {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
      std::cout << filename << " open failed" << std::endl;
      exit(1);
    }
    char diskblock[BLOCK_SIZE];
    long offset = blocknum * BLOCK_SIZE;
    fseek(fp, offset, SEEK_SET);
    fread(diskblock, BLOCK_SIZE, 1, fp);
    blocknode *newblock = new blocknode;
    newblock->blocknum = blocknum;
    newblock->dirty = false;
    newblock->lock = false;
    newblock->nextblock = newblock->preblock = newblock;
    newblock->data = new char[BLOCK_SIZE];
    memcpy(newblock->data, diskblock, BLOCK_SIZE);
    if (bufblocknum < BUFFER_SIZE) {
      /*
      int i;
      for(i = 0; i < BUFFER_SIZE; ++i)    //linear search, better: binary
      search, best: freelist if(buf[i] == NULL) break;
      */
      if (freebuf.empty()) {
        std::cout << "stack error" << std::endl;
      }
      int position = freebuf.top();
      freebuf.pop();
      bufblocknum++;
      mapBuffer.insert(std::pair<int, int>(blocknum, position));
      buf[position] = newblock;
      if (headblock != NULL)
        insertDoubleList(newblock, headblock, preblock, nextblock);
      headblock = newblock;

    } else  // LRU strategy
    {
      blocknode *tailblock = headblock->preblock;  // double circular linked
                                                   // list
      while (tailblock->lock == true)  // find the unlocked block
      {
        tailblock = tailblock->preblock;
      }
      iter = mapBuffer.find(tailblock->blocknum);
      if (iter == mapBuffer.end()) {
        std::cout << "can not find the tail block!";
        exit(1);
      }
      int index = iter->second;
      blocknode *tempblock = buf[index];
      deleteDoubleList(tempblock, preblock, nextblock);
      if (tempblock->dirty == true)  // the block has been altered
      {
        writeBlockBacktoDisk(filename,
                             tempblock);  // convert (blocknode *) to (block *)
      }
      {
        delete tempblock->data;
        delete tempblock;
      }
      mapBuffer.erase(iter);
      buf[index] = newblock;
      insertDoubleList(newblock, headblock, preblock, nextblock);
      headblock = newblock;
      mapBuffer.insert(std::pair<int, int>(blocknum, index));
    }
    fclose(fp);
    return newblock;
  } else {
    blocknode *useblock = buf[iter->second];
    if (bufblocknum != 1 && headblock != useblock) {
      // remove from the double circular linked list
      useblock->preblock->nextblock = useblock->nextblock;
      useblock->nextblock->preblock = useblock->preblock;
      // insert the useblock into the head of the list
      useblock->preblock = headblock->preblock;
      headblock->preblock->nextblock = useblock;
      useblock->nextblock = headblock;
      headblock->preblock = useblock;
      // change the most recently used block - headblock
      headblock = useblock;
    }
    return useblock;
  }
}

block *BufferManager::getBlockBufferUnused(char *filename, int blocknum) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    std::cout << filename << " open failed when getBlockBufferUnused"
              << std::endl;
    exit(1);
  }
  block *useblock = new block;
  useblock->blocknum = blocknum;
  useblock->data = new char[BLOCK_SIZE];
  fseek(fp, blocknum * BLOCK_SIZE, SEEK_SET);
  if (fread(useblock->data, BLOCK_SIZE, 1, fp) == 0) {
    std::cout << "read error: " << blocknum << "is invalid" << std::endl;
    exit(1);
  }
  fclose(fp);
  return useblock;
}

void BufferManager::freeBlockBufferUnused(char *filename, int blocknum) {
  FILE *fp = fopen(filename, "rb+");
  // insert the freeing block into blockpool
  long blockpool;
  fseek(fp, 0L, SEEK_SET);
  fread(&blockpool, 4, 1, fp);  // find the first free block

  fseek(fp, (long)blocknum * BLOCK_SIZE, SEEK_SET);
  fwrite(&blockpool, 4, 1, fp);
  // fflush(fp);

  if (fseek(fp, 0L, SEEK_SET) != 0) {
    std::cout << "seek error" << std::endl;
  }
  if (fwrite(&blocknum, 4, 1, fp) == 0) {
    std::cout << "write error" << std::endl;
  }
  fflush(fp);
}

void BufferManager::freeBlock(char *filename, int blocknum) {
  // delete the block in the buffer
  std::map<int, int>::iterator iter;
  iter = mapBuffer.find(blocknum);
  if (iter != mapBuffer.end()) {
    int index = iter->second;
    blocknode *tempblock = buf[index];
    if (tempblock == headblock) {
      headblock = headblock->nextblock;
    }
    buf[index] = NULL;
    mapBuffer.erase(iter);
    freebuf.push(index);
    deleteDoubleList(tempblock, preblock, nextblock);
    {
      delete tempblock->data;
      delete tempblock;
    }
    bufblocknum--;
    if (bufblocknum == 0) headblock = NULL;
  }
  // free the block in the disk
  freeBlockBufferUnused(filename, blocknum);
}

void BufferManager::writeBlockBacktoDisk(char *filename, block *bufblock) {
  if (bufblock == NULL) {
    std::cout << "written block is NULL" << std::endl;
    exit(1);
  }
  if (bufblock->data == NULL) {
    std::cout << "written data is NULL" << std::endl;
    exit(1);
  }
  FILE *fp = fopen(filename, "rb+");
  if (fp == NULL) {
    std::cout << filename << "open failed when write" << std::endl;
    exit(1);
  }
  long offset = bufblock->blocknum * BLOCK_SIZE;
  fseek(fp, offset, SEEK_SET);
  fwrite(bufblock->data, BLOCK_SIZE, 1, fp);
  fclose(fp);
}

void BufferManager::writeAllBlockBack(char *filename) {
  if (headblock == NULL) {
    std::cout << "buf is NULL" << std::endl;
    return;
  }
  blocknode *head = headblock;
  for (int i = 0; i < bufblocknum; ++i) {
    if (headblock == head && i != 0) {
      std::cout << "bufblocknum did not map with headblock" << std::endl;
      exit(1);
    }
    //        for(int j = 0; j < BLOCK_SIZE; ++j)
    //            printf("%02X ",head->data[j]);
    writeBlockBacktoDisk(filename, headblock);
    headblock = headblock->nextblock;
  }
}

void BufferManager::showBufBlockList() {
  blocknode *head = headblock;
  while (1) {
    if (head != NULL) std::cout << head->blocknum << std::endl;
    head = head->nextblock;
    if (head == headblock) break;
  }
}

void BufferManager::setDirty(blocknode *useblock) { useblock->dirty = true; }
/*
int main()
{
    using namespace std;
    char filename[] = "data.txt";
    BufferManager bm(filename);
    block *a = bm.getNewBlockBufferUnused(filename);
    for(int i = 0; i < 40; ++i)
    {
        blocknode *b = bm.getNewBlock(filename);
    }
    bm.freeBlock(filename,3);
    bm.freeBlock(filename,11);
    blocknode *head = bm.getBufHeadBlock();
    blocknode *end =head->preblock;
    
    while(head != end)
    {
        cout << head->blocknum << endl;
        head = head->nextblock;
    }
    
    //bm.freeBlockBufferUnused(filename,3);
    //bm.freeBlockBufferUnused(filename,5);
    //bm.freeBlockBufferUnused(filename,8);
    //bm.getBlockBufferUnused(filename,3);
    //blocknode *a = bm.getNewBlock(filename);
    //blocknode *b = bm.getNewBlock(filename);
    //cout << a->blocknum << endl;
    //cout << b->blocknum << endl;
    //cout << c->blocknum << endl;
    
    // for(int i = 0; i < 34; ++i)
    // {
    //     blocknode *c = bm.getNewBlock(filename);
    // }
    // for(int i = 0; i < 34; ++i)
    // {
    //     bm.getBlock(filename,i+2);
    // }
    // blocknode *c = bm.getBlock(filename,3);
    // bm.getBlock(filename,5);
    // bm.getBlock(filename,15);
    // blocknode *head = bm.getBufHeadBlock();
    // blocknode *end = head;
    // while(true)
    // {
    //     cout << head->blocknum << endl;
    //     head = head->nextblock;
    //     if(head == end)break;
    // }
    system("pause");
    return 0;
}
*/
