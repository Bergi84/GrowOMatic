#include <stdio.h>
#include "pico/stdlib.h"
#include "sequencer_armm0.h"

TSequencer gSequencer;

typedef struct
{
    uint32_t taskNo;
    uint32_t runCnt;
    uint8_t taskId;
    bool wait;
} taskInfo_t;

taskInfo_t gTaskInfo[4];

uint32_t runCnt1 = 0;
uint32_t runCnt2 = 0;
uint32_t runCnt3 = 0;
uint32_t runCnt4 = 0;


void testTaskQeued(void* arg)
{
    taskInfo_t* locTaskInfo = (taskInfo_t*)arg;
    locTaskInfo->runCnt++;

    printf("task %d runs %d times\n\r", locTaskInfo->taskNo, locTaskInfo->runCnt);
    if(locTaskInfo->taskNo == 0)
    {
        gSequencer.queueTask(gTaskInfo[1].taskId);
        gTaskInfo[2].wait = true;
    }

    if(locTaskInfo->taskNo == 1)
        gSequencer.queueTask(gTaskInfo[0].taskId);    
}

void testTaskWait(void* arg)
{
    while(1)
    {
        taskInfo_t* locTaskInfo = (taskInfo_t*)arg;
        locTaskInfo->wait = false;
        locTaskInfo->runCnt++;

        printf("task %d runs %d times\n\r", locTaskInfo->taskNo, locTaskInfo->runCnt);
        if(locTaskInfo->taskNo == 2)
            gTaskInfo[3].wait = true;

        gSequencer.waitForEvent(&locTaskInfo->wait);
    }
}

extern uint32_t __StackTop;

int main()
{
    stdio_init_all();

    gSequencer.init(&__StackTop, PICO_STACK_SIZE);

    gTaskInfo[0].taskNo = 0;
    gSequencer.addTask(gTaskInfo[0].taskId, testTaskQeued, (void*) &gTaskInfo[0]);

    gTaskInfo[1].taskNo = 1;
    gSequencer.addTask(gTaskInfo[1].taskId, testTaskQeued, (void*) &gTaskInfo[1]);

    gTaskInfo[2].taskNo = 2;
    gSequencer.addTask(gTaskInfo[2].taskId, testTaskWait, (void*) &gTaskInfo[2]);

    gTaskInfo[3].taskNo = 3;
    gSequencer.addTask(gTaskInfo[3].taskId, testTaskWait, (void*) &gTaskInfo[3]);  

    gSequencer.queueTask(gTaskInfo[0].taskId);
    gSequencer.queueTask(gTaskInfo[2].taskId);
    gSequencer.queueTask(gTaskInfo[3].taskId);

    gSequencer.startIdle();
}