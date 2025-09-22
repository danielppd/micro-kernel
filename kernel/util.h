#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <stdint.h>


static inline void outb(uint16_t port, uint8_t val) {
__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
uint8_t ret;
__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
return ret;
}


void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);


#endif