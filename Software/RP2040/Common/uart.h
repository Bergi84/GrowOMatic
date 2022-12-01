#ifndef UART_H_
#define UART_H_

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

class TUart
{
protected:
    void (*mRxCb)(void*);
    void* mRxCbArg;

    bool (*mTxCb)(void*);
    void* mTxCbArg;   

public:
    // configure uart, 8 Data bits and 1 stop bit assumed as default
    virtual void config(uint32_t aBaudRate, uart_parity_t aParity){};

    // read all aviable bytes from fifo
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen){return 0;};

    // read one byte, blocks until one bate is aviable
    virtual void rxChar(uint8_t *aC){};
    virtual bool rxPending(){return false;};

    // returns address of flag register and mask for waiting
    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk){};

    // is called if data recieved, if fifo is enabled it is called
    // after reception of 4 byte or 32bit idle on line, also enables
    // RX interrupt
    virtual void installRxCb(void (*pFunc)(void*), void* aArg){};

    // writes a uint8_t and blocks until uint8_t has been sent
    virtual void txChar(uint8_t aC){};

    // writes data of aBuf to uart, returns written bytes
    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen){return 0;};

    virtual bool txFree(){return false;};

    virtual void txGetWaitFree(void* &aAdr, uint32_t &aMsk){};
    virtual void txGetWaitIdle(void* &aAdr, uint32_t &aMsk){};

    // disable TX and switch TX pin to tristate
    virtual void disableTx(bool aDis){};

    // is called when tx need data, also enables TX interrupt
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg){};

    // disable fifo
    virtual void disableFifo(bool aDis){};
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
    void init(uart_inst_t *aUart, uint8_t aTxGpio, uint8_t aRxGpio);
    virtual void config(uint32_t aBaudRate, uart_parity_t aParity);
    virtual uint32_t rxBlock(uint8_t* aBuf, uint32_t aMaxLen);
    virtual void rxChar(uint8_t *aC)
    {
        *aC = uart_getc(mUart);
    }

    virtual bool rxPending()
    {
        return uart_is_readable(mUart);
    }

    virtual void rxGetWait(void* &aAdr, uint32_t &aMsk)
    {
        aAdr = (void*) &uart_get_hw(mUart)->fr;
        aMsk = UART_UARTFR_RXFE_BITS;
    }

    virtual void txChar(uint8_t aC)
    {
        uart_putc (mUart, aC);

        enIrqCb(true);
    }

    virtual uint32_t txBlock(uint8_t* aBuf, uint32_t aLen);
    virtual bool txFree()
    {
        return uart_is_writable(mUart);
    }

    virtual void rxGetWaitFree(void* &aAdr, uint32_t &aMsk)
    {
        aAdr = (void*) &uart_get_hw(mUart)->fr; 
        aMsk = UART_UARTFR_TXFF_BITS;
    }

    virtual void rxGetWaitIdle(void* &aAdr, uint32_t &aMsk)
    {
        aAdr = (void*) &uart_get_hw(mUart)->fr; 
        aMsk = UART_UARTFR_BUSY_BITS;
    }

    virtual void disableTx(bool aDis);
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
    virtual void installRxCb(void (*pFunc)(void*), void* aArg);
    virtual void installTxCb(bool (*pFunc)(void*), void* aArg);
    virtual void disableFifo(bool aDis);
};


class TPioUart : public TUart
{

};

#endif // UART_H_