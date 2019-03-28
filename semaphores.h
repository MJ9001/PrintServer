#include <semaphore.h>
#ifndef A3_SEMAPHORES
#define A3_SEMAPHORES
#define STARTING_VAL 1
struct Sema{
    sem_t gateSem, mutexSem;
    int valSem = STARTING_VAL;
};

void waitSem(Sema *sem);
void signalSem(Sema *sem);
void initSem(Sema *sem);
#endif