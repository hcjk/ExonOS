#include "scheduler.h"

#define MAX_TASKS 8

struct task {
    task_fn fn;
    void *ctx;
};

static struct task tasks[MAX_TASKS];
static uint32_t task_count;
static uint32_t ticks;

void scheduler_init(void) {
    task_count = 0;
    ticks = 0;
}

int scheduler_add(task_fn fn, void *ctx) {
    if (!fn || task_count >= MAX_TASKS) {
        return 0;
    }
    tasks[task_count].fn = fn;
    tasks[task_count].ctx = ctx;
    task_count++;
    return 1;
}

void scheduler_tick(void) {
    ticks++;
    for (uint32_t i = 0; i < task_count; ++i) {
        tasks[i].fn(tasks[i].ctx);
    }
}

uint32_t scheduler_ticks(void) {
    return ticks;
}
