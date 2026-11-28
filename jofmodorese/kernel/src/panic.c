#include "types.h"
#include "bios.h"
#include "task.h"
#include "panic.h"

#define PANIC_STACK_SIZE 512    /* Bytes */

const char *panic_msg;
uint32_t    panic_regs[15];
uint16_t   *panic_group0;
uint16_t   *panic_frame;

/* The panic runs on a stack of its own, isr.asm uses the top */
static uint32_t panic_stack[PANIC_STACK_SIZE / 4];
uint32_t *panic_stack_top = &panic_stack[PANIC_STACK_SIZE / 4];

static void put_reg(const char *name, uint32_t value) {
    bios_puts(name);
    bios_put_hex32(value);
}

/*
 * Interrupts are disabled and the BIOS console is polled, so the
 * output works no matter what state the kernel is in.
 */
void kpanic(void) {
    uint16_t sr = panic_frame[0];
    uint32_t pc = ((uint32_t)panic_frame[1] << 16) | panic_frame[2];

    bios_puts("\r\n*** KERNEL PANIC: ");
    bios_puts(panic_msg);
    bios_puts(" ***\r\n");

    bios_puts("Task: ");
    bios_puts(current ? current->name : "-");
    put_reg("  PC: $", pc);
    bios_puts("  SR: $");
    bios_put_hex16(sr);
    bios_puts("\r\n");

    /*
     * Special status word: bit 4 read (1) / write (0),
     * bit 3 not instruction (1) / instruction fetch (0),
     * bits 2-0 function code.
     */
    if (panic_group0) {
        uint16_t ssw = panic_group0[0];
        put_reg("Fault address: $", ((uint32_t)panic_group0[1] << 16) | panic_group0[2]);
        bios_puts("  IR: $");
        bios_put_hex16(panic_group0[3]);
        bios_puts(ssw & 0x10 ? "  read" : "  write");
        bios_puts(ssw & 0x08 ? " data" : " instruction");
        bios_puts("  FC: ");
        bios_putc('0' + (ssw & 7));
        bios_puts("\r\n");
    }

    for (int i = 0; i < 8; i++) {
        bios_puts(i == 4 ? "\r\n" : i ? " " : "");
        bios_putc('D');
        bios_putc('0' + i);
        put_reg(": $", panic_regs[i]);
    }
    bios_puts("\r\n");
    for (int i = 0; i < 8; i++) {
        bios_puts(i == 4 ? "\r\n" : i ? " " : "");
        bios_putc('A');
        bios_putc('0' + i);
        /* A7 as it was before the CPU pushed SR and PC */
        put_reg(": $", i < 7 ? panic_regs[8 + i] : (uint32_t)panic_frame + 6);
    }
    bios_puts("\r\nSystem halted.\r\n");

    while (1);
}
