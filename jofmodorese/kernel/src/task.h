#ifndef _TASK_H
#define _TASK_H

#include "types.h"

#define MAX_TASKS   16
#define STACK_SIZE  2048    /* Bytes per task */
#define TASK_NAME_LEN 16    /* Including terminator */

typedef enum {
    TASK_FREE = 0,          /* Slot unused (BSS is cleared, so all slots start free) */
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,          /* Waiting for wake_tick */
    TASK_BLOCKED,           /* Waiting for an event */
    TASK_ZOMBIE             /* Finished, slot not yet reclaimed */
} task_state_t;

typedef struct task {
    uint32_t     sp;          /* MUST be first: used at offset 0 by switch.asm */
    struct task *next;        /* Ready queue / wait queue link */
    uint8_t      state;       /* task_state_t */
    uint8_t      priority;
    uint16_t     id;
    uint32_t     wake_tick;
    uint32_t    *stack_base;  /* Lowest address of stack, holds STACK_MAGIC */
    char         name[TASK_NAME_LEN];  /* Copied, always zero-terminated */
} task_t;

typedef void (*task_entry_t)(void);

/* Currently running task (also used by switch.asm) */
extern task_t *current;

/* Create a task, returns NULL if no free slot */
task_t *task_create(const char *name, task_entry_t entry);

/*
 * Create the idle task and start running the first ready task,
 * does not return. The idle task only runs when no other task is
 * ready.
 */
void task_start(void);

/* Give up the CPU to the next ready task (switch.asm) */
void task_yield(void);

/*
 * Called on every timer tick, returns non-zero when the current
 * task should be switched out (time slice used up).
 */
int sched_tick(void);

#endif
