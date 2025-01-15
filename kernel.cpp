#include <stdint.h>
#include <stdbool.h>
#include <string.h> 


#define VGA_COLOR_WHITE 0x0F
#define VGA_COLOR_BLACK 0x00

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
volatile uint16_t* VGA_BUFFER = (uint16_t*)0xB8000;
uint16_t cursor_position = 0;

char input_buffer[256];
size_t input_length = 0;

extern "C" uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

extern "C" void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

void update_cursor() {
    uint16_t pos = cursor_position;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_BUFFER[i] = (VGA_COLOR_BLACK << 8) | ' ';
    }
    cursor_position = 0;
    update_cursor();
}

void vga_put_char(char c, uint8_t color) {
    if (c == '\n') {
        cursor_position += VGA_WIDTH - (cursor_position % VGA_WIDTH);
    } else if (c == '\b' && cursor_position > 0) {
        cursor_position--;
        VGA_BUFFER[cursor_position] = (color << 8) | ' ';
    } else {
        VGA_BUFFER[cursor_position++] = (color << 8) | c;
    }

    if (cursor_position >= VGA_WIDTH * VGA_HEIGHT) {
        for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
            VGA_BUFFER[i] = VGA_BUFFER[i + VGA_WIDTH];
        }
        for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
            VGA_BUFFER[i] = (color << 8) | ' ';
        }
        cursor_position -= VGA_WIDTH;
    }

    update_cursor();
}

void vga_put_string(const char* str, uint8_t color) {
    while (*str) {
        vga_put_char(*str++, color);
    }
}

#define KEY_A        0x1E
#define KEY_B        0x30
#define KEY_C        0x2E
#define KEY_D        0x20
#define KEY_E        0x12
#define KEY_F        0x21
#define KEY_G        0x22
#define KEY_H        0x23
#define KEY_I        0x17
#define KEY_J        0x24
#define KEY_K        0x25
#define KEY_L        0x26
#define KEY_M        0x32
#define KEY_N        0x31
#define KEY_O        0x18
#define KEY_P        0x19
#define KEY_Q        0x10
#define KEY_R        0x13
#define KEY_S        0x1F
#define KEY_T        0x14
#define KEY_U        0x16
#define KEY_V        0x2F
#define KEY_W        0x11
#define KEY_X        0x2D
#define KEY_Y        0x15
#define KEY_Z        0x2C
#define KEY_1        0x02
#define KEY_2        0x03
#define KEY_3        0x04
#define KEY_4        0x05
#define KEY_5        0x06
#define KEY_6        0x07
#define KEY_7        0x08
#define KEY_8        0x09
#define KEY_9        0x0A
#define KEY_0        0x0B
#define KEY_ENTER    0x1C
#define KEY_BACKSPACE 0x0E

bool key_pressed[256] = { false };

char scancode_to_char(uint8_t scancode) {
    switch (scancode) {
        case KEY_A: return 'a';
        case KEY_B: return 'b';
        case KEY_C: return 'c';
        case KEY_D: return 'd';
        case KEY_E: return 'e';
        case KEY_F: return 'f';
        case KEY_G: return 'g';
        case KEY_H: return 'h';
        case KEY_I: return 'i';
        case KEY_J: return 'j';
        case KEY_K: return 'k';
        case KEY_L: return 'l';
        case KEY_M: return 'm';
        case KEY_N: return 'n';
        case KEY_O: return 'o';
        case KEY_P: return 'p';
        case KEY_Q: return 'q';
        case KEY_R: return 'r';
        case KEY_S: return 's';
        case KEY_T: return 't';
        case KEY_U: return 'u';
        case KEY_V: return 'v';
        case KEY_W: return 'w';
        case KEY_X: return 'x';
        case KEY_Y: return 'y';
        case KEY_Z: return 'z';
        case KEY_1: return '1';
        case KEY_2: return '2';
        case KEY_3: return '3';
        case KEY_4: return '4';
        case KEY_5: return '5';
        case KEY_6: return '6';
        case KEY_7: return '7';
        case KEY_8: return '8';
        case KEY_9: return '9';
        case KEY_0: return '0';
        case KEY_ENTER: return '\n';
        case KEY_BACKSPACE: return '\b';
        default: return 0;
    }
}

void process_command(const char* command) {
    if (command[0] == 'n' && command[1] == 'e' && command[2] == 'o' && command[3] == 'f' &&
        command[4] == 'e' && command[5] == 't' && command[6] == 'c' && command[7] == 'h' && command[8] == '\0') {
        vga_put_string("\nVER: 0.0.1\n", VGA_COLOR_WHITE);
        vga_put_string("KERNEL: NEBULA\n", VGA_COLOR_WHITE);
        vga_put_string("Designed by koredev 2025\n", VGA_COLOR_WHITE);
    } else {
        vga_put_string("\nCommand not found: ", VGA_COLOR_WHITE);
        vga_put_string(command, VGA_COLOR_WHITE);
    }
}


void keyboard_interrupt_handler() {
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) {
        key_pressed[scancode & 0x7F] = false;
    } else {
        if (!key_pressed[scancode]) {
            key_pressed[scancode] = true;
            char key = scancode_to_char(scancode);

            if (key) {
                if (key == '\n') {
                    input_buffer[input_length] = '\0'; 
                    process_command(input_buffer);
                    input_length = 0; 
                } else if (key == '\b') {
                    if (input_length > 0) {
                        input_length--;
                        vga_put_char('\b', VGA_COLOR_WHITE);
                    }
                } else {
                    if (input_length < sizeof(input_buffer) - 1) {
                        input_buffer[input_length++] = key;
                        vga_put_char(key, VGA_COLOR_WHITE);
                    }
                }
            }
        }
    }
}

void print_boot_message() {
    vga_put_string("byteOS x86\n", VGA_COLOR_WHITE);
    vga_put_string("DEVELOPMENT KERNEL! EXPECT BUGS\n", VGA_COLOR_WHITE);
    vga_put_string("MIT ByteOS 0.0.1, kernel \"nebula 1.0\"\n", VGA_COLOR_WHITE);
}

void print_prompt() {
    vga_put_string("byteW> ", VGA_COLOR_WHITE);
}

extern "C" void kernel_main() {
    clear_screen();
    print_boot_message();
    print_prompt();

    while (1) {
        keyboard_interrupt_handler();
    }
}
