/*
 * sequencer_armm.cpp
 *
 *  Created on: 14.04.2022
 *      Author: Klaus
 */

#include "sequencer_armm0.h"
#include "hardware/sync.h"

bool TSequencer::init(void* aStackBase, uint32_t aStackSize)
{
  stackBaseAdr = (uint8_t*) aStackBase;
  stackSize = aStackSize/SEQ_MAXSTACKS;

  for(int i = 0; i < arrayLen; i++)
  {
    usedId[i] = 0;
  }

  for(int i = 0; i < SEQ_MAXSTACKS; i++)
  {
    stacks[i].sp = 0;
    stacks[i].event = 0;
    stacks[i].id = invalidId;
  }

  schedLastStackInd = 0;
  schedLastTaskId = 0;
  highestTaskId = 0;

  idleCb.pFunc = 0;

  return true;
}

bool TSequencer::addTask(uint8_t &aSeqID, void (*aPFunc)(void*) , void* aPArg)
{
  for(int i = 0; i < arrayLen; i++)
  {
    for(int j = 0; j < 32; j++)
    {
      uint32_t mask = (1U << j);
      if((usedId[i] & mask) == 0)
      {
        uint8_t tmpID = i*32 + j;
        if(tmpID >= SEQ_MAXTASKS)
        {
          aSeqID = invalidId;
          return false;
        }
        else
        {
          usedId[i] |= mask;
          aktivTask[i] &= ~mask;
          queuedTask[i] &= ~mask;
          tasks[tmpID].pFunc = aPFunc;
          tasks[tmpID].pArg = aPArg;
          aSeqID = tmpID;
          if(highestTaskId < tmpID)
            highestTaskId = tmpID;

          return true;
        }
      }
    }
  }
  return false;
}

bool TSequencer::delTask(uint8_t aSeqID)
{
  uint32_t i = aSeqID >> 5;
  uint32_t j = aSeqID & 0x1F;

  usedId[i] &= ~(1U << j);

  uint8_t highestId = 0;

  for(int i = 0; i < arrayLen; i++)
  {
    if(usedId[i] != 0)
    {
      // cortex M0 do not support __CLZ so we need an emulation
      // for __CLZ(usedId[i])
      uint32_t clzBuf = usedId[i];
      uint32_t clzCnt = 0;
      for(; clzCnt < 32 and !(clzBuf & 0x80000000); clzCnt++)
      {
        clzCnt++;
        clzBuf <<= 1;
      }

      highestId = (31 - clzCnt) + i*32;
    }
  }

  highestTaskId = highestId;

  return true;
}

bool TSequencer::killTask(uint8_t aSeqID)
{
  for(int i = 0; i < SEQ_MAXTASKS; i++)
  {
    if(stacks[i].id == aSeqID)
    {
      stacks[i].id = 0;
      stacks[i].event = 0;
      if(i == aktivStackInd)
      {
        scheduler();
      }
      return true;
    }
  }
  return false;
}

bool TSequencer::queueTask(uint8_t aSeqID)
{
  uint32_t i = aSeqID >> 5;
  uint32_t j = aSeqID & 0x1F;
  uint32_t mask = 1U << j;

  if(usedId[i] & mask)
  {
    uint32_t status = save_and_disable_interrupts();

    queuedTask[i] |= mask;

    restore_interrupts(status);

    return true;
  }

  return false;
}

void TSequencer::waitForEvent(bool* aEvt)
{
  if(aEvt == 0)
    return;

  stackRec_t* stack = &stacks[aktivStackInd];
  // bool can a 8Bit value so we must avoid 
  // unaligend access
  stack->event = (uint32_t*)(((uint32_t)aEvt) & 0xFFFFFFFC);          // alligned address
  stack->msk = 0x000000FF << ((((uint32_t)aEvt) & 0x00000003)*8);     // mask

  switchTask(&stack->sp, true);
}

void TSequencer::waitForEvent(uint32_t* aEvt, uint32_t msk)
{
  if(aEvt == 0)
    return;

  stackRec_t* stack = &stacks[aktivStackInd];
  stack->event = aEvt;
  stack->msk = msk;     // mask
  
  switchTask(&stack->sp, true);
}

uint32_t TSequencer::getAktivTask()
{
  return stacks[aktivStackInd].id;
}

bool TSequencer::setIdleFunc(void (*aPFunc)(void*), void* aPArg)
{
  idleCb.pFunc = aPFunc;
  idleCb.pArg = aPArg;

  return true;
}

inline void TSequencer::startTask(uint8_t stackInd, uint8_t taskInd)
{
  void *sp = (void*)(((uint32_t)stackBaseAdr) - stackSize * stackInd);
  uint32_t clobber;

  // switch to new stack
  // todo: add a guard for stack overflow detection ad the bottom of each stack
  __asm volatile
  (
    // load new stack pointer
    "cpsid i                        \n"
    "MSR MSP, %1                    \n"
    "isb                            \n"
    "cpsie i                        \n"

    : "+&r" (clobber) // no output parameter
    : "r" (sp)
    : "memory"
  );

  aktivStackInd = stackInd;
  stackRec_t* stack = &stacks[stackInd];

  // store stack context
  stack->sp = stacks[stackInd].sp;
  stack->event = 0;
  stack->id = taskInd;

  // set task as active
  uint32_t i = taskInd >> 5;
  uint32_t j = taskInd & 0x1F;
  uint32_t mask = 1U << j;

  aktivTask[i] |= mask;

  // can be modified from interrupt context
  // and is not a atomic instruction
  uint32_t status = save_and_disable_interrupts();

  queuedTask[i] &= ~mask;

  restore_interrupts(status);

  taskCbRec_t* task = &tasks[taskInd];

  // call task
  task->pFunc(task->pArg);

  // set task as inactive
  aktivTask[i] &= ~mask;
  stack->sp = 0;
  stack->id = invalidId;

  // return is not allowed because the stack is not valid anymore
  // so we call the scheduler instead

  scheduler();
}

