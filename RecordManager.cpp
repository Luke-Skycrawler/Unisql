#include "pch.h"
#include "RecordManager.h"
#include <cstring>
#include <iostream>

RecordManager::RecordManager(char *filename) : bufmanager(filename) {}

RecordManager::~RecordManager() {}

recordData *RecordManager::insertRecord(char *filename, const int blocknum,
                                        char *record) {
  blocknode *headerblock = bufmanager.getBlock(filename, blocknum);
  char *datafield = headerblock->data;
  int nextblock, recordnum, freelist, recordlist, total_recordnum, recordsize;
  int curblocknum = blocknum;
  bool isinsert = false;
  // memcpy(&nextblock,datafield,4);
  // memcpy(&recordnum,datafield + 4,4);
  // memcpy(&freelist,datafield + 8,4);
  // memcpy(&recordlist,datafield + 12,4);
  memcpy(&total_recordnum, datafield + 16, 4);
  memcpy(&recordsize, datafield + 20, 4);
  total_recordnum += 1;
  memcpy(datafield + 16, &total_recordnum, 4);
  recordData *newrecord = new recordData;
  newrecord->record = new char[recordsize];
  memcpy(newrecord->record, record, recordsize);
  newrecord->recordsize = recordsize;
  blocknode *useblock = headerblock;
  do {
    datafield = useblock->data;
    memcpy(&nextblock, datafield, 4);
    memcpy(&recordnum, datafield + 4, 4);
    memcpy(&freelist, datafield + 8, 4);
    memcpy(&recordlist, datafield + 12, 4);
    if (freelist + 4 + recordsize <= BLOCK_SIZE) {
      int nextfreespace;
      memcpy(&nextfreespace, datafield + freelist,
             4);  // get the next free space
      memcpy(datafield + freelist, &recordlist, 4);
      memcpy(datafield + freelist + 4, record, recordsize);  // insert the
                                                             // record
      memcpy(datafield + 12, &freelist, 4);  // update recordlist
      newrecord->offset = freelist + 4;
      newrecord->blocknum = curblocknum;
      if (nextfreespace == 0) {
        freelist += 4 + recordsize;
      } else {
        freelist = nextfreespace;
      }
      recordnum += 1;
      memcpy(datafield + 4, &recordnum, 4);  // update recordnum;
      memcpy(datafield + 8, &freelist, 4);   // update free list
      isinsert = true;
      bufmanager.setDirty(useblock);
      break;
    } else if (nextblock) {
      useblock = bufmanager.getBlock(filename, nextblock);
      curblocknum = nextblock;
    }
  } while (nextblock);
  if (isinsert == false) {
    blocknode *newblock = bufmanager.getNewBlock(filename);
    if (newblock == NULL) {
      return NULL;
    }
    newrecord->blocknum = newblock->blocknum;
    // connect newblock to previous useblock
    memcpy(datafield, &newblock->blocknum, 4);
    bufmanager.setDirty(useblock);
    datafield = newblock->data;
    // initialize the header of the block
    memset(newblock->data, 0, BLOCK_SIZE);
    recordnum = 1;
    recordlist = 0;
    freelist = 16;

    memcpy(datafield + 4, &recordnum, 4);
    memcpy(datafield + 8, &freelist, 4);
    memcpy(datafield + 12, &recordlist, 4);

    // insert first record
    memcpy(datafield + freelist + 4, record, recordsize);
    memcpy(datafield + 12, &freelist, 4);
    freelist += 4 + recordsize;
    memcpy(datafield + 8, &freelist, 4);
    newrecord->offset = 20;
    isinsert = true;
    bufmanager.setDirty(newblock);
  }
  return newrecord;
}

