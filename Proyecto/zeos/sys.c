/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <system.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern struct circular_buffer CB;

extern Byte x, y;

extern Byte color;

extern struct Shared_frame shared_frames[10]; 

extern int phys_mem[TOTAL_PAGES]; 

char c[256];

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  int pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);

  /* Copy parent's SYSTEM and CODE to child. */   
  page_table_entry *parent_PT = get_PT(current());   
  for (pag=0; pag<NUM_PAG_KERNEL; pag++) {     
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));   
  }   
  for (pag=0; pag<NUM_PAG_CODE; pag++)   {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));   
  }   

  /* Copy parent's DATA to child. */   
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)   {     
  /* Copy on write */      
    unsigned int cow_frame = get_frame(parent_PT, pag);
    set_ss_pag(process_PT, pag, cow_frame);      
    process_PT[pag].bits.rw = 0;
    parent_PT[pag].bits.rw = 0; 
    ++phys_mem[cow_frame];   
  }

  /* Copy parent's SHARED MEMORY */ 
  int frame; 
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA-1; pag<TOTAL_PAGES; ++pag) {
    if (parent_PT[pag].bits.present == 1) {
      frame = get_frame(parent_PT, pag);
      set_ss_pag(process_PT, pag, frame);
      for (i = 0; i < 10; ++i) {
        if (shared_frames[i].frame == frame) 
          ++shared_frames[i].references;
      }
    }
  }

  set_cr3(get_DIR(current())); 

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512


int sys_write(int fd, char *buffer, int nbytes) {
  char localbuffer [TAM_BUFFER];
  int bytes_left;
  int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}

int sys_read(char *b, int maxchars) {
  int cont = 0;

  if(!access_ok(0, b, maxchars)) return -EFAULT;
  if(b == NULL) return -EFAULT;
  if(maxchars < 0) return -EINVAL;

  for (int i = 0; i < maxchars; ++i) {
    c[i] = cb_read(&CB); 
    if (c[i] == '\0') break; 
    ++cont; 
  }

  copy_to_user(&c, b, cont); 
  return cont;
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i, j;

  page_table_entry *process_PT = get_PT(current());

  /* COW liberation of data pages */
  for (i=0; i<NUM_PAG_DATA; i++)
  { 
    int cow_frame = get_frame(process_PT, PAG_LOG_INIT_DATA+i); 
    if (phys_mem[cow_frame] == 1) free_frame(cow_frame);
    else --phys_mem[cow_frame]; 
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }

  /* Shared pages management */ 
  int START_PAGE = NUM_PAG_DATA + NUM_PAG_CODE + NUM_PAG_KERNEL;
  int frame = 0; 
  for (i=START_PAGE; i<TOTAL_PAGES; ++i) {
    if (process_PT[i].bits.present == 1) {
      frame = get_frame(process_PT, i); 
      for (j = 0; j < 10; ++j) {
        if (shared_frames[j].frame == frame) break; 
      }
      --shared_frames[j].references;
      if (shared_frames[j].references == 0 && shared_frames[j].marked)
        memset((void*)(i<<12), 0, PAGE_SIZE);
      del_ss_pag(process_PT, i);      
    }
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

int sys_gotoxy(int new_x, int new_y)
 {
  if (x > 79 || x < 0) return -ECANCELED; 
  if (y > 24 || y < 0) return -ECANCELED;
  x = new_x;
  y = new_y;  
  return 0; 
}

int sys_set_color(int fg, int bg) 
{
  if(fg < 0 || fg > 15 || bg < 0 || bg > 15) return -EINVAL;
  color = 0 | bg << 4 | fg;
  return 0; 

  /* COLORS

  0 BLACK
  1 DARK BLUE
  2 GREEN CONSOLE
  3 DARK CYAN
  4 DARK RED
  5 PURPLE
  6 BROWN
  7 GRAY
  8 DARK GRAY
  9 PASTEL
  10  GREEN CONSOLE BRIGHTER
  11  LIGHT BLUE
  12  RED
  13  PINK  
  14  YELLOW
  15  WHITE

  */

}

int sys_shmat(int id, void* addr) 
{ 
  if (id > 9 || id < 0) return -EINVAL; 
  if (addr != NULL) {
    if (((int)addr & 0x00000FFF) != 0) return -EFAULT;
  }
  else addr = (void*)(284 << 12); 
  unsigned int page = (int)addr >> 12; 
  if (page < 0 || page > 1023) return -EFAULT; 
  
  page_table_entry *PT = get_PT(current()); 
  if (page < 284) page = 284; 
  while ((PT[page].bits.present == 1) && page < 1024) ++page; 
  if (page >= 1024) return -ENOMEM; 

  set_ss_pag(PT, page, shared_frames[id].frame);
  ++shared_frames[id].references;
  return (int)page << 12;
}

int sys_shmdt(void* addr) 
{
  if (addr != NULL) {
    if (((int)addr & 0x00000FFF) != 0) return -EFAULT;
  }
  else addr = (void*)(284 << 12); 
  unsigned int page = (int)addr >> 12; 
  if (page < 0 || page > 1023) return -EFAULT;

  int frame = get_frame(get_PT(current()), page);
  for(int i = 0; i < 10; ++i) {
    if(shared_frames[i].frame == frame) {
      --shared_frames[i].references;
      if(shared_frames[i].references == 0 && shared_frames[i].marked == 1) {
        memset((void*)(frame<<12), 0, PAGE_SIZE);
      }
    }
  }
  del_ss_pag(get_PT(current()), page);
  set_cr3(get_DIR(current()));
  return 0;
}

int sys_shmrm(int id) 
{
  if (id > 9 || id < 0) return -EINVAL;
  shared_frames[id].marked = 1;
  return 0; 
}
  
