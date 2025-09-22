#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>


void keyboard_init(void);
int kbd_pop_char(char *out); // 1 se pegou, 0 se vazio


#endif