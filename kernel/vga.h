#ifndef VGA_H
#define VGA_H
#include <stdint.h>


#define VGA_WIDTH 80
#define VGA_HEIGHT 25


void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_write(const char *s);
void vga_putat(char c, uint8_t color, int x, int y);
void vga_setcolor(uint8_t color);


#endif