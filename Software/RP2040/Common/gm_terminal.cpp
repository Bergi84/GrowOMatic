#include "gm_terminal.h"

void gm_terminal::init(TUart* aUart)
{
    mUart = aUart;

    mUart->config(112500, UART_PARITY_NONE);
    mUart->installRxCb(rxCb, this);
}

void gm_terminal::rxCb(void* aArg)
{
    // todo: implement
}