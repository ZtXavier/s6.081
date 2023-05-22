// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// lab5 这里我们使用一个全局的引用计数
// 因为考虑到多核的架构,所以锁我们使用
// kmem.lock来同步
uint8 page_Num[PHYSTOP / PGSIZE + 1];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}


// 这个函数用来以页为单位释放[pa_start, pa_end]这个范围内的物理内存。
// pa_start和pa_end函数不一定要是完全页对齐的，这个函数首先会使用
// PGROUNDUP宏将页面强制对齐，然后逐页释放到终点页面。
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  // 这里使用向下取整,是保守的思想,为了防止将有效的页面释放
  // 使字节对齐,每次大小为PGSIZE
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    // 在释放的时候,将我们的引用计数置为1
    // 因为置为1后进入kfree()函数后进行判断释放
    page_Num[(uint64)p / PGSIZE] = 1; 
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  acquire(&kmem.lock);
  // 这里增加了锁,为了防止多核并发
  uint64 i = ((uint64)pa) / PGSIZE;
  page_Num[i]--;
  // 当页面引用计数大于零的时候什么都不必要做
  // 如果小于零的话,需要将该页面添加到freelist中
  if(page_Num[i] > 0) {
    release(&kmem.lock);
    return;
  }
  release(&kmem.lock);
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) 
  {
    kmem.freelist = r->next;
    // 当我们申请一个页面时,初始化该页面计数为1
    page_Num[(uint64)r / PGSIZE] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// 通过虚拟页表来查取物理页表,判断该页表的标记
int cowcheck(pagetable_t page, uint64 rva) {
  // 先判断该虚拟内存的大小是否超出最大值
  if(rva  >= MAXVA) {
    return -1;
  }
  pte_t *pte;
  // 然后我们在多级页表中找到三级页表,并判断是否标记了
  // PTE_COW的flags
  if((pte = walk(page, rva, 0)) == 0 || !(*pte & PTE_COW)) {
    printf("not cow page\n");
    return -1;
  }
  // 如果该页的引用计数大于等于2
  // 我们需要做一个写时复制的操作
  if(page_Num[PTE2PA(*pte) / PGSIZE] >= 2) {
    page_Num[PTE2PA(*pte) / PGSIZE]--;
    char *mem;
    if((mem = kalloc()) == 0) {
      return -1;
    }
    // 具体的过程是如果子进程执行写的操作
    // 我们需要从从父进程中复制该页,并修改
    // 这个页的标记为可写,清楚PTE_COW标记
    memmove(mem, (char*)PTE2PA(*pte), PGSIZE);
    uint flag = PTE_FLAGS(*pte) | PTE_W;
    flag = flag & (~PTE_COW);
    (*pte) = PA2PTE((uint64)mem) | flag;
  } else {
    (*pte) |= PTE_W;
    (*pte) &= (~PTE_COW);
  }
  return 0;
} 

// 这个函数是为了同步的增加某个
// 页面的引用计数
void 
inc_page_num(uint64 pa) {
  int i = pa / PGSIZE;
  acquire(&kmem.lock);
  page_Num[i]++;
  release(&kmem.lock);
}