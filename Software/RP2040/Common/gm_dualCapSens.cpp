#include "gm_dualCapSens.h"
#include <string.h>

GM_dualCapSense::GM_dualCapSense() :
mPara( (TParaTable::paraRec_t[2][cParaListLength]) {
    {
        [PARA_LEVEL] =       {.para = 0,           .pFAccessCb = paraLevelCb,  .cbArg = this,       .defs = &cParaList[PARA_LEVEL]},
        [PARA_VAL_AS] =      {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_AS]},
        [PARA_VAL_L10] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L10]},
        [PARA_VAL_L20] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L20]},
        [PARA_VAL_L30] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L30]},
        [PARA_VAL_L40] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L40]},
        [PARA_VAL_L50] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L50]},
        [PARA_VAL_L60] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L60]},
        [PARA_VAL_L70] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L70]},
        [PARA_VAL_L80] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L80]},
        [PARA_VAL_L90] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L90]},
        [PARA_VAL_L100] =    {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L100]},
        [PARA_CAL] =         {.para = 0,           .pFAccessCb = paraCalCb,    .cbArg = this,       .defs = &cParaList[PARA_CAL]},
        [PARA_THRES] =       {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_THRES]},
        [PARA_DIR] =         {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_DIR]},
        [PARA_CAL_AS] =      {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_AS]},
        [PARA_CAL_L10] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L10]},
        [PARA_CAL_L20] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L20]},
        [PARA_CAL_L30] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L30]},
        [PARA_CAL_L40] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L40]},
        [PARA_CAL_L50] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L50]},
        [PARA_CAL_L60] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L60]},
        [PARA_CAL_L70] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L70]},
        [PARA_CAL_L80] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L80]},
        [PARA_CAL_L90] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L90]},
        [PARA_CAL_L100] =    {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L100]}
    },
    {
        [PARA_LEVEL] =       {.para = 0,           .pFAccessCb = paraLevelCb,  .cbArg = this,       .defs = &cParaList[PARA_LEVEL]},
        [PARA_VAL_AS] =      {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_AS]},
        [PARA_VAL_L10] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L10]},
        [PARA_VAL_L20] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L20]},
        [PARA_VAL_L30] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L30]},
        [PARA_VAL_L40] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L40]},
        [PARA_VAL_L50] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L50]},
        [PARA_VAL_L60] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L60]},
        [PARA_VAL_L70] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L70]},
        [PARA_VAL_L80] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L80]},
        [PARA_VAL_L90] =     {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L90]},
        [PARA_VAL_L100] =    {.para = 0,           .pFAccessCb = paraValCb,    .cbArg = this,       .defs = &cParaList[PARA_VAL_L100]},
        [PARA_CAL] =         {.para = 0,           .pFAccessCb = paraCalCb,    .cbArg = this,       .defs = &cParaList[PARA_CAL]},
        [PARA_THRES] =       {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_THRES]},
        [PARA_DIR] =         {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_DIR]},
        [PARA_CAL_AS] =      {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_AS]},
        [PARA_CAL_L10] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L10]},
        [PARA_CAL_L20] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L20]},
        [PARA_CAL_L30] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L30]},
        [PARA_CAL_L40] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L40]},
        [PARA_CAL_L50] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L50]},
        [PARA_CAL_L60] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L60]},
        [PARA_CAL_L70] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L70]},
        [PARA_CAL_L80] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L80]},
        [PARA_CAL_L90] =     {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L90]},
        [PARA_CAL_L100] =    {.para = 0,           .pFAccessCb = 0,            .cbArg = 0,          .defs = &cParaList[PARA_CAL_L100]}
    }
}),
mEndpoint( (TParaTable::endpoint_t[2]) { 
    {
        { { 
            .baseInd = CCapLevelBaseRegAdr,
            .type = (uint16_t)EPT_CAPLEVEL    
        } }, 
        .length = cParaListLength, 
        .para = mPara[0],
        .next = 0,
        .typeName = cTypeName
    },
    {
        { { 
            .baseInd = CCapLevelBaseRegAdr + 32,
            .type = (uint16_t)EPT_CAPLEVEL    
        } }, 
        .length = cParaListLength, 
        .para = mPara[1],
        .next = 0,
        .typeName = cTypeName
    }    
})
{
    strncpy(mEndpoint[0].epName, cTypeName, sizeof(mEndpoint[0].epName));
    strncpy(mEndpoint[1].epName, cTypeName, sizeof(mEndpoint[1].epName));

    uint32_t len = strlen(cTypeName);
    mEndpoint[0].epName[len] = '0';
    mEndpoint[0].epName[len+1] = 0;
    mEndpoint[1].epName[len] = '1';
    mEndpoint[2].epName[len+1] = 0;
}

void GM_dualCapSense::init(TParaTable* aParaTable, TSequencer* aSeq_c1)
{
    mPT = aParaTable;

    mCapSens.init(pio0, gpio_dls_capSensExc_base, gpio_dls_capSens_base, gpio_dls_capSens_chNo, 0x000007FF, 62500, 125, aSeq_c1);

    mPT->addEndpoint(&mEndpoint[0]);
    mPT->addEndpoint(&mEndpoint[1]);
}

