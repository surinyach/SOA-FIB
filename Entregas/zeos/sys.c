/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <interrupt.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

char data[512];

int globalpid = 95;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork() {
	return 0;
}

int sys_fork() {

  // a)
  if(list_empty(&freequeue)) 
  	return -ENOMEM;

  struct list_head *first = list_first(&freequeue);
  list_del(first);
  struct task_struct *child_ts = list_head_to_task_struct(first);

  union task_union *child_tu = (union task_union *)child_ts;
  union task_union *parent_tu = (union task_union *)current();

  // b)
  copy_data(parent_tu, child_tu, KERNEL_STACK_SIZE*4);

  // c)
  allocate_DIR(child_ts);

  // d)
  int frames[NUM_PAG_DATA];
  int new_frame, i, j;
  for(i = 0; i < NUM_PAG_DATA; ++i) {
  	new_frame = alloc_frame();
  	if(new_frame != -1) 
  		frames[i] = new_frame;
  	else {
  		for(j = 0; j < i; ++j)
  			free_frame(frames[j]);
  		list_add_tail(first, &freequeue);
  		return -EAGAIN;			//añadir al perror en lib.c
  	}
  }

  // e)
  page_table_entry *child_pt = get_PT(child_ts);
  for(i = 0; i < NUM_PAG_DATA; ++i) 
  	set_ss_pag(child_pt, PAG_LOG_INIT_DATA+i, frames[i]);
 
  // f)
  // Compartido (system code, system data)
  page_table_entry *parent_pt = get_PT(current());
  for(i = 0; i < NUM_PAG_KERNEL; ++i)
  	set_ss_pag(child_pt, i , get_frame(parent_pt, i));

  // Compartido (user code)
  for(i = 0; i < NUM_PAG_CODE; ++i) {
  	set_ss_pag(child_pt, PAG_LOG_INIT_CODE+i , get_frame(parent_pt, PAG_LOG_INIT_CODE+i));
  }

  // Copiamos user data y user stack
  int ESPACIO_LIBRE = NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE;
  int ESPACIO_COPIA = NUM_PAG_KERNEL;

  for(i = 0; i < NUM_PAG_DATA ; ++i) {
  	set_ss_pag(parent_pt, ESPACIO_LIBRE+i, frames[i]);
  	copy_data((void*)((ESPACIO_COPIA+i) << 12), (void*)((ESPACIO_LIBRE+i) << 12), PAGE_SIZE); 
  	del_ss_pag(parent_pt, ESPACIO_LIBRE+i);
  }
  set_cr3(get_DIR(current()));

  // g)
  child_tu->task.PID = ++globalpid;
  //block and unblock modifications
  child_tu->task.parent = current();      //añadimos puntero al padre
  child_tu->task.pending_unblocks = 0;
  list_add_tail(&(child_tu->task.brother_list), &current()->child_list);     //añadimos el hijo al la lista de hijos del padre
  INIT_LIST_HEAD(&(child_tu->task.child_list)); 

  // h) i)
  child_tu->stack[KERNEL_STACK_SIZE-18] = (unsigned long)ret_from_fork;
  child_tu->stack[KERNEL_STACK_SIZE-19] = 0;
  child_ts->kernel_esp =(unsigned long)&child_tu->stack[KERNEL_STACK_SIZE-19];
   
  // j)
  child_tu->task.state = ST_READY;
  list_add_tail(&child_ts->list, &readyqueue);

  // k)
  return child_tu->task.PID;
}

void sys_block() {
  if(current()->pending_unblocks == 0) {
    current()->state = ST_BLOCKED;
    list_add_tail(&(current()->list), &blocked);
    schedule();
  }
  else current()->pending_unblocks -= 1;
}

int sys_unblock(int pid) {
	struct task_struct* ts; 
	struct list_head* lh; 
	//for (lh = (&(current()->child_list))->next; lh != (&(current()->child_list)); lh = lh->next) {
	list_for_each(lh, &current()->child_list) {
		ts = list_head_to_task_struct(lh);
		if (pid == ts->PID) {
			if (ts->state == ST_BLOCKED) {
				ts->state = ST_READY; 
				list_del(&(ts->list));
				list_add_tail(&(ts->list), &readyqueue); 
				return 0; 
			}
			else {
				ts->pending_unblocks += 1;
				return 0; 
			}		
		}
	}
	return -1; 
}

void sys_exit() {
	  struct list_head* check_init = list_first(&current()->brother_list);
  	if (check_init != &current()->brother_list) list_del(&(current()->brother_list)); 
  	struct list_head* header = list_first(&current()->child_list); 
  	while(header != &current()->child_list) {
  		list_del(header);
  		list_add_tail(header, &idle_task->child_list);
  		header = list_first(&current()->child_list);
  	}

	page_table_entry *pt = get_PT(current());
	for(int i = 0; i < NUM_PAG_DATA; ++i) {
		free_frame(get_frame(pt, PAG_LOG_INIT_DATA + i));
		del_ss_pag(pt, PAG_LOG_INIT_DATA + i);		
	}
	list_add_tail(&current()->list, &freequeue);
	sched_next_rr();
}

int sys_write (int fd, char * buffer, int size)
{
	int check = check_fd(fd, ESCRIPTURA);
	if(check < 0) return -EBADF;
	if(!access_ok(0,buffer,size)) return -EFAULT;
	if(buffer == NULL) return -EFAULT;
	if(size < 0) return -EINVAL;
	
	int written = 0;
	while(written < size) {
		check = copy_from_user(buffer, data, size);
		if (check < 0) return -1;
		written += sys_write_console(data, size);
		buffer += 512;
	}
	
	return written;
}

int sys_gettime()
{
	return zeos_ticks;
}





