#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"
#include "defs.h"

int
systeminfo(uint64 addr)
{
    struct proc *p = myproc();
    struct sysinfo si;

    si.freemem = freemem();
    si.nproc = nproc();

    // copy a struct sysinfo back to user space
    if(copyout(p->pagetable, addr, (char *)&si, sizeof(si)) < 0) {
      return -1;
    }

    return 0;
}