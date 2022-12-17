#include "gm_terminal.h"

void gm_terminal::init(TUart* aUart, TSequencer *aSeq)
{
    mUart = aUart;
    mSeq = aSeq;

    mUart->config(112500, UART_PARITY_NONE);
    mUart->installRxCb(rxCb, this);

    mSeq->addTask(mRxTaskId, termRxTask, this);
    mSeq->addTask(mTxTaskId, termTxTask, this);

    for(int i = 0; i < TERMINAL_LINE_CNT; i++)
    {
        mLineBuf[i][0] = 0;
    }

    mAktLine = 0;
    mCursorPos = 0;
    mEscAkt = false;

    mTxBufWInd = 0;
    mTxBufRInd = 0;
}

void gm_terminal::rxCb(void* aArg)
{
    gm_terminal *pObj = (gm_terminal*) aArg; 

    pObj->mSeq->queueTask(pObj->mRxTaskId);
}

void gm_terminal::txCb(void* aArg)
{
    gm_terminal *pObj = (gm_terminal*) aArg; 

    pObj->mSeq->queueTask(pObj->mTxTaskId);
}

void gm_terminal::termTxTask(void* aArg)
{
    gm_terminal *pObj = (gm_terminal*) aArg; 

    uint32_t locWInd = pObj->mTxBufWInd;
    uint32_t locRInd = pObj->mTxBufRInd;

    if(locWInd != locRInd)
    {
        if(locWInd > locRInd)
        {
            uint32_t txAvail = locWInd - locRInd;
            uint32_t txLen = pObj->mUart->txBlock(&pObj->mTxBuf[pObj->mTxBufRInd], txAvail);
            pObj->mTxBufRInd += txLen;
        }
        else
        {
            uint32_t txAvail = TERMINAL_TX_BUF_LEN - locRInd;
            uint32_t txLen = pObj->mUart->txBlock(&pObj->mTxBuf[pObj->mTxBufRInd], txAvail);
            if(txLen == txAvail)
            {         
                if(locWInd > 0)
                {
                    txLen = pObj->mUart->txBlock(&pObj->mTxBuf[0], locWInd);
                    pObj->mTxBufRInd = txLen;
                }
                else
                {
                    pObj->mTxBufRInd = 0;
                }
            }
            else
            {
                pObj->mTxBufRInd += txLen;
            }
        }
    }
}

void gm_terminal::termRxTask(void* aArg)
{
    gm_terminal *pObj = (gm_terminal*) aArg; 

    uint8_t buf[16];
    uint32_t readLen = pObj->mUart->rxBlock(buf, 16);
    uint32_t readCnt = 0;

    while(readLen)
    {
        if(pObj->mEscAkt)
        {
            buf[readCnt] = pObj->decodeEsc(buf[readCnt]);
            if(buf[readCnt])
            {
                pObj->mEscAkt = false;
            }
        }
        else
        {
            if(buf[readCnt] == CTRLSYM_ESC)
            {
                pObj->mEscAkt = true;
                pObj->mEscPos = 0;
                pObj->mEscdState = ESCD_ESC;
            }
        }

        if(!pObj->mEscAkt)
        {
            if(pObj->mAktApp)
            {
                pObj->mAktApp->parse(buf[readCnt]);
            }
            else
            {
                pObj->recordChar(buf[readCnt]);
            }
        }

        readCnt++;
        if(readCnt == readLen)
        {
            readLen = pObj->mUart->rxBlock(buf, 16);
            readCnt = 0;
        }
    }
}

uint8_t gm_terminal::decodeEsc(uint8_t aChar)
{
    mEscBuf[mEscPos] = aChar;
    mEscPos++;
    while(mEscPos == TERMINAL_MAX_ESC_LEN);

    switch(mEscdState)
    {
        case ESCD_ESC:
            switch(aChar)
            {
                case '[':
                    mEscdState = ESCD_CSI;
                    break;
                case ']':
                    mEscdState = ESCD_OSC;
                    break;
                default:
                    mEscdState = ESCD_UNKNOWEN; 
                    break;
            }
            break;

        case ESCD_CSI:
            if(0x40 <= aChar && 0x7E >= aChar)
            {
                switch(aChar)
                {
                    case 'A':
                        return CTRLSYM_UP;
                    case 'B':
                        return CTRLSYM_DOWN;
                    case 'C':
                        return CTRLSYM_RIGHT;
                    case 'D':
                        return CTRLSYM_LEFT;
                    default:
                        return CTRLSYM_UNKNOWEN; 
                }
            }
            break;

        case ESCD_OSC:
            if(aChar == '\\' && mEscBuf[mEscPos-2] == CTRLSYM_ESC)
            {

            }
            break;

        case ESCD_UNKNOWEN:
            break;
    }
    return 0;
}

