#ifndef DBFILE_H
#include "pch.h"
#define DBFILE_H
#define BLOCK_SIZE 4096
#define MAX_TABLENUM_SIZE 1022
#include <string>
#include <vector>
// block
struct block {
  int blocknum;
  char *data;  // 4KB
};
struct blocknode : block {
  bool dirty;
  bool lock;
  blocknode *nextblock;
  blocknode *preblock;
  /*data field(real block in disk)
  *variable:
  *header: blocknum, recordnum, freespaceend
  *entry: recordposition, recordsize
  *free space
  *records
  *fixed-length:
  *header:nextblock 4B,recordnum 4B,freespacelist 4B,recordlist 4B,
                  (total_recordnum 4B, record_size 4B)
  *record:nextrecord 4B, real_data
  *freespace
  */
};
// tableblock in db
/*
 *headernode: one block
 *nexttable 4B, freelist 4B, attribute_size 4B
 *attribute(max(32)) 32B    total: 32 * 32B = 1KB
 *node:
 *nexttable	4B
 *tablenum_size 4B
 *blocknumoftable(tablenum)	4B
 */

/*
 *dbname 120B(only in the first block)
 *tablesize 4B, nextblock 4B
 *tablename 124B, tablelist_headnum 4B
 */

/*
 *blockpool(linked list)
 *freeblock:
 *nextblock 4B
 *endblock:
 *0 4B
 */
#endif
