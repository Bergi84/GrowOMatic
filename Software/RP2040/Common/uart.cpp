#include "uart.h"

TUart::TUart()
{

};

THwUart::THwUart()
{

}

THwUart::~THwUart()
{

}

void THwUart::init(uart_inst_t *aUart, uint8_t aTxGpio, uint8_t aRxGpio)
{
    mUart = aUart;
    mTxGpio = aTxGpio;
    mRxGpio = aRxGpio;

    mTxCbEn = true;

    gpio_set_function(aTxGpio, GPIO_FUNC_UART);
    gpio_set_drive_strength(aTxGpio, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_slew_rate(aTxGpio, GPIO_SLEW_RATE_FAST);

    gpio_set_function(aRxGpio, GPIO_FUNC_UART);
    gpio_set_pulls (aRxGpio, true, false);
    gpio_set_input_hysteresis_enabled (aRxGpio, true);

    uart_set_baudrate(aUart, 115200);
    uart_set_format(aUart, 8, 1, UART_PARITY_NONE);

    uart_set_fifo_enabled(aUart, true);
} 

void THwUart::config(uint32_t aBaudRate, uart_parity_t aParity)
{
    uart_set_baudrate(mUart, aBaudRate);
    uart_set_format(mUart, 8, 1, aParity);
}

uint32_t THwUart::rxBlock(uint8_t* aBuf, uint32_t aMaxLen)
{
    uint32_t len = 0;

    while(len < aMaxLen and uart_is_readable(mUart))
        aBuf[len++] = uart_getc(mUart);

    return len;
}

uint32_t THwUart::txBlock(uint8_t* aBuf, uint32_t aLen)
{
    uint32_t len = 0;

    while(len < aLen and uart_is_writable(mUart))
        uart_putc(mUart, aBuf[len++]);

    enIrqCb(true);

    return len;
}

void THwUart::disableTx(bool aDis)
{
    if(aDis)
    {
        uart_get_hw(mUart)->cr &= ~UART_UARTCR_TXE_BITS;
        gpio_set_oeover(mTxGpio, GPIO_OVERRIDE_LOW);
        enIrqCb(false);
    }
    else
    {
        uart_get_hw(mUart)->cr |= UART_UARTCR_TXE_BITS;
        gpio_set_oeover(mTxGpio, GPIO_OVERRIDE_NORMAL);
        enIrqCb(true);
    }
}

void THwUart::installRxCb(void (*pFunc)(void*), void* aArg)
{
    mRxCb = pFunc;
    mRxCbArg = aArg;

    uart_set_irq_enables(mUart, mRxCb != 0, mTxCb != 0 and mTxCbEn);

}
void THwUart::installTxCb(bool (*pFunc)(void*), void* aArg)
{
    mTxCb = pFunc;
    mTxCbArg = aArg;

    uart_set_irq_enables(mUart, mRxCb != 0, mTxCb != 0 and mTxCbEn);
}

void THwUart::setIrqHandler(void(*aIrqHandler)())
{
    if(mUart == uart0)
    {       
        irq_set_exclusive_handler(UART0_IRQ, aIrqHandler);
        irq_set_enabled(UART0_IRQ, true);
    }
    else
    {
        irq_set_exclusive_handler(UART1_IRQ, aIrqHandler);
        irq_set_enabled(UART1_IRQ, true);
    }
}

void THwUart::disableFifo(bool aDis)
{
    uart_set_fifo_enabled(mUart, !aDis);
}


void THwUart::rxChar(uint8_t *aC)
{
    *aC = uart_getc(mUart);
}

bool THwUart::rxPending()
{
    return uart_is_readable(mUart);
}

void THwUart::rxGetWait(void* &aAdr, uint32_t &aMsk)
{
    aAdr = (void*) &uart_get_hw(mUart)->fr;
    aMsk = UART_UARTFR_RXFF_BITS;   // rx fifo full
}

void THwUart::txChar(uint8_t aC)
{
    uart_putc (mUart, aC);

    enIrqCb(true);
}

bool THwUart::txFree()
{
    return uart_is_writable(mUart);
}

void THwUart::txGetWaitFree(void* &aAdr, uint32_t &aMsk)
{
    aAdr = (void*) &uart_get_hw(mUart)->fr; 
    aMsk = UART_UARTFR_TXFE_BITS;   // TX fifo empty
}

void THwUart::txGetWaitIdle(void* &aAdr, uint32_t &aMsk)
{
    aAdr = (void*) &uart_get_hw(mUart)->fr; 
    aMsk = UART_UARTFR_BUSY_BITS;   // tx transmitting data
}

TUsbUart::TUsbUart()
{

}

TUsbUart::~TUsbUart()
{

}

void TUsbUart::tusbWorker(void* aArg)
{
    TUsbUart *pObj = (TUsbUart*) aArg;

    tud_task();
    tud_cdc_write_flush();
    if(tud_cdc_connected())
    {
        if(tud_cdc_write_available() && pObj->mTxCb != 0)
            pObj->mTxCb(pObj->mTxCbArg);
        if(tud_cdc_available() && pObj->mRxCb != 0)
            pObj->mRxCb(pObj->mRxCbArg);
    }
}

void TUsbUart::init(TSequencer *aSeq)
{
    mSeq = aSeq;
    mSeq->addTask(mTaskIdWorker, tusbWorker, this);

    tusb_init();
}

void TUsbUart::rxChar(uint8_t *aC)
{
    // wait until conected and one char is available
    while(!tud_cdc_connected() || tud_cdc_available() == 0)
    {
        if(tud_cdc_connected())
        {
            // avoiding dead lock
            tud_task();
        }
    }

    tud_cdc_read(aC, (uint32_t) 1);
}

bool TUsbUart::rxPending()
{
    return tud_cdc_connected() && tud_cdc_available();
}

void TUsbUart::txChar(uint8_t aC)
{
    // wait until byte can be written
    while(!tud_cdc_connected() || tud_cdc_write_available() == 0)
    {
        if(tud_cdc_connected())
        {   
            // avoiding dead lock
            tud_task();
            tud_cdc_write_flush();
        }
    }

    uint8_t lC = aC;
    tud_cdc_write(&lC, 1);
    mSeq->queueTask(mTaskIdWorker);
}

uint32_t TUsbUart::txBlock(uint8_t* aBuf, uint32_t aLen)
{
    if(!tud_cdc_connected())
        return 0;

    uint32_t n = tud_cdc_write_available();
    if(n > aLen)
        n = aLen;

    uint32_t written = tud_cdc_write(aBuf, n);
    mSeq->queueTask(mTaskIdWorker);
    return written;
}

uint32_t TUsbUart::rxBlock(uint8_t* aBuf, uint32_t aMaxLen)
{
    if(!tud_cdc_connected())
        return 0;

    uint32_t n = tud_cdc_available();
    if(n > aMaxLen)
        n = aMaxLen;

    return tud_cdc_read(aBuf, n);
}

bool TUsbUart::txFree()
{
    return tud_cdc_connected() && tud_cdc_write_available();
}

void TUsbUart::installRxCb(void (*pFunc)(void*), void* aArg)
{
    mRxCb = pFunc;
    mRxCbArg = aArg;
}

void TUsbUart::installTxCb(bool (*pFunc)(void*), void* aArg)
{
    mTxCb = pFunc;
    mTxCbArg = aArg;
}