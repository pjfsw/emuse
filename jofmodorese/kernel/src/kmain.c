#include "types.h"
#include "bios.h"
#include "timer.h"
#include "isr.h"
#include "task.h"

/*
 * Neither task yields: they only take turns because the timer
 * interrupt switches between them.
 */
static void hello_world_task(void) {
    uint32_t n = 0;
    while (1) {
        if ((++n & 0x1fff) == 0) {
            bios_puts("hello world\r\n");
        }
    }
}

static void counter_task(void) {
    uint32_t counter = 0;
    while (1) {
        if ((++counter & 0xfff) == 0) {
            bios_puts("counter ");
            bios_put_hex32(counter);
            bios_puts(" jiffies ");
            bios_put_hex32(timer_get_jiffies());
            bios_puts("\r\n");
        }
    }
}

static void crashing_task(void) {
    uint8_t *ptr = 0x10000;
    while (1) {
        uint16_t *ptr2 = ptr;
        bios_put_hex16(*ptr2);
        ptr++;
    }
}

int kmain(void)
{
    bios_puts("\r\nJofmodore SE kernel\r\n");    
    isr_configure();
    task_create("counter", counter_task);
    task_create("hello world", hello_world_task);
    task_create("crashing task", crashing_task);
    task_start();
    return 0;
}
