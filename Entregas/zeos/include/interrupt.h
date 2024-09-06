/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;
extern int zeos_ticks;

/* HANDLERS */ 

void keyboard_handler();
void clock_handler();
void p_f_handler();

/* FAST SYSCALLS */

void writeMSR(long a, long b);
void syscall_handler_sysenter();

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void setIdt();

#endif  /* __INTERRUPT_H__ */
