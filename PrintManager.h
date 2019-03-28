#include <time.h>
#ifndef A3_PRINTMANAGER
#define A3_PRINTMANAGER
#define MAX_REQUEST_SIZE 1000
#define PRINT_JOBS 25
struct PrintRequest{
    int completed = -1;
    int active = -1;
    int jobSize = -1;
    int userID = -1;
    int printNum = -1;
    timespec timestamp;
    char data[MAX_REQUEST_SIZE] = {0};
};

struct PrintNode{
    int previous = -1;
    int next = -1;
    PrintRequest PR;
    int id;
};
//PrintRequest *printerQueue;
PrintRequest createPrintRequest(int ID, int bytes);

void printManagerInit();

int addPrintRequest(PrintRequest pr);
PrintRequest takePrintRequest();//Take next PrintRequest in queue.

long getTimeDiff(timespec time1, timespec time2);

int getPreviousPrint();

#endif