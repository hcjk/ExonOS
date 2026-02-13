#pragma once

#include <stdint.h>

typedef void (*task_fn)(void *ctx);

void scheduler_init(void);
int scheduler_add(task_fn fn, void *ctx);
void scheduler_tick(void);
uint32_t scheduler_ticks(void);
