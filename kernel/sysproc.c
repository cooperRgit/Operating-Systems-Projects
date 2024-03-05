#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "stddef.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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

//sigreturn resets the handling flag and is "supposed" to return the original a0 register value
uint64
sys_sigreturn(void) {
  struct proc *p = myproc();

  if (p->trapframe_copy != 0) {
      // Restore the original trapframe
      memmove(p->trapframe, p->trapframe_copy, sizeof(struct trapframe));

      // Free the memory for tf_copy
      kfree((char*)p->trapframe_copy);
      p->trapframe_copy = 0;
  }

  // Reset handling flag
  p->handling = 0;

  // Return value of a0 from the original trapframe
  return p->trapframe->a0;
}


//sys_sigalarm
uint64
sys_sigalarm(int ticks, void(*handler)()){
  struct proc *p = myproc();

  p->alarm_ticks = ticks;
  p->alarm_handler = handler;

  return 0;
}