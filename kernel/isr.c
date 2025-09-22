#include "idt.h"
#include "vga.h"
#include <stdint.h>

/* Vamos padronizar: SEMPRE passamos para o handler comum
   dois valores empilhados: (err_code, int_no).
   Para exceções que não geram error code, o stub empilha 0. */

/* Declaração do handler em C */
void isr_handler_c(uint32_t int_no, uint32_t err_code);

/* idt_load(ptr) em ASM (pega argumento pela pilha) */
__asm__(
".globl idt_load\n"
"idt_load:\n"
"  mov  4(%esp), %eax\n"
"  lidt (%eax)\n"
"  ret\n"
);

/* ISR comum: salva regs, chama C(int_no,err_code), limpa a pilha (8 bytes) e iret */
__asm__(
".globl isr_common\n"
"isr_common:\n"
"  pusha\n"
"  mov  32(%esp), %eax    # int_no (depois do pusha)\n"
"  mov  36(%esp), %edx    # err_code (depois do pusha)\n"
"  push %edx              # arg2: err_code\n"
"  push %eax              # arg1: int_no\n"
"  call isr_handler_c\n"
"  add  $8, %esp\n"
"  popa\n"
"  add  $8, %esp          # remove (err_code,int_no) empilhados pelo stub\n"
"  iret\n"
);

/* Macros: com e sem error code */
#define ISR_NOERR(n) \
extern void isr##n(void); \
__asm__( \
".globl isr" #n "\n" \
"isr" #n ":\n" \
"  cli\n" \
"  pushl $0\n"             /* err_code = 0 */ \
"  pushl $" #n "\n"        /* int_no */ \
"  jmp isr_common\n" \
);

#define ISR_WERR(n) \
extern void isr##n(void); \
__asm__( \
".globl isr" #n "\n" \
"isr" #n ":\n" \
"  cli\n" \
"  pushl $" #n "\n"        /* int_no; CPU já empilhou err_code */ \
"  jmp isr_common\n" \
);

/* Exceções com error code: 8, 10–14, 17 */
ISR_NOERR(0)  ISR_NOERR(1)  ISR_NOERR(2)  ISR_NOERR(3)  ISR_NOERR(4)  ISR_NOERR(5)
ISR_NOERR(6)  ISR_NOERR(7)  ISR_WERR(8)   ISR_NOERR(9)  ISR_WERR(10) ISR_WERR(11)
ISR_WERR(12)  ISR_WERR(13)  ISR_WERR(14)  ISR_NOERR(15) ISR_NOERR(16) ISR_WERR(17)
ISR_NOERR(18) ISR_NOERR(19) ISR_NOERR(20) ISR_NOERR(21) ISR_NOERR(22) ISR_NOERR(23)
ISR_NOERR(24) ISR_NOERR(25) ISR_NOERR(26) ISR_NOERR(27) ISR_NOERR(28) ISR_NOERR(29)
ISR_NOERR(30) ISR_NOERR(31)

/* Instala todas as ISRs na IDT */
static void idt_set_isrs(void) {
    extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
    extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
    extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
    extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
    extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
    extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
    extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
    extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);
    extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
    extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

    void* isrs[] = {
        isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,isr12,isr13,isr14,isr15,
        isr16,isr17,isr18,isr19,isr20,isr21,isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31
    };
    for (int i=0;i<32;i++)
        idt_set_gate(i, (uint32_t)isrs[i], 0x08, 0x8E);
}

void isr_install(void) { idt_set_isrs(); }

/* Handler C: log simples e “segura” pra não spammar */
static const char *names[] = {
    "DE", "DB", "NMI","BP","OF","BR","UD","NM",
    "DF","CSO","TS","NP","SS","GP","PF","15",
    "MF","AC","MC","XF","20","21","22","23",
    "24","25","26","27","28","29","30","31"
};

void isr_handler_c(uint32_t int_no, uint32_t err_code) {
    static int exc_count = 0;
    
    /* Evita loop infinito de exceções */
    if (++exc_count > 5) {
        vga_write("PANIC: Muitas excecoes! Sistema travado.\n");
        for(;;) __asm__ volatile("hlt");
    }
    
    vga_write("Excecao capturada (");
    if (int_no < 32) vga_write(names[int_no]); else vga_write("??");
    vga_write(") err=");
    
    /* Print em hexadecimal para mais informação */
    char hex[] = "0123456789ABCDEF";
    vga_putc(hex[(err_code >> 4) & 0xF]);
    vga_putc(hex[err_code & 0xF]);
    vga_putc('\n');

    /* Para exceções críticas, trava o sistema imediatamente */
    if (int_no == 8 || int_no == 13 || int_no == 14) { // Double fault, GP, Page fault
        vga_write("PANIC: Excecao critica! Sistema travado.\n");
        for(;;) __asm__ volatile("hlt");
    }
    
    /* Reset contador após um tempo sem exceções */
    if (exc_count > 0) exc_count--;
}
