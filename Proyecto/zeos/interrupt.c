/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <sched.h>

#include <zeos_interrupt.h>

#include <system.h>

#include <mm_address.h>
#include <mm.h>
#include <utils.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

int zeos_ticks = 0;

/* Macros for reading the CR2 register*/
#define read_cr2() ({ \
     unsigned int __dummy; \
     __asm__( \
             "movl %%cr2,%0\n\t" \
             :"=r" (__dummy)); \
     __dummy; \
})

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


void p_f_routine(unsigned long eip) 
{

  unsigned int page = read_cr2(); 
  page = page >> 12;  
  unsigned int MAX_PAGE_DATA = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA;
  unsigned int MIN_PAGE_DATA = NUM_PAG_KERNEL + NUM_PAG_CODE; 

  /* Page fault due to COW */ 
  if ((page >= MIN_PAGE_DATA) && (page < MAX_PAGE_DATA)) {
   
    page_table_entry *PT = get_PT(current());
    PT[page].bits.rw = 1; 
    int old_frame = get_frame(PT, page); 

    /* Frame only used by one process */ 
    if (phys_mem[old_frame] == 1) {
      PT[page].bits.rw = 1;
    }

    /* Frame used by more than one process */
    else {
      int new_frame = alloc_frame(); 

      /* There is at least one physical frame free */ 
      if (new_frame != -1) {
        /* Get a free logical page to temporally allocate the parent frame */
        unsigned int aux_page; 
        for (aux_page = 284; aux_page < TOTAL_PAGES; ++aux_page) {
          if (PT[aux_page].bits.present == 0) break; 
        }
        /* Case with an empty logical address */ 
        if (aux_page < TOTAL_PAGES) {
          set_ss_pag(PT, aux_page, old_frame); 
          set_ss_pag(PT, page, new_frame); 
          copy_data((void*)(aux_page << 12), (void*)(page << 12), PAGE_SIZE);
          del_ss_pag(PT, aux_page);
        }

        /* Case with full memory (all pages with valid shared memory) */
        else {
          aux_page = 284; 
          unsigned int shared_frame_holder = get_frame(PT, aux_page);
          set_ss_pag(PT, aux_page, old_frame); 
          set_ss_pag(PT, page, new_frame); 
          copy_data((void*)(aux_page << 12), (void*)(page << 12), PAGE_SIZE);
          set_ss_pag(PT, aux_page, shared_frame_holder);
        }
        --phys_mem[old_frame]; 
      }
      /* No physical frames available */ 
      else {
         printk("\nNo physical frames left!");
         while(1); 
      }
    }
    
    set_cr3(get_DIR(current())); 
    return;  
  }

  else {
    printk("\nProcess generates a PAGE FAULT exception at EIP:@0x");
    hex(eip); 
    while(1);  
  }
}

void clock_routine()
{
  zeos_show_clock();
  zeos_ticks ++;
  
  schedule();
}

void keyboard_routine()
{
  unsigned char c = inb(0x60);
  if (c&0x80) cb_write(&CB,char_map[c&0x7f]);
}

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

void clock_handler();
void keyboard_handler();
void system_call_handler();
void p_f_handler(); 

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void setSysenter()
{
  setMSR(0x174, 0, __KERNEL_CS);
  setMSR(0x175, 0, INITIAL_ESP);
  setMSR(0x176, 0, (unsigned long)system_call_handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(14, p_f_handler, 0); 

  setSysenter();

  set_idt_reg(&idtR);
}
