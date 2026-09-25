#include "pic.h"
#include "io.h"

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_init(void) {
    // ICW1: Start initialization in cascade mode
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: Vector offset (Master: 0x20 [32], Slave: 0x28 [40])
    outb(PIC1_DATA, IRQ_OFFSET);
    io_wait();
    outb(PIC2_DATA, IRQ_OFFSET + 8);
    io_wait();

    // ICW3: Cascade wiring
    outb(PIC1_DATA, 4); // Tell Master that Slave is at IRQ2 (0000 0100)
    io_wait();
    outb(PIC2_DATA, 2); // Tell Slave its cascade identity (0000 0010)
    io_wait();

    // ICW4: 8086/88 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore saved masks or enable all
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}
