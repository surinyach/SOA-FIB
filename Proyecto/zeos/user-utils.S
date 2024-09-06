#include <asm.h>

ENTRY(syscall_sysenter)
	push %ecx
	push %edx
	push $SYSENTER_RETURN
	push %ebp
	mov %esp, %ebp
	sysenter
	
ENTRY(SYSENTER_RETURN)
	pop %ebp
	pop %edx
	pop %edx
	pop %ecx
	ret

/* int write(int fd, char *buffer, int size) */
ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $4, %eax
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int read(char *b, int maxchars) */
ENTRY(read)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;
	movl $3, %eax
	movl 0x8(%ebp), %ebx;	//b
	movl 0xC(%ebp), %ecx;	//maxchars
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok // if (eax < 0) ->
	popl %ebp
	ret

/* int gotoxy(int x, int y) */
ENTRY(gotoxy)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;
	movl $5, %eax
	movl 0x8(%ebp), %ebx;	//x
	movl 0xC(%ebp), %ecx;	//y
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok // if (eax < 0) ->
	popl %ebp
	ret

/* int set_color(int fg, int bg) */
ENTRY(set_color)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;
	movl $6, %eax
	movl 0x8(%ebp), %ebx;	//fg
	movl 0xC(%ebp), %ecx;	//bg
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok // if (eax < 0) ->
	popl %ebp
	ret

/* int gettime() */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int fork() */
ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* void exit() */
ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int yield() */
ENTRY(yield)
	pushl %ebp
	movl %esp, %ebp
	movl $13, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int get_stats(int pid, struct stats *st) */
ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $35, %eax
	movl 0x8(%ebp), %ebx;	//pid
	movl 0xC(%ebp), %ecx;	//st
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* void* shmat(int id, void* addr) */
ENTRY(shmat)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx; 
	movl $7, %eax
	movl 0x8(%ebp), %ebx;	//id
	movl 0xC(%ebp), %ecx;	//addr
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int shmdt(void* addr) */
ENTRY(shmdt)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx; 
	movl $8, %eax
	movl 0x8(%ebp), %ebx;	//addr
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int shmrm(int id) */
ENTRY(shmrm)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx; 
	movl $9, %eax
	movl 0x8(%ebp), %ebx;	//id
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret


/* Common code for negative return */
nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	popl %ebp
	ret

