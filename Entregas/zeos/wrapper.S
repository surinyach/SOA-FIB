#include <asm.h>

ENTRY(write)			
	/* INICIALIZACION */
	pushl %ebp
	movl %esp, %ebp
	/* GUARDAR PARAMETROS USUARIOS */
	pushl %ebx
	pushl %ecx
	pushl %edx 
	/* PASAR PARAMETROS DE LA FUNCION */
	movl 8(%ebp), %edx	
	movl 12(%ebp), %ecx     
	movl 16(%ebp), %ebx
	/* GUARDAR SIGUIENTE @ TRAS SYSENTER */
	pushl $next_w
	/* CARGAR IDENTIFICADOR SYSCALL */
	movl $4, %eax
	/* CREAR FAKE DYNAMIC LINK */
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_w: 
	/* RESTAURAR VALORES Y PILA */
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	popl %ebx
	/* PROCESAR EL RESULTADO */
	cmpl $0, %eax
	jge ok_w
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_w: 
	/* FINALIZACION */
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(gettime)		
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	pushl %ecx
	pushl %edx
	pushl $next_gt
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_gt:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_gt
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_gt:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	pushl %ecx
	pushl %edx
	pushl $next_e
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_e:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_e
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_e:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	pushl %ecx
	pushl %edx
	pushl $next_f
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_f:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_f
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_f:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	pushl %ecx
	pushl %edx
	pushl $next_gp
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_gp:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_gp
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_gp:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(block)
	pushl %ebp
	movl %esp, %ebp
	movl $18, %eax
	pushl %ecx
	pushl %edx
	pushl $next_b
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_b:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_b
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_b:
	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(unblock)
	pushl %ebp
	movl %esp, %ebp
	movl $19, %eax
	pushl %ecx
	pushl %edx
	movl 8(%ebp), %edx
	pushl $next_ub
	pushl %ebp
	movl %esp, %ebp
	sysenter
next_ub:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge ok_ub
	negl %eax
	movl %eax, errno
	movl $-1, %eax
ok_ub:
	movl %ebp, %esp
	popl %ebp
	ret