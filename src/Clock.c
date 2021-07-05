#include "Clock.h"

#include <Windows.h>

static f64 InverseFrequency = 0.0f;
static u64 InitTime = 0;

void Clock_Init() {
    LARGE_INTEGER frequency = {};
    QueryPerformanceFrequency(&frequency);
    InverseFrequency = 1.0 / cast(f64) frequency.QuadPart;

    LARGE_INTEGER counter = {};
    QueryPerformanceCounter(&counter);
    InitTime = counter.QuadPart;
}

void Clock_Start(Clock* clock) {
    LARGE_INTEGER counter = {};
    QueryPerformanceCounter(&counter);
    clock->StartTime = cast(f64) (counter.QuadPart - InitTime) * InverseFrequency;
    clock->Elapsed = 0.0f;
}

void Clock_Update(Clock* clock) {
    LARGE_INTEGER counter = {};
    QueryPerformanceCounter(&counter);
    f64 time = cast(f64) (counter.QuadPart - InitTime) * InverseFrequency;
    clock->Elapsed = time - clock->StartTime;
}
