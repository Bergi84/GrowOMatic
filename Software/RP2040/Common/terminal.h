#ifndef GM_TERMINAL_H_
#define  GM_TERMINAL_H_

#include "uart.h"
#include "sequencer_armm0.h"

class TTermApp;
class TTermPathMng;

enum ctrlSym_e {
    // ascii c0 control symbols
    CTRLSYM_ETX = 0x03, // abort, crtl + c
    CTRLSYM_BS = 0x08,  // backspace
    CTRLSYM_CR = 0x0D,  // cariage return
    CTRLSYM_LF = 0x0A,  // line feed
    CTRLSYM_ESC = 0x1B, // esc sequence
    CTRLSYM_DEL = 0x7F, // delete
    
    // internal control symbols, c1 control symbols are not supported
    CTRLSYM_UNKNOWEN = 0x80,
    CTRLSYM_UP,
    CTRLSYM_DOWN,
    CTRLSYM_LEFT,
    CTRLSYM_RIGHT,
    CTRLSYM_EL
};

#define TERMINAL_LINE_LENGTH        81
#define TERMINAL_LINE_CNT           5
#define TERMINAL_MAX_ESC_LEN        16
#define TERMINAL_TX_BUF_LEN         512

class TTerminal 
{
public:
    void init(TUart* aUart, TSequencer *aSeq, TTermPathMng *aPathMng);

private:
    friend class TTermApp;

    TUart *mUart;
    TSequencer *mSeq;
     TTermPathMng *msPathMng;

    uint8_t mLineBuf[TERMINAL_LINE_CNT][TERMINAL_LINE_LENGTH];
    uint32_t mAktLine;
    uint32_t mCursorPos;
    uint32_t mCpyLine;

    bool mEscAkt;
    uint8_t mEscBuf[TERMINAL_MAX_ESC_LEN];
    uint32_t mEscPos;

    TTermApp* mRootApp;
    TTermApp* mAktApp;

    TTermPathMng* mPathMng;

    typedef enum
    {
        ESCD_ESC,
        ESCD_CSI,
        ESCD_OSC,
        ESCD_UNKNOWEN,
    } escDecode_t;

    escDecode_t mEscdState;
    uint8_t mRxTaskId;
    uint8_t mTxTaskId;

    uint8_t mTxBuf[TERMINAL_TX_BUF_LEN];
    uint32_t mTxBufWInd;
    uint32_t mTxBufRInd;

    static void rxCb(void* aArg);
    static void txCb(void* aArg);

    void recordChar(uint8_t aChar);
    static void termTxTask(void* aArg);
    static void termRxTask(void* aArg);

    uint8_t decodeEsc(uint8_t aChar);

    void parseLine();
    void clrLine();
    void putChar(uint8_t aChar);
    void putString(const char *aStr, uint32_t len);

    // aDist max = 99
    void moveCursor(ctrlSym_e aDir, uint32_t aDist);
    void newLine();
    void exitApp();
};


#endif /*  GM_TERMINAL_H_ */