void gm_terminal::recordChar(uint8_t aChar)
{
    if(aChar > 0x1f && aChar < 0x7f)
    {
        mCpyLine = mAktLine;

        uint32_t inc = 0;

        // find end of line
        while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
            inc++;

        if(mCursorPos + inc == TERMINAL_LINE_LENGTH - 1)
        {
            if(inc > 1)
                inc = inc - 2;
            else
                inc = 0;
        }

        // move all byte one index right until insertion point
        while(inc != 0) 
        {
            mLineBuf[mAktLine][mCursorPos + inc + 1] = mLineBuf[mAktLine][mCursorPos + inc];
            inc--;
        };
        mLineBuf[mAktLine][mCursorPos + 1] = mLineBuf[mAktLine][mCursorPos];

        // insert char
        if(mCursorPos == TERMINAL_LINE_LENGTH - 1)
        {
            mLineBuf[mAktLine][mCursorPos-1] = aChar;
            putChar(CTRLSYM_BS);
            putChar(aChar);
        }
        else
        {
            mLineBuf[mAktLine][mCursorPos] = aChar;
            putChar(aChar);
            mCursorPos++;
        }

        // print rest of line
        while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
        {
            putChar(mLineBuf[mAktLine][mCursorPos + inc]);
            inc++;
        }

        // move cursor back to insertion point
        if(inc)
        {
            moveCursor(CTRLSYM_LEFT, inc);
        }

    }
    else
    {
        // command codes
        switch(aChar)
        {
            case CTRLSYM_BS:
            case CTRLSYM_DEL:
                if(mCursorPos != 0)
                {
                    mCpyLine = mAktLine;
                    uint32_t inc = 0;
                    putChar(aChar);
                    clrLine();
                    while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
                    {
                        putChar(mLineBuf[mAktLine][mCursorPos + inc]);
                        mLineBuf[mAktLine][mCursorPos + inc - 1] = mLineBuf[mAktLine][mCursorPos + inc];
                        inc++;
                    };
                    if(inc)
                    {
                        moveCursor(CTRLSYM_LEFT, inc);
                    }
                    mLineBuf[mAktLine][mCursorPos + inc - 1] = mLineBuf[mAktLine][mCursorPos + inc];
                    mCursorPos--;
                }
                break;

            case CTRLSYM_CR:
                putChar(CTRLSYM_CR);
                putChar(CTRLSYM_LF);
                
                if(mLineBuf[mAktLine][0] != 0)
                {
                    parseLine();
                    if(mAktLine == TERMINAL_LINE_CNT - 1)
                        mAktLine = 0;
                    else
                        mAktLine++;
                    
                    mCpyLine = mAktLine;
                    mCursorPos = 0;
                    mLineBuf[mAktLine][mCursorPos] = 0;
                }
                newLine();
                break;

            case CTRLSYM_UP:
                {
                    uint32_t locCpyLine = mCpyLine;

                    if(locCpyLine == 0)
                        locCpyLine = TERMINAL_LINE_CNT - 1;
                    else
                        locCpyLine--;

                    if(mLineBuf[locCpyLine][0] != 0 && locCpyLine != mAktLine)
                    {
                        if(mCursorPos)
                        {
                            moveCursor(CTRLSYM_LEFT, mCursorPos);
                            mCursorPos = 0;
                            clrLine();
                        }

                        while(mLineBuf[locCpyLine][mCursorPos] != 0) 
                        {
                            mLineBuf[mAktLine][mCursorPos] = mLineBuf[locCpyLine][mCursorPos];
                            putChar(mLineBuf[locCpyLine][mCursorPos]);
                            mCursorPos++;
                        };
                        mLineBuf[mAktLine][mCursorPos] = mLineBuf[locCpyLine][mCursorPos];
                        mCpyLine = locCpyLine;
                    }
                }
                break;

            case CTRLSYM_DOWN:
                {
                    uint32_t locCpyLine = mCpyLine;

                    if(locCpyLine == TERMINAL_LINE_CNT - 1)
                        locCpyLine = 0;
                    else
                        locCpyLine++;

                    if(mLineBuf[locCpyLine][0] != 0 && mCpyLine != mAktLine)
                    {
                        if(mCursorPos)
                        {
                            moveCursor(CTRLSYM_LEFT, mCursorPos);
                            mCursorPos = 0;
                            clrLine();
                        }

                        while(mLineBuf[locCpyLine][mCursorPos] != 0) 
                        {
                            mLineBuf[mAktLine][mCursorPos] = mLineBuf[locCpyLine][mCursorPos];
                            putChar(mLineBuf[locCpyLine][mCursorPos]);
                            mCursorPos++;
                        };
                        mLineBuf[mAktLine][mCursorPos] = mLineBuf[locCpyLine][mCursorPos];
                        mCpyLine = locCpyLine;
                    }
                }
                break;

            case CTRLSYM_LEFT:
                if(mCursorPos != 0)
                {
                    mCursorPos--;
                    putChar(CTRLSYM_LEFT);
                }
                break;

            case CTRLSYM_RIGHT:
                if(mLineBuf[mAktLine][mCursorPos] != 0)
                {
                    mCursorPos++;
                    putChar(CTRLSYM_RIGHT);                   
                }
                break;

            default:
                break;
        }
    }
}

