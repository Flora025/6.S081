# MIT 6.S081

### 学习方式

Prep -> Lec video&slides -> Lab

### 进度记录

- [x] 【0820】init repo && materials && setup
- [x] 【0821】reading1+lec1



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