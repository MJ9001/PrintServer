#include "PrintManager.h"
#include "semaphores.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <csignal>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <cassert>

#define PRINT_JOBS 25
#define MIN_DELAY 100
#define MAX_DELAY 1000
#define MIN_REQUEST_SIZE 100
#define MAX_REQUEST_SIZE 1000

#define SORTING_MODE 1

using namespace std;



void delayRandom();
int getRndJobs();
int main(int argc, char **args);
void *printer(void *id);
void producer(int id);
void onExit(int s);
void exitSems();
void printResults();
void processPrint(PrintRequest pr);
int generateRequestSize();
long int randseed;
long totalWaitTime = 0;
long totalJobs = 0;
long totalBytes = 0;
Sema *queueFullSem, *queueEmptySem, *rwSem;
int *previousPR;

int activePrints = 0;

void delayRandom()
{
    usleep((int)(MIN_DELAY + ((double)rand() / RAND_MAX) * (MAX_DELAY - MIN_DELAY)));
}

int getRndJobs()
{
    return round(((double)rand() / RAND_MAX) * 30);
}

int generateRequestSize()
{
    //return 500;
    return MIN_REQUEST_SIZE + round((double)rand() / RAND_MAX * 
        (MAX_REQUEST_SIZE - MIN_REQUEST_SIZE));
}

void processPrint(PrintRequest pr)
{
    //Print the request (byte count -> delay length)
    
    usleep(pr.jobSize*10);
    
    timespec *currTime = (timespec*)malloc(sizeof(timespec));
    clock_gettime(CLOCK_REALTIME, currTime);
    long waitTime = getTimeDiff(pr.timestamp, *currTime);
    totalWaitTime += waitTime;
    
    totalJobs++;
    totalBytes += pr.jobSize;
    
    cout << "Print " << pr.printNum << " took " << waitTime << "ms.\n";
}

Sema* createSem(int id, int strtValue)
{
    int shID = shmget (IPC_PRIVATE , sizeof(Sema), IPC_CREAT | 0666);
    Sema* SEMA = (Sema*)shmat(shID, NULL, 0);
    SEMA->valSem = strtValue;
    return SEMA;
}

void initSems()
{
    queueFullSem = createSem(1, 24);
    initSem(queueFullSem);
    queueEmptySem = createSem(2, 1);
    initSem(queueEmptySem);
    waitSem(queueEmptySem);
    rwSem = createSem(3, 1);
    initSem(rwSem);
}

void exitSems()
{
    
}


/**
*Command line paramaters:
*1 - Number of producers(user) processes
*2 - number of printer(consumer) threads
**/
int main(int argc, char **args)
{
    int producers, printers, forked, i, *tmp;
    string getlineStr;
    if(argc > 4)
    {
        cout << "Too many arguments!\n";
        exit(-1);
    }
    
    if(argc < 3)
    {
        cout << "Paramaters: [filename] [# of users] [# of printers] [argument]\n";
        exit(-1);
    }
    
    //Set producers and consumers count
    producers = stoi(args[1]);
    printers = stoi(args[2]);
    
    pthread_t threads[printers];
    
    
    if(producers <= 0 || printers <= 0)
    {
        cout << "Paramaters: [filename] [# of users] [# of printers] [arguments]\n";
        exit(-1);
    }
    cout << "Printer server starting...\n";
    //Detect Ctrl+C
    signal(SIGINT, onExit);
    
    randseed = static_cast<long int>(time(0)); //Set random seed for threads and processes
    srand(randseed);
    
    //Initialize semaphores
    initSems();
    
    //Initialize Print Manager
    printManagerInit();

    //Previous Print Request init. for processes
    int shID = shmget (IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);
    previousPR = (int*)shmat(shID, NULL, 0);
    
    //Create printers
    for(i = 0; printers > i; i++)
    {
        tmp = (int *)malloc(sizeof(int));
        *tmp = i;
        pthread_create(&threads[i], NULL, &printer, (void *)tmp);
    }
    cout << "Printers created.\n";
    //Create producers (users)
    for(i = 0; producers > i; i++)
    {
        if (fork() != 0)
        {
            producer(i);
            return 0;
        }
    }
    cout << "Producers created.\n";
    
    while(1)
    {
        
        //cout << "VALUE: " << *TEST << endl;
    }
    /*int t = 0;
    while(t++ < 100)
    {
        getline(cin, getlineStr);
        cout << "VALUE: " << *TEST << endl;
    }*/
}

void onExit(int s)
{
    cout << "Exiting printer server.\n";
    exit(1);
}

int prnts = 0;
void *printer(void *id) //Consumer
{
    PrintRequest pr;
    cout << "Printer: " << *(int *)id << " up.\n";
    srand(randseed + *(int *)id);
    while(1)
    {
        waitSem(queueEmptySem);
        waitSem(rwSem);
        //cout << "Call cout: " << ++prnts << "\n\n";
        //cout << "Rw signal\n";
        //Grab a print off of the queue.
        pr = takePrintRequest();
        
        signalSem(rwSem);
        signalSem(queueFullSem);
        //Process the print Request
        processPrint(pr);
        
        
    }
}

void producer(int id)
{
    srand(randseed + 10000 + id);
    int jobs = getRndJobs();
    //waitSem(rwSem);
    cout << "User: " << id << " up with " << jobs << " jobs.\n";
    //signalSem(rwSem);
    for(int i = 0; i < jobs; i++)
    {
        waitSem(queueFullSem);
        waitSem(rwSem);
        int bytes = generateRequestSize();
        cout << "Creating print request, ID: " << id << " size: " << bytes << ".\n";
        //Create PrintRequest
        PrintRequest pr = createPrintRequest(id, bytes);
        
        //Add it to the queue
        int prRes = addPrintRequest(pr);
        assert(prRes == 0);
        
        int PP = *previousPR;
        *previousPR = id;
        signalSem(rwSem);
        signalSem(queueEmptySem);
        //If same user add print twice in a row randomly delay.
        if(PP == id)
            delayRandom();
        
    }
    
    while(1);
    //exit(0);
}