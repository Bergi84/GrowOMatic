#include "gm_busSlave.h"

GM_busSlave::GM_busSlave()
{
    mState = S_IDLE;
}

void GM_busSlave::init(TUart *aUart0, TUart *aUart1)
{
    crcInitTab();

    byteTimeUs = (9999999 + mBaudRate)/mBaudRate;
    mCom[0].errCnt = 0;
    mCom[1].errCnt = 0;
    mCom[0].byteCnt = 0;
    mCom[1].byteCnt = 0;
    mCom[0].sec = false;
    mCom[1].sec = false;
    mCom[0].otherCom = &mCom[1];
    mCom[1].otherCom = &mCom[0];
    mCom[0].uart = aUart0;
    mCom[1].uart = aUart1;

    mCom[0].uart->config(mBaudRate, UART_PARITY_NONE);
    mCom[1].uart->config(mBaudRate, UART_PARITY_NONE);
    mCom[0].uart->disableFifo(true);
    mCom[1].uart->disableFifo(true);
    mCom[0].uart->disableTx(true);
    mCom[1].uart->disableTx(true);
    mCom[0].uart->installRxCb(rx0CbWrapper, (void*)this);
    mCom[1].uart->installRxCb(rx1CbWrapper, (void*)this);
};


void GM_busSlave::setParaCb(void (*aParaWCb)(void*, uint16_t, uint32_t), bool (*aParaRCb)(void* , uint16_t , uint32_t &), void* aParaCbArg)
{
    mParaWCb = aParaWCb;
    mParaRCb = aParaRCb;
    mParaCbArg = aParaCbArg;
}    