std::list<recordData> RecordManager::selectRecord(char *filename,
                                                  const int blocknum) {
  blocknode *headerblock = bufmanager.getBlock(filename, blocknum);
  std::list<recordData> records;
  recordData recordpiece;
  char *datafield = headerblock->data;
  int nextblock, recordnum, freelist, recordlist, total_recordnum, recordsize,
      curblocknum;
  int nextrecord;
  memcpy(&nextblock, datafield, 4);
  // memcpy(&recordnum,datafield + 4,4);
  // memcpy(&freelist,datafield + 8,4);
  // memcpy(&recordlist,datafield + 12,4);
  memcpy(&total_recordnum, datafield + 16, 4);
  memcpy(&recordsize, datafield + 20, 4);
  char *recordcontent = new char[recordsize];
  blocknode *useblock = NULL;
  curblocknum = blocknum;
  while (1) {
    memcpy(&nextblock, datafield, 4);
    memcpy(&recordnum, datafield + 4, 4);
    memcpy(&freelist, datafield + 8, 4);
    memcpy(&recordlist, datafield + 12, 4);

    for (int i = 0; i < recordnum; ++i) {
      memcpy(&nextrecord, datafield + recordlist, 4);
      memcpy(recordcontent, datafield + recordlist + 4, recordsize);
      if (nextrecord == 0 && i != recordnum - 1) {
        std::cout << "recordnum error" << std::endl;
        exit(1);
      }
      recordpiece.record = new char[recordsize];
      memcpy(recordpiece.record, recordcontent, recordsize);
      recordpiece.recordsize = recordsize;
      recordpiece.blocknum = curblocknum;
      recordpiece.offset = recordlist + 4;
      records.push_back(recordpiece);
      recordlist = nextrecord;
    }
    if (nextblock == 0) break;
    useblock = bufmanager.getBlock(filename, nextblock);
    datafield = useblock->data;
    curblocknum = nextblock;
  }
  delete[] recordcontent;
  return records;
}

void RecordManager::deleteRecord(char *filename, const int headblocknum,
                                 recordData record) {
  blocknode *headerblock = bufmanager.getBlock(filename, headblocknum);
  char *datafield = headerblock->data;
  int recordnum, freelist, total_recordnum, recordsize;
  int nextrecord;
  memcpy(&total_recordnum, datafield + 16, 4);
  memcpy(&recordsize, datafield + 20, 4);
  // char *recordcontent = new char[recordsize];
  total_recordnum -= 1;
  memcpy(datafield + 16, &total_recordnum, 4);
  bufmanager.setDirty(headerblock);
  blocknode *alterblock = bufmanager.getBlock(filename, record.blocknum);
  datafield = alterblock->data;
  memcpy(&recordnum, datafield + 4, 4);
  memcpy(&freelist, datafield + 8, 4);
  recordnum -= 1;
  memcpy(datafield + 4, &recordnum, 4);

  // find the record to be deleted
  memcpy(&nextrecord, datafield + record.offset - 4, 4);

  if (nextrecord == 0)  // tail record
  {
    // linear search
    int recordlist;
    memcpy(&recordlist, datafield + 12, 4);
    int currecord = nextrecord = recordlist;
    while (nextrecord != record.offset - 4) {
      currecord = nextrecord;
      memcpy(&nextrecord, datafield + nextrecord, 4);
    }
    if (nextrecord == recordlist) {
      recordlist = 0;
      memcpy(datafield + 12, &recordlist, 4);
    }
    nextrecord = 0;
    memcpy(datafield + currecord, &nextrecord, 4);
    nextrecord = record.offset - 4;
  } else {
    // copy the next record into the position where the record to be deleted
    memcpy(datafield + record.offset - 4, datafield + nextrecord,
           recordsize + 4);
  }
  // add the next record into the free list
  memcpy(datafield + nextrecord, &freelist, 4);
  memcpy(datafield + 8, &nextrecord, 4);

  bufmanager.setDirty(alterblock);
}

int RecordManager::createTable(char *filename, const int recordsize) {
  blocknode *headerblock = bufmanager.getNewBlock(filename);
  char *datafield = headerblock->data;
  // initialize the headerblock
  memset(datafield, 0, BLOCK_SIZE);
  int nextblock = 0;
  int recordnum = 0;
  int freelist = 24;
  int recordlist = 0;
  int total_recordnum = 0;
  memcpy(datafield, &nextblock, 4);
  memcpy(datafield + 4, &recordnum, 4);
  memcpy(datafield + 8, &freelist, 4);
  memcpy(datafield + 12, &recordlist, 4);
  memcpy(datafield + 16, &total_recordnum, 4);
  memcpy(datafield + 20, &recordsize, 4);
  bufmanager.setDirty(headerblock);
  return headerblock->blocknum;
}

void RecordManager::dropTable(char *filename, const int blocknum) {
  blocknode *dropblock;
  char *datafield;
  int nextblock, curblock;
  curblock = blocknum;
  while (curblock != 0) {
    dropblock = bufmanager.getBlock(filename, curblock);
    datafield = dropblock->data;
    memcpy(&nextblock, datafield, 4);
    bufmanager.freeBlock(filename, curblock);
    curblock = nextblock;
  }
}
/*
int main()
{
    using namespace std;
    char record[] = "12345";
    char filename[] = "data.txt";
    RecordManager rm(filename);
    for(int i = 0; i < 10; ++i)
    {
        rm.insertRecord(filename,3,record);
    }
    system("pause");
    return 0;
}
*/
