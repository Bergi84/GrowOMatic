#include "gm_terminal.h"

void gm_terminal::init(TUart* aUart, TSequencer *aSeq)
{
    mUart = aUart;
    mSeq = aSeq;

    mUart->config(112500, UART_PARITY_NONE);
    mUart->installRxCb(rxCb, this);

    mSeq->addTask(mTaskId, terminalTask, this);

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

    pObj->mSeq->queueTask(pObj->mTaskId);
}

void gm_terminal::terminalTask(void* aArg)
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
            if(0x40 >= aChar && 0x7E <= aChar)
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
        // text
        uint32_t inc = 0;
        while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
            inc++;

        if(mCursorPos + inc == TERMINAL_LINE_LENGTH - 1)
        {
            if(inc > 1)
                inc = inc - 2;
            else
                inc = 0;
        }

        while(inc != 0) 
        {
            mLineBuf[mAktLine][mCursorPos + inc + 1] = mLineBuf[mAktLine][mCursorPos + inc];
        };

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
    }
    else
    {
        // command codes
        switch(aChar)
        {
            case CTRLSYM_BS:
                if(mCursorPos != 0)
                {
                    mCpyLine = mAktLine;
                    uint32_t inc = 0;
                    while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
                    {
                        mLineBuf[mAktLine][mCursorPos + inc - 1] = mLineBuf[mAktLine][mCursorPos + inc];
                        inc++;
                    };
                    mLineBuf[mAktLine][mCursorPos + inc - 1] = mLineBuf[mAktLine][mCursorPos + inc];
                    mCursorPos--;
                    putChar(aChar);
                }
                break;

            case CTRLSYM_LF:
                putChar(CTRLSYM_LF);
                if(mLineBuf[mAktLine][0] = 0)
                {
                    newLine();
                }
                else
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
                break;

            case CTRLSYM_DEL:
                if(mLineBuf[mAktLine][mCursorPos] != 0)
                {
                    mCpyLine = mAktLine;
                    uint32_t inc = 0;
                    while(mLineBuf[mAktLine][mCursorPos + inc] != 0) 
                    {
                        mLineBuf[mAktLine][mCursorPos + inc] = mLineBuf[mAktLine][mCursorPos + inc + 1];
                        inc++;
                    };                    
                }
                break;

            case CTRLSYM_UP:
                if(mCpyLine == 0)
                    mCpyLine = TERMINAL_LINE_CNT - 1;
                else
                    mCpyLine++;

                clrLine();
                while(mLineBuf[mCpyLine][mCursorPos] != 0) 
                {
                    mLineBuf[mAktLine][mCursorPos] = mLineBuf[mCpyLine][mCursorPos];
                    putChar(mLineBuf[mCpyLine][mCursorPos]);
                    mCursorPos++;
                };  
                mLineBuf[mAktLine][mCursorPos] = mLineBuf[mCpyLine][mCursorPos];
                break;

            case CTRLSYM_DOWN:
                if(mCpyLine == TERMINAL_LINE_CNT - 1)
                    mCpyLine = 0;
                else
                    mCpyLine--;

                clrLine();
                while(mLineBuf[mCpyLine][mCursorPos] != 0) 
                {
                    mLineBuf[mAktLine][mCursorPos] = mLineBuf[mCpyLine][mCursorPos];
                    putChar(mLineBuf[mCpyLine][mCursorPos]);
                    mCursorPos++;
                };  
                mLineBuf[mAktLine][mCursorPos] = mLineBuf[mCpyLine][mCursorPos];
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

void gm_terminal::parseLine()
{
    // todo: implement

    newLine();
}

void gm_terminal::clrLine()
{
    // todo: implement
}

void gm_terminal::putChar(uint8_t aChar)
{
    // todo: implement
}

void gm_terminal::newLine()
{
    // todo: implement
}