#pragma once

#include "Typedefs.h"

typedef struct Clock {
    f64 StartTime;
    f64 Elapsed;
} Clock;

void Clock_Init();

void Clock_Start(Clock* clock);
void Clock_Update(Clock* clock);
