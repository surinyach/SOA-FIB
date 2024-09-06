/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <interrupt.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return (struct task_struct*)((unsigned int)l&0xfffff000);
}

extern struct list_head blocked;

/* Colas de ready y free */
struct list_head freequeue;

struct list_head readyqueue;

/* Task Struct del idle_task*/
struct task_struct *idle_task=NULL;

int remaining_ticks = 200;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	struct list_head *first = list_first(&freequeue);
	list_del(first);
	idle_task = list_head_to_task_struct(first);
	idle_task->PID = 0;
	set_quantum(idle_task, 100);
	allocate_DIR(idle_task);
	INIT_LIST_HEAD(&(idle_task->child_list));
	union task_union *ut_idle = (union task_union*)idle_task; 
	ut_idle->stack[KERNEL_STACK_SIZE-1] = (unsigned long)cpu_idle;
	ut_idle->stack[KERNEL_STACK_SIZE-2] = 0;
	idle_task->kernel_esp = (int)&(ut_idle->stack[KERNEL_STACK_SIZE-2]);
}

void init_task1(void)
{
	struct list_head *first = list_first(&freequeue);
	list_del(first);
	struct task_struct *ts = list_head_to_task_struct(first);
	ts->PID = 1;
	set_quantum(ts, 200);
	allocate_DIR(ts);
	set_user_pages(ts);
	INIT_LIST_HEAD(&(ts->child_list));
	INIT_LIST_HEAD(&ts->brother_list); 
	ts->pending_unblocks = 0;
	union task_union *ut_task1 = (union task_union*)ts;
	tss.esp0 = (unsigned long)&ut_task1->stack[KERNEL_STACK_SIZE];
	writeMSR(0x175,(unsigned long)&ut_task1->stack[KERNEL_STACK_SIZE]);
	set_cr3(ts->dir_pages_baseAddr);
}


void init_sched() 
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	INIT_LIST_HEAD(&blocked);
	for(int i = 0; i < NR_TASKS; ++i) {
		list_add_tail(&(task[i].task.list), &freequeue);
	}
}

void inner_task_switch(union task_union *new) 
{
	tss.esp0 = (unsigned long)&new->stack[KERNEL_STACK_SIZE];
	writeMSR(0x175, (unsigned long)&new->stack[KERNEL_STACK_SIZE]);
	set_cr3(get_DIR((struct task_struct *)new));
	current()->kernel_esp = ts_save_ebp();
	ts_change_esp(new->task.kernel_esp); 
}

int get_quantum (struct task_struct *t) {
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

void update_sched_data_rr() {
	--remaining_ticks;
}

int needs_sched_rr() {
	if(current()->state == ST_BLOCKED) return 1;
	if(remaining_ticks == 0 && !list_empty(&readyqueue)) return 1;
	if(remaining_ticks == 0)remaining_ticks = get_quantum(current());
	return 0;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue) {
	if(t->state == ST_RUN) {
		t->state = ST_READY;
		list_add_tail(&(t->list), dst_queue);
	}
	else if(t->state == ST_READY) {
		t->state = ST_RUN;
		list_del(&(t->list));
	}
}

void sched_next_rr() {
	struct list_head *new_lh;
	struct task_struct *new_ts;
	if(list_empty(&readyqueue)) {
		remaining_ticks = get_quantum(idle_task);
		task_switch((union task_union*)idle_task);
	}
	else {
		new_lh = list_first(&readyqueue);
		new_ts = list_head_to_task_struct(new_lh);
		update_process_state_rr(new_ts, NULL);
		remaining_ticks = get_quantum(new_ts);
		union task_union *ut = (union task_union*)new_ts;
		task_switch(ut);
	}
}

void schedule() {
	update_sched_data_rr();
	if(needs_sched_rr()) { 
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

