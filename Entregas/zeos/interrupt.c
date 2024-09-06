/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <libc.h>
#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'',' ','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l',' ',
  '\0',' ','\0',' ','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();
  
  setInterruptHandler(33, keyboard_handler,0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(14, p_f_handler, 0);
  
  writeMSR(0x174, __KERNEL_CS);
  writeMSR(0x175, INITIAL_ESP);
  writeMSR(0x176, (unsigned long)syscall_handler_sysenter);
  
  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

  set_idt_reg(&idtR);
}

/* ROUTINE */

void keyboard_routine() {  

  unsigned char keyboard_data = inb(0x60);
  
  if(!(keyboard_data>>7)) {
    keyboard_data &= 0x7F;
    if(char_map[keyboard_data] != '\0' && keyboard_data < sizeof(char_map))
    	printc_xy(0, 24, char_map[keyboard_data]);	
    else printc_xy(0, 24, 'C');
  }
   
}

void clock_routine(){
  ++zeos_ticks;
  //if (zeos_ticks == 3000) task_switch((union task_union*)temporal_child); TESTING FORK
  zeos_show_clock();
  schedule();
}


//bin to hex
void hex(unsigned long n) {
	char *data ="0123456789ABCDEF";
	char buffer[1];
	for(int i = 28; i >= 0; i -= 4) {
		buffer[0] = data[(n >> i) & 0xF];
		printk(buffer); 
	}
	return;
}


void p_f_routine(unsigned long param, unsigned long eip) {
  printk("\nProcess generates a PAGE FAULT exception at EIP:@0x");
  hex(eip); 
  while(1);  
}






/* LONG BIN TO HEX
void p_f_routine(unsigned long param, unsigned long eip) {
  char buffer [8];  
  for (int i = 0; i < 8; i += 2) {
	 char aux = eip & 0x000000FF;
	 char lower = aux & 0x0F; 
      	 char upper = (aux & 0xF0) >> 4; 
	 if (lower >= 10) lower = 'A' + (lower - 10); 
         else lower = '0' + lower;
	 if (upper >= 10) upper = 'A' + (upper - 10); 
         else upper = '0' + upper;
         buffer[8-i-1] = lower; 
	 buffer[8-i-2] = upper; 	 
	 eip = eip >> 8; 
  }
  printk("\nProcess generates a PAGE FAULT exception at EIP:@0x");
  printk(buffer); 
  while(1);  
}
*/


































