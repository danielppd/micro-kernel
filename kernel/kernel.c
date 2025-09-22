// kernel/kernel.c
#include <stdint.h>
#include "vga.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "util.h"

// Snake Game - Estruturas e variáveis
#define MAX_SNAKE_LENGTH 100
#define GAME_WIDTH 78
#define GAME_HEIGHT 22

typedef struct {
    int x, y;
} Point;

static Point snake[MAX_SNAKE_LENGTH];
static int snake_length = 3;
static Point food;
static int score = 0;
static int high_score = 0; // Nova: maior pontuação
static int dx = 1, dy = 0; // direção inicial: direita
static int game_running = 1;
static int game_over = 0;
static int game_started = 0; // Novo: controla se o jogo começou
static uint32_t game_tick = 0;
static uint32_t delay_counter = 0;
static int speed_boost = 0; // Sistema de aceleração
static uint32_t base_speed = 50; // Velocidade base
static uint32_t boost_speed = 20; // Velocidade acelerada

static void draw_hud(void){
    // Desenha o HUD sempre na primeira linha (y=0) em posição fixa
    const char *msg = "JOGO DA COBRINHA — WASD/Setas: mover, Q: sair, R: reiniciar";
    for(int i=0; msg[i] && i<60; ++i) {
        vga_putat(msg[i], 0x0A, i, 0);  // verde, linha 0
    }
}

static void init_snake(void) {
    snake_length = 3;
    // Posição inicial segura (dentro da área de jogo)
    snake[0].x = 10; snake[0].y = 12; // cabeça
    snake[1].x = 9;  snake[1].y = 12; // corpo
    snake[2].x = 8;  snake[2].y = 12; // cauda
    dx = 0; dy = 0; // Sem movimento inicial - esperando comando
    
    // Atualiza high score se necessário
    if (score > high_score) {
        high_score = score;
    }
    
    score = 0;
    game_over = 0;
    game_started = 0; // Começa pausado
    game_running = 1;
    game_tick = 0; // Reset do timer também
    delay_counter = 0;
    speed_boost = 0; // Reset do turbo
}

static void spawn_food(void) {
    static uint32_t seed = 5381;
    seed = ((seed << 5) + seed) + (inb(0x60) & 0xFF);
    
    // Gera posição aleatória que não colida com a cobra
    do {
        food.x = 1 + (seed % 78);  // x de 1 a 78
        food.y = 2 + (seed % 21);  // y de 2 a 22
        seed = ((seed << 5) + seed) + 1;
        
        // Verifica se não está na cobra
        int collision = 0;
        for(int i = 0; i < snake_length; i++) {
            if(snake[i].x == food.x && snake[i].y == food.y) {
                collision = 1;
                break;
            }
        }
        if(!collision) break;
    } while(1);
}

static void draw_borders(void) {
    // Bordas horizontais
    for(int x = 0; x < 80; x++) {
        vga_putat('#', 0x08, x, 1);  // topo
        vga_putat('#', 0x08, x, 23); // baixo
    }
    // Bordas verticais
    for(int y = 1; y < 24; y++) {
        vga_putat('#', 0x08, 0, y);  // esquerda
        vga_putat('#', 0x08, 79, y); // direita
    }
}

