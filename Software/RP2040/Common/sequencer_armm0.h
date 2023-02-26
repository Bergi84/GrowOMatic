/*
 * sequencer_armm.h
 *
 *  Created on: 14.04.2022
 *      Author: Klaus
 */

#include "pico/stdlib.h"
#include "hardware/sync.h"

#ifndef SEQUENCER_ARMM0_H_
#define SEQUENCER_ARMM0_H_

#ifndef SEQ_MAXTASKS
  #define SEQ_MAXTASKS 32
#endif

#ifndef SEQ_MAXSTACKS
  #define SEQ_MAXSTACKS 4
#endif


class TSequencer
{
private:
  typedef struct
  {
    void* sp;
    uint32_t* event;
    uint32_t msk;
    uint8_t id;
  }
  stackRec_t;

  uint8_t* mStackBaseAdr;
  uint32_t mStackSize;
  stackRec_t mStacks[SEQ_MAXSTACKS];
  volatile uint8_t mAktivStackInd;

  typedef struct
  {
    void (*pFunc)(void*);
    void* pArg;
  }
  taskCbRec_t;

  taskCbRec_t tasks[SEQ_MAXTASKS];
  taskCbRec_t mIdleCb;

  static constexpr uint8_t CInvalidId = -1;
  static constexpr uint32_t CArrayLen = ((SEQ_MAXTASKS - 1)/32 + 1);
  uint32_t mUsedId[CArrayLen];
  uint32_t mAktivTask[CArrayLen];
  uint32_t mQueuedTask[CArrayLen];
  spin_lock_t *mSpinLock;

  uint8_t mSschedLastStackInd;
  uint8_t mSchedLastTaskId;
  uint8_t mHighestTaskId;

public:
  bool init(void* aStackBase, uint32_t aStackSize);

  // only queue Task function is allowed to call from interrupt kontext
  // all other functions must called from idle Task
  bool addTask(uint8_t &aSeqID, void (*aPFunc)(void*), void* aPArg);
  uint32_t getAktivTask();
  bool delTask(uint8_t aSeqID);
  bool killTask(uint8_t aSeqID);
  bool queueTask(uint8_t aSeqID);

  // puase calling Task until given bool becomes true
  void waitForEvent(bool* aEvt);
  void waitForEvent(uint32_t* aEvt, uint32_t msk = 0xFFFFFFFF);

  bool setIdleFunc(void (*aPFunc)(void*), void* aPArg);

  // this function starts the idle loop and never returns
  void startIdle() { scheduler(); };

private:
  void startTask(uint8_t stackInd, uint8_t taskInd);
  void switchTask(void **sp, bool pause);
  void scheduler();

/*
  __attribute__((naked)) void storeTask(void **sp);
  __attribute__((naked)) void restoreTask(void **sp);
*/
};



#endif /* SEQUENCER_ARMM0_H_ */
