#ifndef GM_BUS_H_
#define GM_BUS_H_

#include "gm_busMaster.h"
#include "gm_busSlave.h"
#include "paraTable.h"

class GM_bus : protected TEpBusDefs
{
private:
    GM_busSlave mSlave;
    GM_busMaster mMaster;

    TParaTable::paraRec_t mBusPara[cParaListLength];
    TParaTable::endpoint_t mBusEp;

    TUart* mUartList[GM_MAXUARTS];
    uint32_t mListLen;
    TSequencer* mSeq;
    TParaTable* mParaTable;

    static void setBusModeCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    GM_bus();

    // first 2 uarts of list used for slave
    void init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TParaTable* aParaTable);
};

#endif /* GM_BUS_H_ */ 