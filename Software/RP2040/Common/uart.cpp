#include "uart.h"

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