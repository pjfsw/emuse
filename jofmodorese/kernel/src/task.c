#include "types.h"
#include "bios.h"
#include "task.h"
#include "kstring.h"
#include "irq.h"

#define STACK_MAGIC     0xDEADBEEF
#define TASK_INITIAL_SR 0x2200  /* Supervisor mode, interrupt mask 2 (see isr_configure) */
#define SAVED_REGS      15      /* d0-d7/a0-a6, see switch.asm */
#define SLICE_TICKS     3       /* Time slice, 20 ms at 150 Hz */

static task_t   tasks[MAX_TASKS];
static uint32_t stacks[MAX_TASKS][STACK_SIZE / 4];
static task_t  *idle;
static volatile uint8_t slice_left;

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
    uint16_t sr = irq_save();   /* The scheduler must not see a half created task */
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
        irq_restore(sr);
        return t;
    }
    irq_restore(sr);
    return NULL;
}

/* Runs when no other task is ready, the timer switches away from it */
static void idle_task(void) {
    while (1);
}

int sched_tick(void) {
    if (current == NULL) {
        return 0;               /* Scheduler not started yet */
    }
    if (current == idle) {
        return 1;               /* Let a task that became ready run at once */
    }
    return --slice_left == 0;
}

/*
 * Select the next task to run, round robin among the ready tasks,
 * or the idle task if no other task is ready. Called with interrupts
 * disabled from switch.asm and isr.asm, after the context of the
 * current task has been saved.
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
    task_t *next = idle;
    int i = current->id;
    for (int n = 0; n < MAX_TASKS; n++) {
        if (++i == MAX_TASKS) {
            i = 0;
        }
        if (tasks[i].state == TASK_READY && &tasks[i] != idle) {
            next = &tasks[i];
            break;
        }
    }
    current = next;
    current->state = TASK_RUNNING;
    slice_left = SLICE_TICKS;
}

void task_start(void) {
    uint16_t sr = irq_save();   /* No task switch before the first task runs */
    idle = task_create("idle", idle_task);
    if (idle == NULL) {
        irq_restore(sr);
        bios_puts("\r\nNo free slot for idle task, halted\r\n");
        while (1);
    }
    current = idle;
    sched_next();               /* First ready task, or idle */
    task_run_first();           /* Restores the SR of the new task */
}