void gm_terminal::moveCursor(ctrlSym_e aDir, uint32_t aDist)
{
    if(aDist > 99)
        return;

    uint8_t buf[8];
    uint32_t len;

    switch(aDir)
    {
        default:
            return;
        case CTRLSYM_UP:
        case CTRLSYM_DOWN:
        case CTRLSYM_LEFT:
        case CTRLSYM_RIGHT:
            buf[0] = '\033';
            buf[1] = '[';

            if(aDist > 10)
            {
                buf[2] = '0' + aDist/10;
                buf[3] = '0' + aDist%10;
                len = 5;
            }
            else
            {
                buf[2] = '0' + aDist;
                len = 4;
            }
            break;
    }


    switch(aDir)
    {
        default:
            break;
        case CTRLSYM_UP:
            buf[len - 1] = 'A';
            break;
        case CTRLSYM_DOWN:
            buf[len - 1] = 'B';
            break;
        case CTRLSYM_LEFT:
            buf[len - 1] = 'D';
            break;
        case CTRLSYM_RIGHT:
            buf[len - 1] = 'C';
            break;
    }

    putString((const char*)buf, len);
}

void gm_terminal::parseLine()
{
    // todo: implement

    newLine();
}

void gm_terminal::clrLine()
{
    putString("\033[K",3);
}

void gm_terminal::putString(const char *aStr, uint32_t len)
{
    uint32_t freeByte;
    if(mTxBufWInd > mTxBufRInd)
        freeByte = TERMINAL_TX_BUF_LEN + mTxBufRInd - 1 - mTxBufWInd;
    else
        freeByte = mTxBufRInd - 1 - mTxBufWInd;

    if(freeByte < len)
        return;

    uint32_t ind = 0;
    while(aStr[ind] != 0 && ind < len)
    {
        putChar(aStr[ind]);
        ind++;
    }
}

void gm_terminal::putChar(uint8_t aChar)
{
    if(aChar < CTRLSYM_UNKNOWEN)
    {
        uint32_t wLocInd = mTxBufWInd;
        uint32_t wLocIncInd;
        
        if(wLocInd == TERMINAL_TX_BUF_LEN - 1)
            wLocIncInd = 0;
        else
            wLocIncInd = wLocInd + 1;
        
        if(wLocIncInd != mTxBufRInd)
        {
            mTxBuf[wLocInd] = aChar;
            mTxBufWInd = wLocIncInd;
        }
    }
    else
    {
        switch(aChar)
        {
            default:
                break;
            case CTRLSYM_UP:
                putString("\033[A",3);
                break;
            case CTRLSYM_DOWN:
                putString("\033[B",3);
                break;
            case CTRLSYM_LEFT:
                putString("\033[D",3);
                break;
            case CTRLSYM_RIGHT:
                putString("\033[C",3);
                break;
            case CTRLSYM_EL:
                putString("\033[K",3);
                break;
        }
    }

    mSeq->queueTask(mTxTaskId);
}

void gm_terminal::newLine()
{
    // todo: implement
}