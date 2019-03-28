#include "PrintManager.h"
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>

PrintNode *printerQueue;
int *printerNodeStart; //Start of node array

int* PreviousPrint;
int* TotalPrints;
PrintRequest getNextPrint();

int *unusedNodes;//array of 25 ints
int *unusedNodePos;//defaultly set to 24

timespec *globalTime;

void printManagerInit()
{
    int i;
    //Initalize printer queue
    int shID = shmget (IPC_PRIVATE , (sizeof(PrintNode) * PRINT_JOBS), IPC_CREAT | 0666);
    printerQueue = (PrintNode*)shmat(shID, NULL, 0);
    
    //Create process shared print count
    shID = shmget (IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);
    TotalPrints = (int*)shmat(shID, NULL, 0);
    *TotalPrints = 0;
    
    //Create int for checking for print done twice
    shID = shmget (IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);
    PreviousPrint = (int*)shmat(shID, NULL, 0);
    *PreviousPrint = -1;
    
    //create start of printer node list "pointer"
    shID = shmget (IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);
    printerNodeStart = (int*)shmat(shID, NULL, 0);
    *printerNodeStart = -1;
    
    //create array of unused node "pointers"
    shID = shmget (IPC_PRIVATE , sizeof(int)*PRINT_JOBS, IPC_CREAT | 0666);
    unusedNodes = (int*)shmat(shID, NULL, 0);
    
    //Initalize unused nodes
    for(i = 0; PRINT_JOBS > i; i++) 
        unusedNodes[i] = i;
    
    //create unused node count
    shID = shmget (IPC_PRIVATE , sizeof(int), IPC_CREAT | 0666);
    unusedNodePos = (int*)shmat(shID, NULL, 0);
    *unusedNodePos = PRINT_JOBS - 1;
    
    
    for(i = 0; PRINT_JOBS > i; i++)
    {
        PrintNode pn;
        PrintRequest pr;
        pn.id = i;
        pn.next = -1;
        pn.previous = -1;
        pn.PR = pr;
        printerQueue[i] = pn;
    }
    
    
    //create global time
    shID = shmget (IPC_PRIVATE , sizeof(timespec), IPC_CREAT | 0666);
    globalTime = (timespec*)shmat(shID, NULL, 0);
    clock_gettime(CLOCK_REALTIME, globalTime);
}
//Return a unused node and mark it as used.
int getNode()
{
    return unusedNodes[(*unusedNodePos)--];
}

int addPrintRequest(PrintRequest pr)
{
    int node = getNode();
    printerQueue[node].PR = pr;
    printerQueue[node].next = -1;
    if(*printerNodeStart == -1)
    {
        *printerNodeStart = node;
        printerQueue[node].previous = -1;
        std::cout << "New start of node! " << *printerNodeStart << "\n"; 
    } else
    {
        int pn;
        pn = printerQueue[*printerNodeStart].id;
        while(printerQueue[pn].next > 0)
            pn = printerQueue[pn].next;
        printerQueue[pn].next = node;
        printerQueue[node].previous = pn;
    }
    //std::cout << "Created print request at: " << node << " num: " << printerQueue[node].PR.printNum << std::endl;
    return 0;
}

//Get a print request from PrinterQueue
PrintRequest takePrintRequest()
{
    int nodeTaking = *printerNodeStart;
    
    //std::cout << "Node in use count: " << PRINT_JOBS - 1 - *unusedNodePos << " Starting node: " << nodeTaking << "\n";
    
    if(nodeTaking < 0)
        std::cout << "ERROR!!!! EMPTY NODE!!! " << nodeTaking << "\n";
        
    PrintRequest pr = printerQueue[nodeTaking].PR;
    //Inc. unused node position, and then set node to top of list
    *unusedNodePos = *unusedNodePos + 1;
    unusedNodes[*unusedNodePos] = nodeTaking;
    
    *printerNodeStart = printerQueue[nodeTaking].next; //Remove node from list
    return pr;
}


PrintRequest createPrintRequest(int ID, int bytes)
{   
    PrintRequest *pr = (PrintRequest *)calloc(1, sizeof(PrintRequest));
    pr->completed = 0;
    pr->active = 0;
    pr->jobSize = bytes;
    pr->userID = ID;
    clock_gettime(CLOCK_REALTIME, &(pr->timestamp));
    *TotalPrints = *TotalPrints + 1;
    pr->printNum = *TotalPrints;
    return *pr;
}

int getPreviousPrint()
{
    return *PreviousPrint;
}


long getTimeDiff(timespec time1, timespec time2)//Get time in milliseconds
{
    return (time2.tv_sec - time1.tv_sec) * 1000 + ((time2.tv_nsec - time1.tv_nsec)/1000000 );
}


