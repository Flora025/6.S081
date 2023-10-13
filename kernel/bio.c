// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
extern uint ticks;

struct {
  struct spinlock bucketlock[NBUCKET]; // spin lock for each bucket
  struct spinlock lock;
  struct spinlock hashlock; // a new lock for the hashtable arr

  struct buf buf[NBUF];
  struct buf buckets[NBUCKET]; // hash table buckets

  int bused; // num of buffer in use
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  initlock(&bcache.hashlock, "bcache_hash");
  for (int i = 0; i < NBUCKET; i++) {
    initlock(&bcache.bucketlock[i], "bcache_bucket");
  }

  // distribute according to the hash of each block
  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int hash = blockno % NBUCKET;


  // printf("acquire 1 st\n");
  acquire(&bcache.bucketlock[hash]);
  // printf("acquire 1 fin\n");

  // Is the block already cached?
  // search the corresponding bucket
  for(b = bcache.buckets[hash].next; b; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucketlock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  // printf("acquire 2 st\n");
  acquire(&bcache.lock);
  // printf("acquire 2 fin\n");

  if (bcache.bused < NBUF) {
    // if there is still buffer left
    b = &bcache.buf[bcache.bused++];
    release(&bcache.lock);
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    b->next = bcache.buckets[hash].next;
    bcache.buckets[hash].next = b;

    release(&bcache.bucketlock[hash]);
    acquiresleep(&b->lock);
    return b;
  }
  release(&bcache.lock);
  release(&bcache.bucketlock[hash]);

  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0; // ensures that bread will read the block data
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }

  struct buf *pre, *minb = 0, *minpre;
  int lru;
  

  acquire(&bcache.hashlock);
  for(int i = 0; i < NBUCKET; i++) {
    lru = -1;
    // printf("acquire 4 st\n");
    acquire(&bcache.bucketlock[hash]);
    // printf("acquire 4 fin\n");

    for(pre = &bcache.buckets[hash], b = pre->next; b; pre = b, b = b->next) {
      if((hash == (blockno % NBUCKET)) && b->dev == dev && b->blockno == blockno){
        b->refcnt++;
        release(&bcache.bucketlock[hash]);
        release(&bcache.hashlock);
        acquiresleep(&b->lock);
        return b;
      }
      if(b->refcnt == 0 && b->lastuse < lru) {
        minb = b;
        minpre = pre;
        lru = b->lastuse;
      }
    }
      // find an unused block
    // printf("minb: %d", minb);
    if(minb) {
      minb->dev = dev;
      minb->blockno = blockno;
      minb->valid = 0;
      minb->refcnt = 1;
      // if block in another bucket, we should move it to correct bucket
      if(hash != blockno % NBUCKET) {
        minpre->next = minb->next;    // remove block
        release(&bcache.bucketlock[hash]);
        hash = blockno % NBUCKET;  // the correct bucket index
        // printf("acquire 5 st\n");

        acquire(&bcache.bucketlock[hash]);
        // printf("acquire 5 fin\n");
        minb->next = bcache.buckets[hash].next;    // move block to correct bucket
        bcache.buckets[hash].next = minb;
      }
      release(&bcache.bucketlock[hash]);
      release(&bcache.hashlock);
      // release(&bcache.hashlock);
      acquiresleep(&minb->lock);
      return minb;
    }
    release(&bcache.bucketlock[hash]);
    // update hash
    if(++hash == NBUCKET) {
        hash = 0;
    }
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int hash = b->blockno % NBUCKET;
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  // printf("acquire 6 st\n");
  acquire(&bcache.bucketlock[hash]);
  // printf("acquire 6 fin\n");

  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;

    // time of most recent use
    b->lastuse = ticks;
  }
  
  release(&bcache.bucketlock[hash]);
}

void
bpin(struct buf *b) {
  int hash = b->blockno % NBUCKET;
  // printf("acquire 7 st\n");
  acquire(&bcache.bucketlock[hash]);
  // printf("acquire 7 fin\n");

  b->refcnt++;
  release(&bcache.bucketlock[hash]);
}

void
bunpin(struct buf *b) {
  int hash = b->blockno % NBUCKET;
  // printf("acquire 8 start\n");
  acquire(&bcache.bucketlock[hash]);
  // printf("acquire 8 fin\n");

  b->refcnt--;
  release(&bcache.bucketlock[hash]);
}


