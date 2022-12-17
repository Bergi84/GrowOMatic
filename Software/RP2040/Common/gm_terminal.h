#ifndef GM_TERMINAL_H_
#define  GM_TERMINAL_H_

#include "uart.h"
#include "sequencer_armm0.h"

class gm_termApp;

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

class gm_terminal 
{
public:
    void init(TUart* aUart, TSequencer *aSeq);

private:
    friend class gm_termApp;

    TUart *mUart;
    TSequencer *mSeq;

    uint8_t mLineBuf[TERMINAL_LINE_CNT][TERMINAL_LINE_LENGTH];
    uint32_t mAktLine;
    uint32_t mCursorPos;
    uint32_t mCpyLine;

    bool mEscAkt;
    uint8_t mEscBuf[TERMINAL_MAX_ESC_LEN];
    uint32_t mEscPos;

    gm_termApp* mRootApp;
    gm_termApp* mAktApp;

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


class gm_termApp
{
public:

protected:
    gm_termApp();

private:
    friend class gm_terminal;

    gm_terminal* mTerm;
    gm_termApp* mNext;
    const char* mKeyPhrase;

    inline void putChar(uint8_t aChar)
    {
        mTerm->putChar(aChar);
    }
    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);
    inline void done()
    {
        mTerm->exitApp();
    }
};

#endif /*  GM_TERMINAL_H_ */