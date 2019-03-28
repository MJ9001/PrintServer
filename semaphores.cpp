#include <semaphore.h>
#include "semaphores.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

void waitSem(Sema *sem)
{
    sem_wait(&(sem->gateSem));
    sem_wait(&(sem->mutexSem));
    sem->valSem = sem->valSem - 1;
    if (sem->valSem > 0) {
        sem_post(&(sem->gateSem));
    }
    sem_post(&(sem->mutexSem));
}

void signalSem(Sema *sem)
{
    sem_wait(&(sem->mutexSem));
    sem->valSem = sem->valSem + 1;
    if (sem->valSem == 1)
        sem_post(&(sem->gateSem));
    sem_post(&(sem->mutexSem));
}

/*void *CSem(void *K)// counting sem, init K
{
    int gateVal = 0, mutexVal = 0;
    printf("Starting thread.\n");
    //valSem = *(int*)K; // value of csem
    waitSem();
    usleep(10);
    sem_getvalue(&gateSem, &gateVal); 
    sem_getvalue(&mutexSem, &mutexVal); 
    printf("valSem= %d, gate= %d, mutex=%d\n", valSem, gateVal, mutexVal);
    signalSem(); // protects valSem
}*/

void initSem(Sema *sem)
{
    sem_init(&(sem->gateSem), 1, (sem->valSem == 0) ? 0 : 1);
    sem_init(&(sem->mutexSem), 1, 1);
}

/*
int main()
{
    pthread_t tid[3];
    int i, *arg, result;
    
    sem_init(&gateSem, 0, (valSem > 0) ? 1 : 0);
    sem_init(&mutexSem, 0, 1);
    printf("Starting value= %d\n", valSem);
    
    for(i = 0; i < 3; i++) {
        arg = (int*)malloc(sizeof(int));
        *arg = STARTING_VAL;
        result = pthread_create(&tid[i], NULL, CSem, arg);
        if(result < 0)
        {
            printf("Error creating thread %d!\n", i);
            exit(0);
        }
    }
    for(i = 0; i < 3; i++) {
        result = pthread_join(tid[i], NULL);
        
        if(result < 0)
        {
            printf("Error joining thread %d!\n", i);
            exit(0);
        }
    }
    return 0;
} */