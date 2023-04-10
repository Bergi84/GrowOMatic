#ifndef RP_UART_h_
#define RP_UART_h_

#include "uart.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "sequencer_armm0.h"
#include "irqVeneer.h"

class THwUart : public TUart
{
private:
    uart_inst_t *mUart;
    uint32_t mTxGpio;
    uint32_t mRxGpio;
    bool mTxCbEn;

    inline void enIrqCb(bool aEna)
    {
        mTxCbEn = aEna;
        uart_get_hw(mUart)->imsc = (bool_to_bit(aEna && mTxCb != 0) << UART_UARTIMSC_TXIM_LSB) |
                                    (bool_to_bit(mRxCb != 0) << UART_UARTIMSC_RXIM_LSB) |
                                    (bool_to_bit(mRxCb != 0) << UART_UARTIMSC_RTIM_LSB);
    }

    // interrupt handler
    static void irqHandler(void* aArg);
    void setIrqHandler(void(*aIrqHandler)());

    irqVeneer_t mIrqVeneer;

public:
    THwUart();
    virtual ~THwUart();

    void init(uart_inst_t *aUart, uint8_t aTxGpio, uint8_t aRxGpio);
    virtual void config(uint32_t aBaudRate, uartPar_t aParity);
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen);
    virtual void __time_critical_func(rxChar)(uint8_t *aC);
    virtual bool rxPending();
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk);
    virtual void __time_critical_func(txChar)(uint8_t aC);
    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen);
    virtual bool txFree();
    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk);
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk);
    virtual void disableTx(bool aDis);
    virtual void installRxCb(void (*pFunc)(void*), void* aArg);
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg);
    virtual void disableFifo(bool aDis);

    
};


class TPioUart : public TUart
{

};

class TUsbUart : public TUart
{
private: 
    static void tusbWorker(void* aArg);
    uint8_t mTaskIdWorker;
    TSequencer* mSeq;

    irq_handler_t mTusbIrq;
    bool mTxCbEn;

    static void irqHandler(void* aArg);
    void setIrqHandler(void (*pFunc)());

    irqVeneer_t mIrqVeneer;

public:
    TUsbUart();
    virtual ~TUsbUart();

    void init(TSequencer* aSeq);
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen);
    virtual void rxChar(uint8_t *aC);
    virtual bool rxPending();
    virtual void txChar(uint8_t aC);
    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen);
    virtual bool txFree();
    virtual void installRxCb(void (*pFunc)(void*), void* aArg);
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg); 

    // not applicable in case of usb uart
    virtual void config(uint32_t aBaudRate, uartPar_t aParity) {};
    virtual void disableFifo(bool aDis) {};
    virtual void disableTx(bool aDis) {};

    // not implemented
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk) {};
    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk) {};
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk) {};
};

#endif /* RP_UART_h_ */