/*
// stores all register to stack and return stack pointer
// if this functions is called the compiler believes this function
// will never return and ignores all following functions
void TSequencer::storeTask(void **sp)
{
  __asm volatile
  (
    // store core registers
    "push {r0-r7}                     \n"
    "MRS r2, CONTROL                  \n"
    "mov r3, r14                      \n"
    "mov r4, r8                       \n"
    "mov r5, r9                       \n"
    "mov r6, r10                      \n"
    "mov r7, r11                      \n"
    "push {r2-r7}                     \n"

    // get stack pointer
    "mrs r4, MSP                      \n"
    "str r4, [r1]                     \n"
  );
  scheduler();
}


void TSequencer::restoreTask(void **sp)
{
  __asm volatile
  (
    // set stack pointer
    "cpsid i                          \n"
    "ldr r4, [r1]                     \n"
    "msr msp, r4                      \n"
    "isb                              \n"
    "cpsie i                          \n"

    // load core registers
    "pop {r2-r7}                      \n"
    "MSR CONTROL, r2                  \n" // restore CONTROL to r1
    "mov r14, r3                      \n"
    "mov r8, r4                       \n"
    "mov r9, r5                       \n"
    "mov r10, r6                      \n"
    "mov r11, r7                      \n"
    "pop {r0-r7}                      \n"

    "BX r14                           \n"
  );
}
*/

// this are copies of store Task and restore task becouse
// return from naked function dont work
// according arm ABI r0 hold object pointer
void TSequencer::switchTask(void **sp, bool pause)
{
  if(pause)
  {
    // after this asm function register r2 - r7 are invalid
    // scheduler call only needs the object pointer in r0 so
    // it should not a problem
    __asm volatile
    (
      // store core registers
      "push {r0-r7}                     \n"
      "MRS r2, CONTROL                  \n"
      "mov r3, r14                      \n"
      "mov r4, r8                       \n"
      "mov r5, r9                       \n"
      "mov r6, r10                      \n"
      "mov r7, r11                      \n"
      "push {r2-r7}                     \n"

      // get stack pointer
      "mrs r4, MSP                      \n"
      "str r4, [r1]                     \n"
    );
    scheduler();
  }
  else
  {
    __asm volatile
    (
      // set stack pointer
      "cpsid i                          \n"
      "ldr r4, [r1]                     \n"
      "msr msp, r4                      \n"
      "isb                              \n"
      "cpsie i                          \n"

      // load core registers
      "pop {r2-r7}                      \n"
      "MSR CONTROL, r2                  \n" // restore CONTROL to r1
      "mov r14, r3                      \n"
      "mov r8, r4                       \n"
      "mov r9, r5                       \n"
      "mov r10, r6                      \n"
      "mov r11, r7                      \n"
      "pop {r0-r7}                      \n"
    );
  }
}

void TSequencer::scheduler()
{
  while(1)
  {
    // check waiting tasks and search free stack for new task
    // waiting tasks have always a higher priority
    uint8_t freeInd = invalidId;
    uint8_t endInd = schedLastStackInd;
    uint8_t tmpId = endInd;

    do
    {
      tmpId = (tmpId == SEQ_MAXSTACKS - 1) ? 0 : tmpId + 1;

      if(stacks[tmpId].sp != 0)
      {
        if(stacks[tmpId].event != 0 && (*stacks[tmpId].event & stacks[tmpId].msk))
        {
          // todo: check stack integrity
          stacks[tmpId].event = 0;
          schedLastStackInd = tmpId;
          aktivStackInd = tmpId;
          switchTask(&stacks[tmpId].sp, false); // this function never returns
        }
      }
      else
      {
        freeInd = tmpId;
      }
    }
    while(tmpId != endInd);

    // if a free stack is available
    // search queued task for execution
    if(freeInd != invalidId)
    {
      endInd = schedLastTaskId;
      tmpId = endInd;

      do
      {
        tmpId = (tmpId == highestTaskId) ? 0 : tmpId + 1;

        uint32_t i = tmpId >> 5;
        uint32_t j = tmpId & 0x1F;
        uint32_t mask = 1U << j;

        if(usedId[i] & queuedTask[i] & ~aktivTask[i] & mask)
        {
          schedLastTaskId = tmpId;
          startTask(freeInd, tmpId);  // this function never return
        }

      }
      while (endInd != tmpId);
    }

    // all stacks used or nothing todo, goto low power mode
    if(idleCb.pFunc != 0)
    {
      idleCb.pFunc(idleCb.pArg);
    }
  }
}