static void draw_world(void){
    // Limpa a tela
    for(int y=0;y<25;y++) {
        for(int x=0;x<80;x++) {
            vga_putat(' ',0x00,x,y);
        }
    }
    
    draw_hud();
    draw_borders();
    
    // Desenha a cobra
    for(int i = 0; i < snake_length; i++) {
        char c = (i == 0) ? 'O' : 'o'; // cabeça diferente do corpo
        uint8_t color = (i == 0) ? 0x0E : 0x0A; // cabeça amarela, corpo verde
        vga_putat(c, color, snake[i].x, snake[i].y);
    }
    
    // Desenha a comida
    vga_putat('*', 0x0C, food.x, food.y);
    
    // Score atual
    const char *label = "Pontos:";
    for(int i=0; label[i]; ++i) vga_putat(label[i],0x0F,0+i,24);
    
    // Converte score para string simples
    char score_str[10];
    int temp_score = score;
    int digits = 0;
    if(temp_score == 0) {
        score_str[0] = '0';
        digits = 1;
    } else {
        while(temp_score > 0) {
            score_str[digits++] = '0' + (temp_score % 10);
            temp_score /= 10;
        }
        // Inverte os dígitos
        for(int i = 0; i < digits/2; i++) {
            char temp = score_str[i];
            score_str[i] = score_str[digits-1-i];
            score_str[digits-1-i] = temp;
        }
    }
    
    for(int i = 0; i < digits; i++) {
        vga_putat(score_str[i], 0x0F, 8 + i, 24);
    }
    
    // High Score
    const char *high_label = " | Recorde: ";
    for(int i=0; high_label[i]; ++i) vga_putat(high_label[i],0x0B,9+i,24);
    
    // Converte high score para string
    char high_str[10];
    int temp_high = high_score;
    int high_digits = 0;
    if(temp_high == 0) {
        high_str[0] = '0';
        high_digits = 1;
    } else {
        while(temp_high > 0) {
            high_str[high_digits++] = '0' + (temp_high % 10);
            temp_high /= 10;
        }
        // Inverte os dígitos
        for(int i = 0; i < high_digits/2; i++) {
            char temp = high_str[i];
            high_str[i] = high_str[high_digits-1-i];
            high_str[high_digits-1-i] = temp;
        }
    }
    
    for(int i = 0; i < high_digits; i++) {
        vga_putat(high_str[i], 0x0B, 21 + i, 24);
    }
    
    // Indicador de velocidade
    if(speed_boost) {
        const char *speed_msg = " [TURBO!]";
        for(int i=0; speed_msg[i]; ++i) vga_putat(speed_msg[i],0x0E,35+i,24);
    }
    
    // Game Over message
    if(game_over) {
        const char *msg = "FIM DE JOGO! Pressione R para reiniciar";
        int start_x = (80 - 39) / 2;
        for(int i = 0; msg[i]; i++) {
            vga_putat(msg[i], 0x0C, start_x + i, 12);
        }
    }
    // Start message
    else if(!game_started) {
        const char *msg1 = "Pressione WASD ou Setas para comecar!";
        const char *msg2 = "Pressione duas vezes na mesma direcao para TURBO!";
        int start_x1 = (80 - 38) / 2;
        int start_x2 = (80 - 50) / 2;
        for(int i = 0; msg1[i]; i++) {
            vga_putat(msg1[i], 0x0E, start_x1 + i, 11);
        }
        for(int i = 0; msg2[i]; i++) {
            vga_putat(msg2[i], 0x0A, start_x2 + i, 13);
        }
    }
}

static int check_collision(int new_x, int new_y) {
    // Colisão com bordas (área de jogo é de x=1 a x=78, y=2 a y=22)
    if(new_x < 1 || new_x > 78 || new_y < 2 || new_y > 22) {
        return 1;
    }
    
    // Colisão com próprio corpo (ignora a cabeça)
    for(int i = 1; i < snake_length; i++) {
        if(snake[i].x == new_x && snake[i].y == new_y) {
            return 1;
        }
    }
    
    return 0;
}

static void move_snake(void) {
    if(game_over) return;
    
    // Nova posição da cabeça
    int new_x = snake[0].x + dx;
    int new_y = snake[0].y + dy;
    
    // Verifica colisões
    if(check_collision(new_x, new_y)) {
        game_over = 1;
        return;
    }
    
    // Move o corpo (cada segmento vai para posição do anterior)
    for(int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i-1];
    }
    
    // Move a cabeça
    snake[0].x = new_x;
    snake[0].y = new_y;
    
    // Verifica se comeu comida
    if(snake[0].x == food.x && snake[0].y == food.y) {
        score++;
        if(snake_length < MAX_SNAKE_LENGTH) {
            snake_length++;
        }
        spawn_food();
        // Reset do turbo ao comer (opcional - pode remover se quiser manter)
        // speed_boost = 0;
    }
}

