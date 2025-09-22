#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/* Instala as ISRs (exceções CPU 0..31) na IDT */
void isr_install(void);

#endif /* ISR_H */
