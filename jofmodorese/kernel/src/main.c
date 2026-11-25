#include "types.h"
#include "bios.h"
#include "timer.h"
#include "isr.h"
#include "task.h"

static void idle_task(void) {
    while (1) {
        task_yield();
    }
}

static void hello_world_task(void) {
    while(1) {
        bios_puts("hello world\r\n");
        task_yield();
    }
}

static void counter_task(void) {
    uint32_t counter = 0;
    while (1) {
        bios_puts("counter ");
        bios_put_hex32(counter++);
        bios_puts(" jiffies ");
        bios_put_hex32(timer_get_jiffies());
        bios_puts("\r\n");
        task_yield();
    }
}

int main(void)
{
    bios_puts("\r\nJofmodore SE kernel\r\n");
    isr_configure();
    task_create("idle", idle_task);
    task_create("counter", counter_task);
    task_create("hello world", hello_world_task);
    task_start();
    return 0;
}