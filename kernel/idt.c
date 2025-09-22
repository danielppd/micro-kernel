#include "idt.h"
#include "util.h"


extern void idt_load(uint32_t);


static struct idt_entry idt[256];
static struct idt_ptr idtp;


void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
idt[num].base_low = base & 0xFFFF;
idt[num].sel = sel;
idt[num].always0 = 0;
idt[num].flags = flags; // 0x8E = present, ring0, 32-bit gate
idt[num].base_high= (base >> 16) & 0xFFFF;
}


void idt_install(void) {
idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
idtp.base = (uint32_t)&idt;
memset(&idt, 0, sizeof(idt));
idt_load((uint32_t)&idtp);
}