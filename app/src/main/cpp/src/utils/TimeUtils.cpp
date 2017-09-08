#include <time.h>

float getCurrentSystemTimeInSecond() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (float) (time.tv_sec + ((float) time.tv_nsec) / 1e9);
}

long getCurrentSystemTimeInMilli() {
    return (long) (getCurrentSystemTimeInSecond() * 1000);
}