#ifndef UART_H_
#define UART_H_

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "sequencer_armm0.h"

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
    virtual void config(uint32_t aBaudRate, uart_parity_t aParity) = 0;

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

public:
    THwUart();
    virtual ~THwUart();

    void init(uart_inst_t *aUart, uint8_t aTxGpio, uint8_t aRxGpio);
    virtual void config(uint32_t aBaudRate, uart_parity_t aParity);
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen);
    virtual void rxChar(uint8_t *aC);
    virtual bool rxPending();
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk);
    virtual void txChar(uint8_t aC);
    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen);
    virtual bool txFree();
    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk);
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk);
    virtual void disableTx(bool aDis);
    virtual void installRxCb(void (*pFunc)(void*), void* aArg);
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg);
    virtual void disableFifo(bool aDis);

    // interrupt handler
    inline void irqHandler()
    {
        if( uart_get_hw(mUart)->imsc & UART_UARTIMSC_TXIM_BITS && 
            uart_get_hw(mUart)->ris & UART_UARTRIS_TXRIS_BITS)
        {
            // debug trap, this should never occure:
            if(!mTxCbEn)
                while(1);

            if(!mTxCb(mTxCbArg))
            {
                enIrqCb(false);
            }
        }

        if( (uart_get_hw(mUart)->imsc & UART_UARTIMSC_RXIM_BITS  && 
            uart_get_hw(mUart)->ris & UART_UARTRIS_RXRIS_BITS) ||
            (uart_get_hw(mUart)->imsc & UART_UARTIMSC_RTIM_BITS  && 
            uart_get_hw(mUart)->ris & UART_UARTRIS_RTRIS_BITS))
        {
            mRxCb(mRxCbArg);
        }
    }

    // this function installs the irq handler and only accepts "C" functions
    // or static memeber function. So we must provide an IRQ Handler wrapper function
    // for our uart->irqHandler member function
    void setIrqHandler(void(*aIrqHandler)());
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
    virtual void config(uint32_t aBaudRate, uart_parity_t aParity) {};
    virtual void disableFifo(bool aDis) {};
    virtual void disableTx(bool aDis) {};

    // not implemented
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk) {};
    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk) {};
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk) {};

    // must called inside the the usb irq Handler
    inline void irqHandler()
    {
        mTusbIrq();
        mSeq->queueTask(mTaskIdWorker);
    }

    // this function installs the irq handler and only accepts "C" functions
    // or static memeber function. So we must provide an IRQ Handler wrapper function
    // for our uart->irqHandler member function
    void setIrqHandler(void (*pFunc)())
    {
        // tinyUSB has already installed an USB handler so we override this handler with ower own
        // and call the tiny USB handler insider ower handler
        mTusbIrq = irq_get_exclusive_handler(USBCTRL_IRQ);
        irq_remove_handler(USBCTRL_IRQ, mTusbIrq);
        irq_set_exclusive_handler(USBCTRL_IRQ, pFunc);
        irq_set_enabled (USBCTRL_IRQ, true);
    }
};

#endif // UART_H_