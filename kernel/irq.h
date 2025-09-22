#ifndef IRQ_H
#define IRQ_H
#include <stdint.h>


void irq_install(void);
void irq_unmask(uint8_t irq);


#endif