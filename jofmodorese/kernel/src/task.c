#include "types.h"
#include "bios.h"
#include "task.h"
#include "kstring.h"

#define STACK_MAGIC     0xDEADBEEF
#define TASK_INITIAL_SR 0x2200  /* Supervisor mode, interrupt mask 2 (see isr_configure) */
#define SAVED_REGS      15      /* d0-d7/a0-a6, see switch.asm */

static task_t   tasks[MAX_TASKS];
static uint32_t stacks[MAX_TASKS][STACK_SIZE / 4];

task_t *current;

/* Restore context of current task and run it (switch.asm) */
void task_run_first(void);

/* A task whose entry function returns ends up here */
static void task_exit(void) {
    current->state = TASK_ZOMBIE;
    while (1) {
        task_yield();
    }
}

/*
 * Create a task with a stack prepared as if it had been switched out:
 *
 *   high address: task_exit   return address when entry returns
 *                 entry       PC  \ exception frame used by rte
 *                 SR              /
 *   sp ->         d0-d7/a0-a6 all zero
 */
task_t *task_create(const char *name, task_entry_t entry) {
    for (int i = 0; i < MAX_TASKS; i++) {
        task_t *t = &tasks[i];
        if (t->state != TASK_FREE) {
            continue;
        }
        uint32_t *stack = stacks[i];
        uint8_t *sp = (uint8_t *)&stack[STACK_SIZE / 4];

        sp -= 4;
        *(uint32_t *)sp = (uint32_t)task_exit;
        sp -= 4;
        *(uint32_t *)sp = (uint32_t)entry;
        sp -= 2;
        *(uint16_t *)sp = TASK_INITIAL_SR;
        for (int r = 0; r < SAVED_REGS; r++) {
            sp -= 4;
            *(uint32_t *)sp = 0;
        }
        stack[0] = STACK_MAGIC;

        t->sp = (uint32_t)sp;
        t->next = NULL;
        t->state = TASK_READY;
        t->priority = 0;
        t->id = i;
        t->wake_tick = 0;
        t->stack_base = stack;
        strlcpy(t->name, name, TASK_NAME_LEN);
        return t;
    }
    return NULL;
}

/*
 * Select the next task to run, round robin. Called from switch.asm
 * after the context of the current task has been saved.
 */
void sched_next(void) {
    if (current->stack_base[0] != STACK_MAGIC) {
        bios_puts("\r\nStack overflow in task ");
        bios_puts(current->name);
        bios_puts(", halted\r\n");
        while (1);
    }
    if (current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }
    int i = current->id;
    for (int n = 0; n < MAX_TASKS; n++) {
        if (++i == MAX_TASKS) {
            i = 0;
        }
        if (tasks[i].state == TASK_READY) {
            current = &tasks[i];
            break;
        }
    }
    current->state = TASK_RUNNING;
}

void task_start(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_READY) {
            current = &tasks[i];
            current->state = TASK_RUNNING;
            task_run_first();
        }
    }
    bios_puts("\r\nNo task to start, halted\r\n");
    while (1);
}
