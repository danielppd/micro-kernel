# Microkernel x86 - Jogo da Cobrinha

Este projeto implementa um microkernel completo para arquitetura x86 que executa diretamente no hardware. O sistema demonstra conceitos fundamentais de sistemas operacionais através de um jogo da cobrinha, integrando:

- Bootloader Multiboot em Assembly x86
- Sistema completo de interrupções (IDT, ISR, IRQ)
- Drivers de hardware (VGA Text Mode, PS/2 Keyboard)
- Gerenciamento de memória manual

O projeto foi desenvolvido como atividade acadêmica para demonstrar a compreensão prática dos fundamentos de sistemas operacionais, desde o processo de boot até a implementação de drivers de dispositivos e aplicações interativas.

> [Link para o Vídeo](https://drive.google.com/drive/folders/1TsycsNTjeugrKYlxztTn5hP7Ax99zI5k?usp=sharing)

## 1. Objetivos e Requisitos Atendidos

### 1.1 Requisitos Técnicos Implementados

O desenvolvimento focou no atendimento completo dos requisitos estabelecidos para um microkernel funcional:

1. **Bootloader Assembly:** Implementação de inicialização Multiboot compliant que configura o ambiente de execução e transfere controle para `kernel_main()`
2. **Saída de Vídeo:** Driver VGA em modo texto com acesso direto à memória em 0xB8000, permitindo controle total sobre a apresentação visual
3. **Sistema de Interrupções:** IDT com 256 entradas, ISRs para tratamento de exceções da CPU, e IRQs para gerenciamento de hardware
4. **Input de Hardware:** Driver PS/2 keyboard com processamento via IRQ1 e PIC remapeado para captura de entrada do usuário
5. **Aplicação Interativa:** Jogo da Cobrinha completo com lógica de colisão, sistema de pontuação e interface multilíngue

### 1.2 Funcionalidades do Jogo

A aplicação do "Snake" foi escolhida por demonstrar múltiplos aspectos de programação de sistemas:

- Controle dual através de teclas WASD e setas direcionais
- Sistema TURBO com aceleração ativada por duplo comando na mesma direção
- Persistência de high score entre diferentes execuções
- Detecção precisa de colisão com bordas do campo e próprio corpo
- Geração procedural de comida com algoritmo anti-colisão

## 2. Arquitetura do Sistema

### 2.1 Estrutura Modular

O sistema foi projetado seguindo princípios de modularidade e separação de responsabilidades:

```
micro-kernel/
├── boot.s              # Bootloader Multiboot Assembly
├── linker.ld           # Script de linking para layout de memória
├── Makefile            # Sistema de build automatizado
└── kernel/
    ├── idt.c/h         # Interrupt Descriptor Table
    ├── isr.c/h         # CPU Exception Handlers (0-31)
    ├── irq.c/h         # Hardware Interrupt Handlers + PIC
    ├── keyboard.c/h    # Driver PS/2 Keyboard + FIFO buffer
    ├── vga.c/h         # Driver VGA Text Mode + Color system
    ├── util.c/h        # Primitivas I/O e Memory management
    └── kernel.c        # Kernel principal + lógica do jogo
```

### 2.2 Fluxo de Inicialização

O processo de inicialização segue uma sequência cuidadosamente orquestrada:

1. GRUB/Multiboot carrega o kernel na memória conforme especificação Multiboot
2. O arquivo boot.s configura o stack inicial e transfere controle para kernel_main()
3. Inicialização do sistema de vídeo VGA para permitir saída visual
4. Instalação da IDT (Interrupt Descriptor Table) para gerenciamento de interrupções
5. Configuração dos handlers ISR/IRQ para tratamento de exceções e hardware
6. Inicialização do driver de teclado e habilitação da IRQ1
7. Habilitação global de interrupções através da instrução STI
8. Entrada no loop principal da aplicação (jogo)

## 3. Detalhes Técnicos de Implementação

### 3.1 Sistema de Interrupções

O sistema de interrupções representa o núcleo da comunicação entre hardware e software:

- **IDT (Interrupt Descriptor Table):** Tabela com 256 entradas mapeando vectors de interrupção para handlers específicos
- **ISRs (Interrupt Service Routines):** Tratamento das 32 exceções padrão do x86 (Division Error, Page Fault, General Protection Fault, etc.)
- **IRQs (Interrupt Requests):** PIC remapeado para evitar conflitos, direcionando IRQs 0-15 para interrupções 32-47
- **IRQ1:** Processamento específico do teclado através de I/O dirigido por interrupções

### 3.2 Driver VGA

O driver de vídeo implementa acesso direto ao hardware VGA em modo texto:

- **Resolução:** Modo texto padrão de 80x25 caracteres
- **Formato de dados:** 16 bits por caractere (8 bits ASCII + 8 bits cor/atributo)
- **Sistema de cores:** Palette de 4 bits oferecendo 16 cores para foreground e background
- **Acesso à memória:** Memory-mapped I/O direto no endereço físico 0xB8000

### 3.3 Driver Keyboard

O driver de teclado implementa comunicação com controlador PS/2:

- **Interface de hardware:** Utilização das portas 0x60 (dados) e 0x64 (status/comando)
- **Buffer de entrada:** FIFO circular de 64 bytes para buffering de input
- **Tradução de códigos:** Conversão de scancodes para ASCII com suporte a teclas especiais
- **Mapeamento dual:** Suporte simultâneo para controles WASD e teclas direcionais

## 4. Instruções de Build e Execução

### 4.1 Pré-requisitos do Sistema

Para compilar e executar o projeto, são necessárias as seguintes ferramentas:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install nasm qemu-system-x86 grub-pc-bin xorriso mtools build-essential gcc-multilib

# Opcional: Cross-compiler para x86
sudo apt install gcc-i686-linux-gnu
```

### 4.2 Processo de Compilação

O sistema de build é automatizado através do Makefile:

```bash
# Limpeza e build completo
make clean && make

# Execução direta no QEMU (método recomendado para desenvolvimento)
make run

# Criação de ISO bootável e teste
make iso
make run-iso

# Limpeza de arquivos temporários
make clean
```

### 4.3 Controles da Aplicação

O jogo responde aos seguintes comandos de entrada:

- **WASD** ou **Teclas direcionais:** Controle de movimento da cobra
- **Duplo comando:** Pressionamento duplo na mesma direção ativa modo TURBO
- **R:** Reinicialização do jogo após Game Over
- **Q:** Encerramento da aplicação e retorno ao kernel

## 5. Resultados e Validação

### 5.1 Testes Funcionais Realizados

O sistema passou por bateria abrangente de testes para validação:

- **Teste de Boot:** Verificação da inicialização correta via GRUB e protocolo Multiboot
- **Teste de Interrupções:** Validação do funcionamento de ISRs e IRQs sem General Protection Faults
- **Teste de Hardware:** Confirmação da responsividade adequada dos drivers VGA e Keyboard
- **Teste de Lógica:** Verificação do funcionamento correto da detecção de colisão, crescimento da cobra e sistema de pontuação
- **Teste de Performance:** Validação da estabilidade do timing e responsividade do sistema

### 5.2 Ambiente de Validação

Os testes foram realizados no seguinte ambiente controlado:

- **Emulador:** QEMU System i386 versão atual
- **Arquitetura alvo:** x86-32 (IA32)
- **Memória disponível:** 32MB RAM
- **Método de boot:** Multiboot via GRUB

