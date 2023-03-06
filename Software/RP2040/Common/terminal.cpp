#include "terminal.h"
#include "termApp.h"
#include "termPathMng.h"
#include <string.h>

#include "termAppLs.h"
#include "termAppCd.h"

TTermAppCd gAppCd;
TTermAppLs gAppLs;

void TTerminal::init(TUart* aUart, TSequencer *aSeq, TTermPathMng *aPathMng)
{
    mUart = aUart;
    mSeq = aSeq;
    mPathMng = aPathMng;

    aPathMng->mPathAccessApp->mTerm = this;

    mUart->config(112500, UP_NONE);
    mUart->installRxCb(rxCb, this);
    mUart->installTxCb(txCb, this);

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
    
    mRootApp = 0;
    mLastApp = 0;
    mAktApp = 0;

    addApp(&gAppCd);
    addApp(&gAppLs);
}

void TTerminal::rxCb(void* aArg)
{
    TTerminal *pObj = (TTerminal*) aArg; 

    pObj->mSeq->queueTask(pObj->mRxTaskId);
}

bool TTerminal::txCb(void* aArg)
{
    TTerminal *pObj = (TTerminal*) aArg; 

    pObj->mSeq->queueTask(pObj->mTxTaskId);

    return false;
}

void TTerminal::termTxTask(void* aArg)
{
    TTerminal *pObj = (TTerminal*) aArg; 

    uint32_t locWInd = pObj->mTxBufWInd;
    uint32_t locRInd = pObj->mTxBufRInd;

    if(locWInd != locRInd)
    {
        uint32_t txAvail;
        uint32_t txLen;
        if(locWInd > locRInd)
        {
            txAvail = locWInd - locRInd;
            txLen = pObj->mUart->txBlock(&pObj->mTxBuf[pObj->mTxBufRInd], txAvail);
            pObj->mTxBufRInd += txLen;
        }
        else
        {
            txAvail = TERMINAL_TX_BUF_LEN - locRInd;
            txLen = pObj->mUart->txBlock(&pObj->mTxBuf[pObj->mTxBufRInd], txAvail);
            if(txLen == txAvail)
            {         
                if(locWInd > 0)
                {
                    txAvail = locWInd;
                    txLen = pObj->mUart->txBlock(&pObj->mTxBuf[0], txAvail);
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

void TTerminal::termRxTask(void* aArg)
{
    TTerminal *pObj = (TTerminal*) aArg; 

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
            if(buf[readCnt] == CTRLSYM_ETX)
            {
                pObj->exitApp();
            }
            else
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
        }

        readCnt++;
        if(readCnt == readLen)
        {
            readLen = pObj->mUart->rxBlock(buf, 16);
            readCnt = 0;
        }
    }
}

uint8_t TTerminal::decodeEsc(uint8_t aChar)
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

void TTerminal::recordChar(uint8_t aChar)
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
                }
                else
                {
                    newLine(false);
                }
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

void TTerminal::moveCursor(ctrlSym_e aDir, uint32_t aDist)
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

            if(aDist > 9)
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

void TTerminal::parseLine()
{
    // find first char
    uint32_t pos = findNextC((char*) mLineBuf[mAktLine]);
    uint8_t* cmpStr = &mLineBuf[mAktLine][pos];

    if(cmpStr[0] == '.' || cmpStr[0] == '/')
    {
        mAktApp = mPathMng->mPathAccessApp;
        mAktApp->start(cmpStr);
    }
    else
    {
        // find next white space
        uint32_t len =  findNextS((char*) cmpStr);

        // find app and launch
        TTermApp* app = mRootApp;
        while(app != 0 && strncmp((char*)cmpStr, app->mKeyPhrase, len) != 0)
            app = app->mNext;

        if(app != 0)
        {
            uint32_t argPos = findNextC((char*) &cmpStr[len]);
            uint8_t* arg = &cmpStr[len + argPos];

            mAktApp = app;
            mAktApp->start(arg);
        }        
        else
        {
            const char* str = "unknowen command\r\n";
            putString(str, strlen(str));
            newLine();
        }
    }
}

void TTerminal::clrLine()
{
    putString("\033[K",3);
}

void TTerminal::putString(const char *aStr, uint32_t len)
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

void TTerminal::putChar(uint8_t aChar)
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
            case CTRLSYM_ES:
                putString("\033[J",3);
                break;
            case CTRLSYM_POS00:
                putString("\033[H",3);
                break;               
            
        }
    }

    mSeq->queueTask(mTxTaskId);
}

uint32_t TTerminal::findNextS(const char* str)
{
    uint32_t pos = 0;
    while(  str[pos] != ' ' &&
            str[pos] != '\n' &&
            str[pos] != '\r' && 
            str[pos] != 0)
        pos++;
    return pos;
}

uint32_t TTerminal::findNextC(const char* str)
{
    uint32_t pos = 0;
    while(  (   str[pos] == ' '  ||
                str[pos] == '\n' ||
                str[pos] == '\r' ) && 
                str[pos] != 0)
        pos++;

    if(str[pos] == 0)
        return 0;
    else
        return pos;
}


void TTerminal::newLine(bool aLineShift)
{
    if(aLineShift)
    {
        if(mAktLine == TERMINAL_LINE_CNT - 1)
            mAktLine = 0;
        else
            mAktLine++;
        
        mCpyLine = mAktLine;
        mCursorPos = 0;
        mLineBuf[mAktLine][mCursorPos] = 0;
    }

    // we use mLineBuf as workbuffer
    uint32_t len = mPathMng->getAktPath((char*) mLineBuf[mAktLine], TERMINAL_LINE_LENGTH);
    mLineBuf[mAktLine][len++] = '>';
    mLineBuf[mAktLine][len++] = 0;

    putString((char*) mLineBuf[mAktLine], len + 2);
    mLineBuf[mAktLine][0] = 0;
}

void TTerminal::exitApp()
{
    mAktApp->exit();
    mAktApp = 0;
    newLine();
}

void TTerminal::addApp(TTermApp* aApp)
{
    if(mLastApp == 0)
    {
        mRootApp = aApp;
        mLastApp = aApp;
        aApp->mNext = 0;
        aApp->mTerm = this;
    }
    else
    {
        mLastApp->mNext = aApp;
        mLastApp = aApp;
        aApp->mNext = 0;
        aApp->mTerm = this;
    }
}