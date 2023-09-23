# 6.S081

## SP Notes

Each lab has its own branch in the repo. Labs completed so far: 

- [🔗Lab Util](https://github.com/Flora025/6.S081/tree/util)

- [🔗Lab Syscall](https://github.com/Flora025/6.S081/tree/syscall)

## Weekly Roadmap

Prep -> Lec video&slides -> Lab



## Progress

- [x] 【0820】init repo && materials && setup
- [x] 【0821】reading1+lec1
- [x] 【0824】lab util
- [x] 【0912】prep2, lec2(slidesOnly)

- [x] 【0915】review concepts+labUtil, lec 3 prep + live

- [x] 【0917】lab syscall

- [x] 【0920+】prep3, lec4+5+6, lab pgtbl

### W3

Prep:

- Chap3
- kernel/memlayout.h, kernel/vm.c, kernel/kalloc.c, kernel/riscv.h, and kernel/exec.c
- Lec 3&4

Lab pgtbl (Page tables):==具体实验报告见notes==

- Print a page table
  - simply following the hints will do
  - vmprint() (or its helper func) is basically an implementation of preorder tree traversal (if done recursively)

- A Kernal page table per process
  - 需求是给每个process构建一个kernel page table。具体的实现方法是：
    - 仿照kvminit函数中创建kernel page table的过程，在创建process也给p创建一个kpagetable，并保存在stuc proc中
    - 维护每个process中kernel page table和kernel stack的mapping。需要注意的是，原本创建kernel stack是在`procinit()`中完成的，这里为了映射kpagetable和kstack，可以把创建功能移动到`allocproc()`中。

- Simplify `copyin` and `copyout`

### W2

Prep：

- pointers in K&R C programming
- chap2, 4.3, 4.4 of xv6 book

- syscall和process相关的源码

Lab syscall:

- 一开始想复杂了
- 实际上实现trace要做的只是：
  - 给每个process打上"标签" ==> 
  - 并把这个标签传递给子进程 ==>  
  - 最后在syscall()时根据标签判断是否需要print trace

```c
// 【思考过程】
// 最终的效果是：在trace PROC后，如果PROC调用了一个syscall，它和它的child process都会print出要求的trace info

// Usage: trace N ARGV
// 1. add the mask to the proc structure & save the user program

// 2. fork出一个子进程然后exec the USERP
// 记得把mask传给它 （在proc.c里可以加个copy p->tracemask)
// DONE: add a new variable in proc.h proc structure

// 获得user args, see syscall.c
// - 用argaddr(int n, uint64 *ip)获取

// 3. 这个syscall（不出意外）会fork子进程来进行别的syscall
// if (p->tracemask > 0): 按照tracemask print需要trace的东西
```

- 实现syscall：
  - 【准备】完成各种文件里的函数/变量申明
  - `sysproc.c`中call sysinfo()
  - `sysinfo.c`中，call nproc() 和freemem()，然后用copyout()把sysinfo拷贝到user处
  - `kernel/kalloc.c`中实现freemem() ：统计free memory的大小（利用struc run统计N of free page，可以参考函数kalloc()）
  - `kernel/proc.c`中实现nproc() ：统计进程数量（穷举NPROC并判断struc proc里的state以计数）

### W1

Prep work: Read Chapter 1 of http://xv6.dgs.zone/

Lec:

- [a text overview](https://pdos.csail.mit.edu/6.828/2020/lec/l-overview.txt)
- [example materials](https://pdos.csail.mit.edu/6.828/2020/lec/l-overview/)

Lab Util:

- Useful commands:
  - `make qemu`: xv6 booting
  - `ctrl-a + x`: quit qemu
  - `make clean`: clean production files
  - `ctrl-p`: print information about each process
  - `make grade`: local test for all commands
  - `./grade-lab-util someCommand`or `make GRADEFLAGS=someCommand grade`: runs the local tests for a specific command
- Reminder: dont forger to add command to `UPROGS` in Makefile
- Error fix:
  - [Mac M1] `make grade` reports `env: python: No such file or directory`. Solution: `ln -s $(which python3) /usr/local/bin/python`
  - 在user/sh.c的runcmd函数前添加`__attribute__((noreturn))`

## Other Notes

### GBD

About How to use GDB https://zhuanlan.zhihu.com/p/354794701

- `make qemu-gdb`

- in another window && same dir:`riscv64-unknown-elf-gdb`