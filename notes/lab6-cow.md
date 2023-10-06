# Lab 6 COW 实验记录

Lab copy-on-write fork要求:

## 6.1 Implement copy-on write

### 1 要求

实现copy-on-write fork功能

### 2 实现

1. 修改`uvmcopy()`，主要完成：A. 将原来的重新为页表分配物理内存，改为映射到parent的物理内存 B. 将两者的PTE_W设置为0

```c
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  pte_t *pte_new;
  uint64 pa, i;
  uint flags;
  // char *mem;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");
    // get parent's pa
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte) & (~PTE_W); // ++

    if((pte_new = walk(new, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    // clear pte_w
    // if((mem = kalloc()) == 0)
    //   goto err;
    // memmove(mem, (char*)pa, PGSIZE);
    if(mappages(new, i, PGSIZE, pa, flags) != 0){ // ++
      // kfree(mem); // --
      goto err;
    }
  }
  return 0;
    // ...
}
```

2. 修改`usertrap()`以处理page faults

要求的行为：When a page-fault occurs on a COW page, allocate a new page with kalloc(), copy the old page to the new page, and install the new page in the PTE with `PTE_W` set. 因为要判断是否cow mapping，所以首先需要在riscv.h中声明一个macro：

```c
// riscv.h
#define PTE_C (1L << 5) // 1 -> is cow mapping
```

然后在uvmcopy中设置相应的flag：

```c
    flags = PTE_FLAGS(*pte) & (~PTE_W) | PTE_C; // ++
```



### 3 reflection



## Grade

