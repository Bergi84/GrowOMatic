#ifndef UART_H_
#define UART_H_

#include "stdint.h"

typedef enum {
    UP_NONE,
    UP_EVEN,
    UP_ODD
} uartPar_t;

class TUart
{
protected:
    TUart();

    void (*mRxCb)(void*);
    void* mRxCbArg;

    bool (*mTxCb)(void*);
    void* mTxCbArg;   

public:
    // configure uart, 8 Data bits and 1 stop bit assumed as default
    virtual void config(uint32_t aBaudRate, uartPar_t aParity) = 0;

    // read all aviable bytes from fifo
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen) = 0;

    // read one byte, blocks until one byte is available
    virtual void rxChar(uint8_t *aC) = 0;
    virtual bool rxPending() = 0;

    // returns address of flag register and mask for waiting
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk) = 0;

    // is called if data recieved, if fifo is enabled it is called
    // after reception of 4 byte or 32bit idle on line, also enables
    // RX interrupt
    virtual void installRxCb(void (*pFunc)(void*), void* aArg) = 0;

    // writes a uint8_t and blocks until uint8_t has been sent
    virtual void txChar(uint8_t aC) = 0;

    // writes data of aBuf to uart, returns written bytes
    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen) = 0;

    virtual bool txFree() = 0;

    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk) = 0;
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk) = 0;

    // disable TX and switch TX pin to tristate
    virtual void disableTx(bool aDis) = 0;

    // is called when tx need data, also enables TX interrupt
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg) = 0;

    // disable fifo
    virtual void disableFifo(bool aDis) = 0;
};

#endif // UART_H_