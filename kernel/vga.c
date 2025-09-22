#include "vga.h"
#include "util.h"


static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;
static uint8_t vga_color = 0x0F; // branco sobre preto
static int cx = 0, cy = 0;


static inline uint16_t vga_entry(char c, uint8_t color) {
return (uint16_t)c | ((uint16_t)color << 8);
}


void vga_setcolor(uint8_t color) { vga_color = color; }


void vga_clear(void) {
for (int y=0; y<VGA_HEIGHT; ++y)
for (int x=0; x<VGA_WIDTH; ++x)
VGA_MEMORY[y*VGA_WIDTH + x] = vga_entry(' ', vga_color);
cx = cy = 0;
}


void vga_init(void) { vga_setcolor(0x0F); vga_clear(); }


static void newline(void) {
cx = 0; cy++;
if (cy >= VGA_HEIGHT) {
// scroll simples
for (int y=1; y<VGA_HEIGHT; ++y)
for (int x=0; x<VGA_WIDTH; ++x)
VGA_MEMORY[(y-1)*VGA_WIDTH + x] = VGA_MEMORY[y*VGA_WIDTH + x];
for (int x=0; x<VGA_WIDTH; ++x)
VGA_MEMORY[(VGA_HEIGHT-1)*VGA_WIDTH + x] = vga_entry(' ', vga_color);
cy = VGA_HEIGHT-1;
}
}


void vga_putc(char c) {
if (c=='\n') { newline(); return; }
VGA_MEMORY[cy*VGA_WIDTH + cx] = vga_entry(c, vga_color);
cx++;
if (cx >= VGA_WIDTH) newline();
}


void vga_write(const char *s) {
while (*s) vga_putc(*s++);
}


void vga_putat(char c, uint8_t color, int x, int y) {
VGA_MEMORY[y*VGA_WIDTH + x] = vga_entry(c, color);
}