void GM_busSlave::rxCb(com_t* aCom)
{
    uint8_t byte;
    aCom->uart->rxChar(&byte);

    if(!aCom->sec)
    {
        if(mState == S_IDLE)
        {
            // both ports are in read mode, if we find a start deliminater
            // we can set the direction an start relaying
            if(byte == mSdW || byte == mSdR)
            {
                mState = S_DEVADR;
                mWrite = (byte == mSdW);
                aCom->byteCnt = 1;
                aCom->otherCom->sec = true;
                aCom->otherCom->uart->disableTx(false);
                aCom->otherCom->uart->txChar(byte);
                mCrc.dw = crcCalc(mCrcInitVal, byte);
                // todo: start byte time out
            }
            else
            {
                aCom->errCnt++;
            }
        }
        else
        {
            aCom->byteCnt++;

            switch (mState)
            {
            case S_DEVADR:
                mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
                if(byte == 0)
                {
                    // request belongs to this device so we must decode the hole massage
                    // we send a the invalid Addr to the next device to signal abort
                    // todo: start byte time out
                    mState = S_REGADR;
                    aCom->otherCom->uart->txChar(mInvalidAdr);
                }
                else
                {
                    // the message belongs to a other device
                    if(byte == mInvalidAdr)
                    {
                        // we got the invalid Adr which means the message belongs to a device before this
                        // and we can abort the relaying
                        // todo: stop time out
                        mState = S_IDLE;
                        aCom->otherCom->uart->txChar(mInvalidAdr);
                    }
                    else
                    {
                        // we got an adr which belongs to a device beyond this
                        // todo: calc turnAroundTimeout 
                        mState = S_FWD;
                        aCom->otherCom->uart->txChar(byte - 1);
                    }
                }
                break;

            case S_REGADR:
                mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
                if(aCom->byteCnt == 4)
                {
                    mState = S_DATA;
                    mRegAdr.b[1] = byte; 

                    if(mWrite)
                    {
                        // todo: start byte time out
                    }
                    else
                    {
                        // in read case we must turnaround the direction
                        mInvalidRegAdr = mParaRCb(mParaCbArg, mRegAdr.w, mData.dw);
                        aCom->uart->disableTx(false);
                        // the first byte is directly loaded from send register to transmit engine
                        // so we can push two bytes consecutivly into the send register
                        aCom->uart->txChar(mData.b[0]);
                        mCrcCalc.dw = crcCalc(mCrcCalc.dw, mData.b[0]);
                        aCom->uart->txChar(mData.b[1]);
                        mCrcCalc.dw = crcCalc(mCrcCalc.dw, mData.b[1]);
                    }                    
                }
                else
                {
                    // todo: start byte time out
                    mRegAdr.b[0] = byte;  
                }
                break;

            case S_DATA:
                if(aCom->byteCnt == 8)
                {
                    mState = S_CRC;
                }

                if(mWrite)
                {
                    mData.b[aCom->byteCnt - 5] = byte;
                    mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
                } 
                else
                {
                    if(aCom->byteCnt < 7)
                    {
                        // send byte 3 & 4 
                        aCom->uart->txChar(mData.b[aCom->byteCnt - 3]);
                        mCrcCalc.dw = crcCalc(mCrcCalc.dw, mData.b[aCom->byteCnt - 3]);
                    }
                    else
                    {
                        // send byte 0 & 1
                        // if the register address is invalid send the inverted checksum
                        if(mInvalidRegAdr)
                            aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 7]);
                        else
                            aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 7]);
                    }
                }
                break;

            case S_CRC:
                if(mWrite)
                {
                    mCrc.b[aCom->byteCnt - 9] = byte;
                    if(aCom->byteCnt == 12)
                    {
                        mState = S_IDLE;
                        if(mCrc.dw == mCrcCalc.dw)
                        {
                            mParaRCb(mParaCbArg, mRegAdr.w, mData.dw);
                        }
                    }
                } 
                else
                {
                    if(aCom->byteCnt == 12)
                    {
                        mState = S_IDLE;
                        aCom->uart->disableTx(true);
                    }

                    if(aCom->byteCnt < 11)
                    {
                        // send byte 3 & 4
                        // if the register address is invalid send the inverted checksum
                        if(mInvalidRegAdr)
                            aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 7]);
                        else
                            aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 7]);
                    } 
                }
                break;

            case S_FWD:
                if(mWrite)
                {
                    aCom->otherCom->uart->txChar(byte);
                    if(aCom->byteCnt == 12)
                    {
                        mState = S_IDLE;
                    }
                    else
                    {
                        // todo: start byte timout
                    }
                }
                else
                {
                    if(aCom->byteCnt == 12)
                    {
                        mState = S_IDLE;
                        aCom->uart->disableTx(true);
                        // todo: disable timout
                    }
                    else
                    {
                        // forword data until turnaround
                        if(aCom->byteCnt < 4)
                        {
                            aCom->otherCom->uart->txChar(byte);
                            // todo: start byte timout
                        }

                        if(aCom->byteCnt == 4)
                        {
                            aCom->otherCom->uart->txChar(byte);
                            aCom->uart->disableTx(false);
                            // todo: start turnaround timeout from here
                        }

                        if(aCom->byteCnt > 4)
                        {
                            // todo: start byte timout
                        }
                    }
                }
                break;
            
            default:
                aCom->errCnt++;
                break;
            }
        }
    }
    else
    {
        // this uart is currently the scondary uart
        aCom->byteCnt++;

        if(aCom->byteCnt == 2 && byte == mInvalidAdr)
        {
            // massage abort detected, this means the recieved massage was for an device before this
            aCom->uart->disableTx(true);
            aCom->byteCnt= 0;
        }

        if(aCom->byteCnt == 4 && !mWrite)
        {
            // turn around for read request needed
            aCom->uart->disableTx(true);
        }

        if(aCom->byteCnt > 4 && !mWrite)
        {
            // forwarding data from readrequest to primary uart
            aCom->otherCom->uart->txChar(byte);
        }

        if(aCom->byteCnt == 12)
        {
            // end of message
            aCom->byteCnt = 0;

            if(mWrite)
            {
                // before we disable the direction we to check if there is already the next
                // transfare is running
                if(mState == S_IDLE)
                {
                    aCom->uart->disableTx(true);
                    aCom->sec = false;
                };
            } 
            else
            {
                aCom->otherCom->uart->txChar(byte);
                aCom->sec = false;
            }
        }
    }  
}


void GM_busSlave::rx0CbWrapper(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->rxCb(&pObj->mCom[0]);
}

void GM_busSlave::rx1CbWrapper(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->rxCb(&pObj->mCom[1]);
}

void GM_busSlave::nextByteTimeOutCb(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    if(pObj->mCom[0].sec)
    {
        pObj->mCom[1].errCnt++;
    }

    if(pObj->mCom[1].sec)
    {
        pObj->mCom[0].errCnt++;
    }

    pObj->mCom[0].sec = false;
    pObj->mCom[1].sec = false;

    pObj->mState = S_IDLE;
}

uint32_t GM_busSlave::crcCalc(uint32_t aCrc, uint8_t aByte)
{
    return (aCrc >> 8) ^ crcTab[((uint8_t)aCrc) ^ aByte];
}

void GM_busSlave::crcInitTab()
{
	for (uint32_t i = 0; i < 256; i++) {

		uint32_t crc = i;

		for (uint32_t j = 0; j < 8; j++) {

			if ( crc & 0x00000001 ) crc = ( crc >> 1 ) ^ mCrcPoly;
			else                     crc =   crc >> 1;
		}

		crcTab[i] = crc;
	}
}