void GM_dualCapSense::paraLevelCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite)
{
    GM_dualCapSense *pObj = (GM_dualCapSense*) aCbArg;

    uint32_t level = 0;

    if(aPParaRec == &pObj->mPara[0][PARA_LEVEL])
    {
        if(pObj->mPara[0][PARA_DIR].para == 0)
        {
            for(int i = 9; i >= 0; i--)
            {
                uint32_t val = pObj->mCapSens.mCapVal[cap_ch_sens_p + i] - pObj->mPara[0][i + PARA_CAL_L10].para;
                if(val > pObj->mPara[0][PARA_THRES].para)
                    level += 10;
                else
                    break;
            }
            aPParaRec->para = level;
        }
        else
        {
            for(int i = 0; i <= 9; i++)
            {
                uint32_t val = pObj->mCapSens.mCapVal[cap_ch_sens_p + i] - pObj->mPara[0][i + PARA_CAL_L10].para;
                if(val > pObj->mPara[0][PARA_THRES].para)
                    level += 10;
                else
                    break;
            }
            aPParaRec->para = level;
        }
    }

    if(aPParaRec == &pObj->mPara[1][PARA_LEVEL])
    {
        if(pObj->mPara[1][PARA_DIR].para == 0)
        {
            for(int i = 9; i >= 0; i--)
            {
                uint32_t val = pObj->mCapSens.mCapVal[cap_ch_sens_n + i] - pObj->mPara[1][i + PARA_CAL_L10].para;
                if(val > pObj->mPara[1][PARA_THRES].para)
                    level += 10;
                else
                    break;
            }
            aPParaRec->para = level;
        }
        else
        {
            for(int i = 0; i <= 9; i++)
            {
                uint32_t val = pObj->mCapSens.mCapVal[cap_ch_sens_n + i] - pObj->mPara[1][i + PARA_CAL_L10].para;
                if(val > pObj->mPara[1][PARA_THRES].para)
                    level += 10;
                else
                    break;
            }
            aPParaRec->para = level;
        }
    }
}

void GM_dualCapSense::paraValCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite)
{
    GM_dualCapSense *pObj = (GM_dualCapSense*) aCbArg;

    if(aPParaRec >= &pObj->mPara[0][PARA_VAL_AS] && aPParaRec <= &pObj->mPara[0][PARA_VAL_L100])
    {
        uint32_t paraInd = (aPParaRec - &pObj->mPara[0][PARA_VAL_AS]);

        if(paraInd == 0)
        {
            // aktiv shield
            aPParaRec->para = pObj->mCapSens.mCapVal[cap_ch_as_p] - pObj->mPara[0][PARA_CAL_AS].para;
        }
        else
        {  
            // sensor field
            aPParaRec->para = pObj->mCapSens.mCapVal[cap_ch_sens_p + paraInd - 1] - pObj->mPara[0][PARA_CAL_AS + paraInd].para;
        }

    }

    if(aPParaRec >= &pObj->mPara[1][PARA_VAL_AS] && aPParaRec <= &pObj->mPara[1][PARA_VAL_L100])
    {
        uint32_t paraInd = (aPParaRec - &pObj->mPara[1][PARA_VAL_AS]);
        
        if(paraInd == 0)
        {
            // aktiv shield
            aPParaRec->para = pObj->mCapSens.mCapVal[cap_ch_as_n] - pObj->mPara[1][PARA_CAL_AS].para;
        }
        else
        {  
            // sensor field
            aPParaRec->para = pObj->mCapSens.mCapVal[cap_ch_sens_n + paraInd - 1] - pObj->mPara[1][PARA_CAL_AS + paraInd].para;
        }
    }
}

void GM_dualCapSense::paraCalCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite)
{
    GM_dualCapSense *pObj = (GM_dualCapSense*) aCbArg;

    if(aPParaRec == &pObj->mPara[0][PARA_CAL])
    {
        switch(aPParaRec->para)
        {
            case 1:
                pObj->mPara[0][PARA_CAL_AS].para = pObj->mCapSens.mCapVal[cap_ch_as_p];

                for(int i = 0; i < 10; i++)
                    pObj->mPara[0][PARA_CAL_L10 + i].para = pObj->mCapSens.mCapVal[cap_ch_sens_p + i];
                break;

            case 2:
                {
                    uint32_t sum = 0;
                    for(int i = 0; i < 10; i++)
                        sum += pObj->mCapSens.mCapVal[cap_ch_sens_p + i] - pObj->mPara[0][i + PARA_CAL_L10].para;

                    pObj->mPara[0][PARA_THRES].para = sum/20;
                }
                break;

            default:
                break;

        }
    }

    if(aPParaRec == &pObj->mPara[1][PARA_CAL])
    {
        switch(aPParaRec->para)
        {
            case 1:
                pObj->mPara[1][PARA_CAL_AS].para = pObj->mCapSens.mCapVal[cap_ch_as_n];

                for(int i = 0; i < 10; i++)
                    pObj->mPara[1][PARA_CAL_L10 + i].para = pObj->mCapSens.mCapVal[cap_ch_sens_n + i];
                break;

            case 2:
                {
                    uint32_t sum = 0;
                    for(int i = 0; i < 10; i++)
                        sum += pObj->mCapSens.mCapVal[cap_ch_sens_n + i] - pObj->mPara[1][i + PARA_CAL_L10].para;

                    pObj->mPara[1][PARA_THRES].para = sum/20;
                }
                break;

            default:
                break;

        }
    }
}