#include "keyboard.h"
#include "serial.h"
#include "../arch/i386/io.h"
#include "../arch/i386/isr.h"

#define KBD_BUFFER_SIZE 256

static volatile char kbd_buffer[KBD_BUFFER_SIZE];
static volatile int kbd_head = 0;
static volatile int kbd_tail = 0;

static bool shift_pressed = false;
static bool caps_lock = false;
static bool ctrl_pressed = false;
static bool extended_code = false;

static const char scancode_ascii_lower[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   /* Right Shift */
    '*',
    0,   /* Alt */
    ' ', /* Space */
    0,   /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1-F10 */
    0,   /* Num lock */
    0,   /* Scroll lock */
    0,   /* Keypad 7 */
    0,   /* Keypad 8 */
    0,   /* Keypad 9 */
    '-', /* Keypad - */
    0,   /* Keypad 4 */
    0,   /* Keypad 5 */
    0,   /* Keypad 6 */
    '+', /* Keypad + */
    0,   /* Keypad 1 */
    0,   /* Keypad 2 */
    0,   /* Keypad 3 */
    0,   /* Keypad 0 */
    '.', /* Keypad . */
    0, 0, 0,
    0,   /* F11 */
    0,   /* F12 */
};

static const char scancode_ascii_upper[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   /* Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   /* Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0,   /* Right Shift */
    '*',
    0,   /* Alt */
    ' ', /* Space */
    0,   /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1-F10 */
    0,   /* Num lock */
    0,   /* Scroll lock */
};

static void kbd_push_char(char c) {
    int next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_tail) {
        kbd_buffer[kbd_head] = c;
        kbd_head = next;
    }
}

static void keyboard_callback(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(0x60);

    if (scancode == 0xE0) {
        extended_code = true;
        return;
    }

    if (extended_code) {
        extended_code = false;
        switch (scancode) {
            case 0x48: kbd_push_char((char)KEY_UP); break;
            case 0x50: kbd_push_char((char)KEY_DOWN); break;
            case 0x4B: kbd_push_char((char)KEY_LEFT); break;
            case 0x4D: kbd_push_char((char)KEY_RIGHT); break;
            case 0x47: kbd_push_char((char)KEY_HOME); break;
            case 0x4F: kbd_push_char((char)KEY_END); break;
            case 0x53: kbd_push_char((char)KEY_DELETE); break;
            case 0x49: kbd_push_char((char)KEY_PAGE_UP); break;
            case 0x51: kbd_push_char((char)KEY_PAGE_DOWN); break;
            default: break;
        }
        return;
    }

    // Key release (break code)
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) {
            shift_pressed = false;
        } else if (released == 0x1D) {
            ctrl_pressed = false;
        }
        return;
    }

    // Key press (make code)
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }
    if (scancode == 0x1D) {
        ctrl_pressed = true;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode < 128) {
        char c;
        bool use_upper = shift_pressed;
        char lower_c = scancode_ascii_lower[scancode];

        if (lower_c >= 'a' && lower_c <= 'z') {
            if (caps_lock) use_upper = !use_upper;
        }

        if (use_upper) {
            c = scancode_ascii_upper[scancode];
            if (c == 0) c = scancode_ascii_lower[scancode];
        } else {
            c = scancode_ascii_lower[scancode];
        }

        if (ctrl_pressed) {
            // Handle Ctrl+C (0x03), Ctrl+L (0x0C), etc.
            if (lower_c >= 'a' && lower_c <= 'z') {
                c = lower_c - 'a' + 1;
            }
        }

        if (c != 0) {
            kbd_push_char(c);
        }
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback); // IRQ 1 = 33
}

bool keyboard_has_char(void) {
    // Check serial input too!
    if (serial_received()) {
        char sc = inb(COM1_PORT);
        if (sc == '\r') sc = '\n';
        kbd_push_char(sc);
    }
    return kbd_head != kbd_tail;
}

char keyboard_getc(void) {
    if (!keyboard_has_char()) return 0;
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

char keyboard_read_char(void) {
    while (!keyboard_has_char()) {
        hlt();
    }
    return keyboard_getc();
}
