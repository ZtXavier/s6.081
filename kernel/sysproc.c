#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
// 我们需要使用这个函数
extern pte_t* walk(pagetable_t, uint64, int);

int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // 因为我们需要记录哪些已经被访问的页
  // 这里是需要存放记结果,如果被访问则
  // 需要返回记录的结果
  uint64 bitMask = 0;
  // 这里需要临时变量
  uint64 Startva;
  uint64 bitMaskva;
  int pagenum;


  // 这里实验给出了argint和argaddr两个函数
  // 需要我们来获取要读取的页面的数量
  if(argint(1, &pagenum) < 0) {
    return -1;
  }

  // 如果读取页面的数量超过了最大读取的范围
  if(pagenum > MAXSCAN) {
    return -1;
  }

  // 读取页面开始地址和指向用户态存放的bitmask指针
  if(argaddr(0, &Startva) < 0) {
    return -1;
  }
  if(argaddr(2, &bitMaskva) < 0) {
    return -1;
  }
  
int i;
pte_t *pte;
// 从起始地址开始,逐页判断PTE_A是否被置位
// 如果被置位,则设置对应的bitmask的位,并将PTE_A清空
  for(i = 0; i < pagenum; Startva += PGSIZE, ++i) {
    if((pte = walk(myproc()->pagetable, Startva, 0)) == 0) {
      panic("pgaccess :walk failed");
    }
    if(*pte & PTE_A) {
      bitMask |= 1 << i;
      *pte &= ~PTE_A; 
    }
  }
  // 将内核的页表地址拷贝到用户态
  copyout(myproc()->pagetable, bitMaskva, (char*)&bitMask, sizeof(bitMask));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
