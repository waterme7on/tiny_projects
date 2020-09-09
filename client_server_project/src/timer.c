#include<time.h>
#include<stdio.h>

typedef struct {
    time_t start;
    time_t cur;
} timer;


void start(timer t) {
    t.start = time(NULL);
}

void stop(timer t) {
    t.cur = time(NULL);
}

void print_time(timer t) {
    printf("time taken: %f\n", difftime(t.cur, t.start));
};