void kernel_main(void) {
    vga_init();
    vga_write("Iniciando IDT/IRQs...\n");
    idt_install();
    isr_install();
    irq_install();
    keyboard_init();
    __asm__ volatile ("sti");

    vga_write("Pronto! Iniciando Jogo da Cobrinha...\n\n");
    
    // Inicializa o jogo
    init_snake();
    spawn_food();
    draw_world();

    while (game_running) {
        char c;
        int redraw = 0;
        
        // Processa input do teclado
        if (kbd_pop_char(&c)) {
            if (c == 'q') {
                // Atualiza high score antes de sair
                if (score > high_score) {
                    high_score = score;
                }
                game_running = 0;
            } else if (c == 'r' && game_over) {
                // Reinicia o jogo
                init_snake();
                spawn_food();
                redraw = 1;
            } else if (!game_over) {
                // Controles de direção (WASD + Setas)
                if (!game_started) {
                    // No início, qualquer direção é permitida
                    if (c == 'w' || c == 1) { dx = 0; dy = -1; game_started = 1; speed_boost = 0; }      // W ou Seta UP
                    else if (c == 's' || c == 2) { dx = 0; dy = 1; game_started = 1; speed_boost = 0; } // S ou Seta DOWN
                    else if (c == 'a' || c == 3) { dx = -1; dy = 0; game_started = 1; speed_boost = 0; }// A ou Seta LEFT
                    else if (c == 'd' || c == 4) { dx = 1; dy = 0; game_started = 1; speed_boost = 0; } // D ou Seta RIGHT
                } else {
                    // Durante o jogo, verifica aceleração e mudança de direção
                    if (c == 'w' || c == 1) { // Cima
                        if (dx == 0 && dy == -1) speed_boost = 1; // Já indo para cima - acelera!
                        else if (dy == 0) { dx = 0; dy = -1; speed_boost = 0; } // Muda direção
                    }
                    else if (c == 's' || c == 2) { // Baixo
                        if (dx == 0 && dy == 1) speed_boost = 1; // Já indo para baixo - acelera!
                        else if (dy == 0) { dx = 0; dy = 1; speed_boost = 0; } // Muda direção
                    }
                    else if (c == 'a' || c == 3) { // Esquerda
                        if (dx == -1 && dy == 0) speed_boost = 1; // Já indo para esquerda - acelera!
                        else if (dx == 0) { dx = -1; dy = 0; speed_boost = 0; } // Muda direção
                    }
                    else if (c == 'd' || c == 4) { // Direita
                        if (dx == 1 && dy == 0) speed_boost = 1; // Já indo para direita - acelera!
                        else if (dx == 0) { dx = 1; dy = 0; speed_boost = 0; } // Muda direção
                    }
                }
            }
        }
        
        // Movimento automático da cobra com delay pesado
        if (game_started && !game_over) {
            delay_counter++;
            
            // Delay muito mais pesado - múltiplos loops
            if (delay_counter >= 1000) {
                delay_counter = 0;
                
                // Delay adicional com loops aninhados
                for(volatile int i = 0; i < 10000; i++) {
                    for(volatile int j = 0; j < 100; j++) {
                        // Loop vazio para consumir ciclos de CPU
                    }
                }
                
                game_tick++;
                uint32_t current_speed = speed_boost ? boost_speed : base_speed;
                if (game_tick >= current_speed) {
                    game_tick = 0;
                    move_snake();
                    redraw = 1;
                }
            }
        }
        
        if (redraw) {
            draw_world();
        }
    }
    
    vga_write("\nEncerrando Jogo da Cobrinha. Voce pressionou Q.\n");
    for(;;) __asm__ volatile ("hlt");
}
