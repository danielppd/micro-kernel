// kernel/irq.c
#include "idt.h"
#include "util.h"
#include <stdint.h>

/* Remapeia o PIC para 0x20–0x2F */
static inline void pic_remap(void) {
    outb(0x20, 0x11); /* ICW1 */
    outb(0xA0, 0x11);

    outb(0x21, 0x20); /* vetor mestre = 0x20 */
    outb(0xA1, 0x28); /* vetor escravo = 0x28 */

    outb(0x21, 0x04); /* mestre: escravo em IRQ2 */
    outb(0xA1, 0x02); /* escravo: cascade ID */

    outb(0x21, 0x01); /* ICW4 */
    outb(0xA1, 0x01);

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

#define IRQ_STUB(n) \
extern void irq##n(void); \
__asm__( \
".globl irq" #n "\n" \
"irq" #n ":\n" \
"  cli\n" \
"  pushl $" #n "\n" \
"  jmp irq_common\n" \
);

IRQ_STUB(0) IRQ_STUB(1) IRQ_STUB(2) IRQ_STUB(3) IRQ_STUB(4) IRQ_STUB(5)
IRQ_STUB(6) IRQ_STUB(7) IRQ_STUB(8) IRQ_STUB(9) IRQ_STUB(10) IRQ_STUB(11)
IRQ_STUB(12) IRQ_STUB(13) IRQ_STUB(14) IRQ_STUB(15)

void irq_handler_c(int irq);

__asm__(
".globl irq_common\n"
"irq_common:\n"
"  pusha\n"
"  mov  32(%esp), %eax\n"
"  push %eax\n"
"  call irq_handler_c\n"
"  add  $4, %esp\n"
"  popa\n"
"  add  $4, %esp          # remove IRQ number empilhado pelo stub\n"
"  iret\n"
);

static inline void pic_eoi(int irq) {
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void irq_install(void) {
    pic_remap();

    extern void irq0(void);  extern void irq1(void);  extern void irq2(void);  extern void irq3(void);
    extern void irq4(void);  extern void irq5(void);  extern void irq6(void);  extern void irq7(void);
    extern void irq8(void);  extern void irq9(void);  extern void irq10(void); extern void irq11(void);
    extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);

    void* irqs[] = { irq0,irq1,irq2,irq3,irq4,irq5,irq6,irq7,irq8,irq9,irq10,irq11,irq12,irq13,irq14,irq15 };
    for (int i=0; i<16; i++)
        idt_set_gate(0x20 + i, (uint32_t)irqs[i], 0x08, 0x8E);
}

__attribute__((weak)) void on_irq(int irq) { (void)irq; }

void irq_handler_c(int irq) {
    on_irq(irq);        // chama o hook do dispositivo (ex.: teclado em irq == 1)
    // End Of Interrupt no PIC (escravo se irq>=8, depois mestre)
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
