#include "util.h"
#include "vga.h"

#define KBD_DATA   0x60
#define KBD_STATUS 0x64

static volatile char fifo[64];
static volatile int head=0, tail=0;

static const char scancode_to_ascii[128] = {
/*0x00*/ 0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', 8,
/*0x10*/ 9,  'q','w','e','r','t','y','u','i','o','p','[',']','\\', 0,
/*0x20*/ 'a','s','d','f','g','h','j','k','l',';','\'','`', 13, 0,
/*0x30*/ 'z','x','c','v','b','n','m',',','.','/', 0, 0, 0, ' ', 0, 0,
};

static inline void push(char c){ int n=(head+1)&63; if(n!=tail){ fifo[head]=c; head=n; } }
int kbd_pop_char(char *out){ if(tail==head) return 0; *out=fifo[tail]; tail=(tail+1)&63; return 1; }

/* Handler real do teclado (IRQ1) */
static void kbd_irq_handler(void) {
    uint8_t sc = inb(KBD_DATA);
    if (sc & 0x80) return; /* ignore releases */
    
    char c = 0;
    
    // Scancode das setas (códigos especiais)
    if (sc == 0x48) c = 1;    // Seta para cima (UP)
    else if (sc == 0x50) c = 2;  // Seta para baixo (DOWN)
    else if (sc == 0x4B) c = 3;  // Seta para esquerda (LEFT)
    else if (sc == 0x4D) c = 4;  // Seta para direita (RIGHT)
    else if (sc < sizeof(scancode_to_ascii)) c = scancode_to_ascii[sc];
    
    if (c) push(c);
}

/* Hook chamado pelo dispatcher de IRQ: agora recebemos o número da IRQ */
void on_irq(int irq) {
    if (irq == 1) {
        /* Verificação adicional para evitar problemas */
        if (inb(0x64) & 0x01) { /* Data available */
            kbd_irq_handler();
        }
    }
}

/* Habilita a IRQ1 no PIC (desmascara) */
void keyboard_init(void) {
    // desmascara IRQ1
    uint8_t mask = inb(0x21);
    mask &= ~(1<<1);
    outb(0x21, mask);
}
