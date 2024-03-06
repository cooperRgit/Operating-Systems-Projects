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

//restore the original trapframe // reset the handling flag // return original a0 register
uint64
sys_sigreturn(void) {
  struct proc *p = myproc();

  //restore the original trapframe state (reverse of usertrap())
  memmove(p->trapframe, p->trapframe_copy, PGSIZE); 

  // Reset handling flag
  p->handling = 0;

  // Return value of a0 from the original trapframe
  return p->trapframe->a0;
}


//kernel side sigalarm system call
uint64
sys_sigalarm(void){
  //current process
  struct proc *p = myproc(); 

  //declare the ticks and the handler
  int ticks; 
  uint64 handler;

  //pass in ticks as the first argument from user side
  argint(0, &ticks);
  //pass in the handler as the second argument from user side
  argaddr(1, &handler);

  //set the process's alarm tick count to the ticks passed in as the argument
  p->alarm_ticks = ticks;
  //set the process's handler to be the handler provided from user side
  p->alarm_handler = handler;

  return 0;
}