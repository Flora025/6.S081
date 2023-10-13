// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for(int i = 0; i < NCPU; i++){
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int cpu;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // get cpuid
  push_off();
  cpu = cpuid();
  pop_off();

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);

}

// steal mem from another cpu's freelist
// return 0 if steal fails
struct run 
*steal(int cpu)
{
  int i;
  
  // check each cpu's freelist
  for (i = 0; i < NCPU; i++) {
    if (i == cpu) continue;

    acquire(&kmem[i].lock);

    if(kmem[i].freelist) {
      // get half of the freelist
      struct run *head = kmem[i].freelist;
      struct run *slow = head;
      struct run *fast = slow->next;
      // 注意c里的写法
      while (fast){
        fast = fast->next;
        if (fast) {
          fast = fast->next;
          slow = slow->next;
        }
      }
      kmem[i].freelist = slow->next;

      slow->next = 0;
      release(&kmem[i].lock);

      return head;
    }
    release(&kmem[i].lock);


  }

  // if all are full, return null;
  return 0;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int cpu;

  // get cpuid
  push_off();
  cpu = cpuid();
  pop_off();

  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if(r)
    kmem[cpu].freelist = r->next;
  release(&kmem[cpu].lock);

  // if freelist is full, steal from another cpu
  if ((!r) && (r = steal(cpu))){
    acquire(&kmem[cpu].lock);
    kmem[cpu].freelist = r->next;
    release(&kmem[cpu].lock);
  }
  
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
