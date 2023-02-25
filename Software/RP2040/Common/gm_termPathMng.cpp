#include "gm_termPathMng.h"
#include <string.h>
#include <stdio.h>

GM_termPathMng::GM_termPathMng() 
{

}

GM_termPathMng::~GM_termPathMng()
{

}

void GM_termPathMng::init(GM_bus* aBus, TParaTable* aParaTable)
{
    mBM = aBus->getBusMaster();
    mPT = aParaTable;
    mAktAdr.type = PT_ROOT;
    mPathAccessApp = &mPA;
}

uint32_t GM_termPathMng::getSubPath(uint32_t aInd, char* aSubPath, uint32_t aSubPathLen, char* aPath)
{
    pathRes_t path;
    if(aPath == 0)
        path = mAktAdr;
    else
        path = pathParse(aPath);

    if(aSubPathLen == 0)
        return 0;

    switch(path.type)
    {
        case PT_BUS:
            if(path.adr == CInvalidAdr)
            {
                GM_device* dev = mBM->findDev(path.bus, aInd);
                if(dev == 0)
                    return 0;

                uint32_t len = printSlaveName(aInd, aSubPath, aSubPathLen);
                return len;
            }
            break;

        case PT_LOC:
            break;

        case PT_UID:
            if(path.uid == CInvalidUid)
            {
                GM_device* dev = mBM->getDeviceLL();
                for(int i = 0; i < aInd && dev != 0; i++)
                    dev = dev->mNext;

                if(dev == 0)
                    return 0;       

                uint32_t len = printUid(dev->getUid(), aSubPath, aSubPathLen);
                return len;
            }
            break;

        case PT_DEV:
            if(path.uid == CInvalidUid)
            {
                GM_device* dev = mBM->getDeviceLL();
                for(int i = 0; i < aInd && dev != 0; i++)
                    dev = dev->mNext;

                if(dev == 0)
                    return 0;       


                uint32_t len = strlen(dev->getDevName());

                if(len >= aSubPathLen)
                    return 0;
                
                strcpy(aSubPath, dev->getDevName());
                return len;
            }
            break;

        case PT_INVALID:
            return 0;

        case PT_ROOT:
            if(mBM != 0 && mBM->isInit())
            {
                if(aInd < mBM->getBusNo())
                {
                    uint32_t busNo = aInd;

                    uint32_t len = strlen(cPathTypeName[PT_BUS]);
                    if(aSubPathLen - 1 <= len)
                        return 0;

                    strcpy(aSubPath, cPathTypeName[PT_BUS]);

                    do
                    {
                        if(len >= aSubPathLen - 1)
                            return 0;
                        
                        aSubPath[len++] = busNo%10 + '0';
                        busNo /= 10;
                    }
                    while(busNo != 0);

                    return len;
                }
                else
                {
                    if(aInd < mBM->getBusNo() + PT_DEV)
                    {
                        uint32_t len = strlen(cPathTypeName[aInd - mBM->getBusNo() + 1]);
                        if(aSubPathLen - 1 <= len)
                            return 0;

                        strcpy(aSubPath, cPathTypeName[aInd - mBM->getBusNo() + 1]);         
                        return len;               
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            else
            {
                if(aInd == 0)
                {
                    uint32_t len = strlen(cPathTypeName[PT_LOC]);
                    if(aSubPathLen - 1 <= len)
                        return 0;

                    strcpy(aSubPath, cPathTypeName[PT_LOC]);         
                    return len;                      
                }
                else
                {
                    return 0;
                }
            }
            break;
    }


    // after this point it is garanted that we have a kowen device
    // so we can find the device and select a endpoint

    GM_device* dev;

    if(path.type != PT_LOC)
    {   
        if(path.type == PT_BUS)
            dev = mBM->findDev(path.bus, path.adr);
        else
            dev = mBM->findDev(path.uid);
    }

    if(path.baseInd == CInvalidReg)
    {
        const char* nameStr; 

        if(path.type == PT_LOC)
        {
            TParaTable::endpoint_t* epLoc = mPT->getEpLL();
            for(int i = 0; i < aInd && epLoc != 0; i++)
                epLoc = epLoc->next;
            
            if(epLoc == 0)
                return 0;

            nameStr = epLoc->epName;
        }
        else
        {
            TEpBase* ep = dev->getEpLL();

            for(int i = 0; i < aInd && ep != 0; i++)
                ep = ep->mNext;

            if(ep == 0)
                return 0;

            nameStr = ep->getEpName();
        }
        if(nameStr == 0)
            return 0;  

        uint32_t len = strlen(nameStr);
        if(aSubPathLen - 1 <= len)
            return 0;

        strcpy(aSubPath, nameStr);         
        return len;     
    }

    if(path.offInd == CInvalidReg)
    {
        const char* nameStr; 

        if(path.type == PT_LOC)
        {
            TParaTable::endpoint_t* epLoc = mPT->findEp(path.baseInd);
            
            if(epLoc == 0)
                return 0;

            if(aInd >= epLoc->length + 4)
                return 0;

            mPT->getParaName(path.baseInd + aInd, &nameStr);
        }
        else
        {
            TEpBase* ep = dev->findEp(path.baseInd);

            if(ep == 0)
                return 0;

            if(aInd >= ep->getParaListLen())
                return 0;

            nameStr = ep->getParaName(aInd);
        }
        if(nameStr == 0)
            return 0;    

        uint32_t len = strlen(nameStr);
        if(aSubPathLen - 1 <= len)
            return 0;

        strcpy(aSubPath, nameStr);         
        return len;    
    }

    // there is no further subpath
    return 0;
}

errCode_T GM_termPathMng::setAktPath(char* aPath, uint32_t aPathLen)
{
    pathRes_t newPath = pathParse(aPath, aPathLen);

    if(newPath.type != PT_INVALID && newPath.offInd == CInvalidReg)
    {
        mAktAdr = newPath;
        return EC_SUCCESS;
    }
    else
    {
        if(newPath.offInd != CInvalidReg)
            return EC_NOTAFOLDER;
        else
            return EC_INVALID_PATH;
    }
}   

uint32_t GM_termPathMng::getAktPath(char* aPath, uint32_t aPathLen)
{
    return genPathString(&mAktAdr, aPath, aPathLen);
}

GM_termPathMng::pathRes_t GM_termPathMng::pathParse(char *aPathStr, uint32_t aStrLen)
{
    if(aPathStr == 0 || aPathStr[0] == 0)
        return mAktAdr;

    pathRes_t path;

    uint32_t pathInd = 0;

    // init path resoluton struct
    if(aPathStr[0] == '.' && aPathStr[1] == '/')
    {
        // '.' means relative path to aktiv path
        path = mAktAdr;
        pathInd = 2;
    }
    else
    {
        if(aPathStr[0] == '/')
        {
            // '/' means a absolut path
            path.type = PT_ROOT;
            pathInd = 1;
        }
        else
        {
            path.type = PT_INVALID;
        }
    }

    

    while(1)
    {
        // find next path element
        char* pathEle = &aPathStr[pathInd];
        uint32_t pathEleLen = 0;

        while(  pathEle[pathEleLen] != 0 && 
                pathEle[pathEleLen] != '/' && 
                (aStrLen == 0 || pathEleLen + pathInd < aStrLen))
            pathEleLen++;
        
        pathInd += pathEleLen;

        if(aPathStr[pathInd] == '/')
            pathInd++;

        if(pathEleLen == 0)
        {
            if(pathEle[pathEleLen] == '/')
            {
                // found two slashes
                path.type = PT_INVALID;
                return path;
            }
            else
            {
                // end of path
                return path;
            }
        }

        // parse path element
        switch(path.type)
        {
            case PT_ROOT:
                if(     mBM != 0 && mBM->isInit() && 
                        strncmp(pathEle, cPathTypeName[PT_BUS], strlen(cPathTypeName[PT_BUS])) == 0 &&
                        strlen(cPathTypeName[PT_BUS]) < pathEleLen)
                {
                    uint32_t cmpStrLen = strlen(cPathTypeName[PT_BUS]);
                    uint32_t busNo = 0;

                    for(int i = cmpStrLen; i < pathEleLen; i++)
                    {
                        if(pathEle[cmpStrLen] < '0' || pathEle[cmpStrLen] > '9')
                        {
                            path.type = PT_INVALID;
                            return path;
                        }
                        else
                        {
                            busNo = busNo*10 + (pathEle[cmpStrLen] - '0');
                        }
                    }
                    path.bus = busNo;
                    path.adr = CInvalidAdr;
                    path.baseInd = CInvalidReg;
                    path.offInd = CInvalidReg;
                    path.type = PT_BUS;
                } 
                else if (   strncmp(pathEle, cPathTypeName[PT_LOC], strlen(cPathTypeName[PT_LOC])) == 0 &&
                            strlen(cPathTypeName[PT_LOC]) == pathEleLen)
                {
                    path.type = PT_LOC;
                    path.baseInd = CInvalidReg;
                    path.offInd = CInvalidReg;
                }
                else if (   mBM != 0 && mBM->isInit() && 
                            strncmp(pathEle, cPathTypeName[PT_UID], strlen(cPathTypeName[PT_UID])) == 0 &&
                            strlen(cPathTypeName[PT_UID]) == pathEleLen)
                {
                    path.type = PT_UID;
                    path.uid = CInvalidUid;
                    path.baseInd = CInvalidReg;
                    path.offInd = CInvalidReg;
                } 
                else if (   mBM != 0 && mBM->isInit() && 
                            strncmp(pathEle, cPathTypeName[PT_DEV], strlen(cPathTypeName[PT_DEV])) == 0 &&
                            strlen(cPathTypeName[PT_DEV]) == pathEleLen)
                {
                    path.type = PT_DEV;
                    path.uid = CInvalidUid;
                    path.baseInd = CInvalidReg;
                    path.offInd = CInvalidReg;
                } 
                else
                {
                    path.type = PT_INVALID;
                    return path;
                }
                break;

            case PT_BUS:
                if(path.adr == CInvalidAdr)
                {
                    if(pathEleLen == 2 && pathEle[0] == '.' && pathEle[1] == '.')
                    {
                        // one dir up
                        path.bus = CInvalidBus;
                        path.type = PT_ROOT;
                    }
                    else
                    {
                        // find slave number
                        uint32_t len = strlen(cSlaveStr);
                        if( strncmp(pathEle, cSlaveStr, len) == 0 &&
                            len < pathEleLen)
                        {
                            uint32_t slaveNo = 0;                        

                            for(int i = len; i < pathEleLen; i++)
                            {
                                if(pathEle[len] < '0' || pathEle[len] > '9')
                                {
                                    path.type = PT_INVALID;
                                    return path;
                                }
                                else
                                {
                                    slaveNo = slaveNo*10 + (pathEle[len] - '0');
                                }
                            }
                            path.adr = slaveNo;
                        }
                        else
                        {
                            path.type = PT_INVALID;
                            return path;
                        }
                    }
                }
                else
                {
                    // find device
                    GM_device* aktDev = mBM->findDev(path.bus, path.adr);

                    parseEpRegName(&path, aktDev, pathEle, pathEleLen);
                }
                break;

            case PT_LOC:
                parseLocEpRegName(&path, pathEle, pathEleLen);
                break;

            case PT_UID:
                if(path.uid == CInvalidUid)
                {
                    if(pathEleLen == 2 && pathEle[0] == '.' && pathEle[1] == '.')
                    {
                        // one dir up
                        path.type = PT_ROOT;
                    }
                    else
                    {
                        GM_device* dev = parseUid(pathEle, pathEleLen);

                        if(dev == 0 || dev->getUid() == CInvalidUid)
                        {
                            path.type = PT_INVALID;
                            return path;
                        }

                        path.uid = dev->getUid();
                    }
                }
                else
                {
                    // find device
                    GM_device* aktDev = mBM->findDev(path.uid);

                    parseEpRegName(&path, aktDev, pathEle, pathEleLen); 
                }
                break;

            case PT_DEV:
                if(path.uid == CInvalidUid)
                {     
                    if(pathEleLen == 2 && pathEle[0] == '.' && pathEle[1] == '.')
                    {
                        // one dir up
                        path.type = PT_ROOT;
                    }
                    else
                    {
                        GM_device* dev = parseDevName(pathEle, pathEleLen);

                        if(dev == 0 || dev->getUid() == CInvalidUid)
                        {
                            path.type = PT_INVALID;
                            return path;
                        }

                        path.uid = dev->getUid(); 
                    }
                }
                else
                {
                    // find device
                    GM_device* aktDev = mBM->findDev(path.uid);

                    parseEpRegName(&path, aktDev, pathEle, pathEleLen); 
                }
                break;

            default:
                path.type = PT_INVALID;
                return path;
        }

    }

    return path;
}

void GM_termPathMng::parseEpRegName(pathRes_t *aPath, GM_device* aDev, char* aStrBuf, uint32_t aBufLen)
{

    if(aDev == 0)
    {
        aPath->type = PT_INVALID;
        return;    
    }

    if(aPath->baseInd != CInvalidReg)
    {
        if(aBufLen == 2 && aStrBuf[0] == '.' && aStrBuf[1] == '.')
        {
            // one dir up
            aPath->baseInd = CInvalidReg;
        }
        else
        {
            // parse register name
            if(aPath->offInd != CInvalidReg)
            {
                // we have already a full path
                aPath->type = PT_INVALID;
                return;
            }

            TEpBase* ep = aDev->findEp(aPath->baseInd);
            uint16_t regOff = parseRegName(ep, aStrBuf, aBufLen);

            if(regOff == CInvalidReg)
            {
                // reg name not found
                aPath->type = PT_INVALID;
                return;                           
            }

            aPath->offInd = regOff;
        }
    }
    else
    {
        if(aBufLen == 2 && aStrBuf[0] == '.' && aStrBuf[1] == '.')
        {
            // one dir up
            aPath->adr = CInvalidAdr;
            aPath->uid = CInvalidUid;
        }
        else
        {
            // parse endpoint name
            TEpBase* ep = parseEpName(aDev, aStrBuf, aBufLen);

            if(ep == 0)
            {
                aPath->type = PT_INVALID;
                return;
            }
            aPath->baseInd = ep->getBaseAdr();
        }
    }
}

void GM_termPathMng::parseLocEpRegName(pathRes_t *aPath, char* aStrBuf, uint32_t aBufLen)
{
    if(aPath->baseInd != CInvalidReg)
    {
        if(aBufLen == 2 && aStrBuf[0] == '.' && aStrBuf[1] == '.')
        {
            // one dir up
            aPath->baseInd = CInvalidReg;
        }
        else
        {
            // parse register name
            if(aPath->offInd != CInvalidReg)
            {
                // we have already a full path
                aPath->type = PT_INVALID;
                return;
            }

            TParaTable::endpoint_t* ep = mPT->findEp(aPath->baseInd);
            uint16_t regOff = parseLocRegName(ep, aStrBuf, aBufLen);

            if(regOff == CInvalidReg)
            {
                // reg name not found
                aPath->type = PT_INVALID;
                return;                           
            }

            aPath->offInd = regOff;
        }
    }
    else
    {
        if(aBufLen == 2 && aStrBuf[0] == '.' && aStrBuf[1] == '.')
        {
            // one dir up
            aPath->adr = CInvalidAdr;
            aPath->uid = CInvalidUid;
        }
        else
        {
            // parse endpoint name
            TParaTable::endpoint_t* ep = parseLocEpName(aStrBuf, aBufLen);

            if(ep == 0)
            {
                aPath->type = PT_INVALID;
                return;
            }
            aPath->baseInd = ep->epId.baseInd;
        }
    }
}

TEpBase* GM_termPathMng::parseEpName(GM_device* aDev, char* aPathStr, uint32_t aPathStrLen)
{
    TEpBase* aktEp = aDev->getEpLL();

    while(aktEp != 0)
    {
        char* epName = aktEp->getEpName();

        if( strncmp(epName, aPathStr, aPathStrLen) == 0 && 
            strlen(epName) == aPathStrLen)
            return aktEp;

        aktEp = aktEp->mNext;
    }

    return 0;
}

TParaTable::endpoint_t* GM_termPathMng::parseLocEpName(char* aPathStr, uint32_t aPathStrLen)
{
    TParaTable::endpoint_t* aktEp = mPT->getEpLL();

    while(aktEp != 0)
    {
        const char* epName = aktEp->epName;

        if( strncmp(epName, aPathStr, aPathStrLen) == 0 && 
            strlen(epName) == aPathStrLen)
            return aktEp;

        aktEp = aktEp->next;
    }

    return 0;
}

uint16_t GM_termPathMng::parseRegName(TEpBase* aEp, char* aPathStr, uint32_t aPathStrLen)
{
    uint32_t len = aEp->getParaListLen();

    int32_t offInd = 0;
    while(offInd < len)
    {
        const char* regName = aEp->getParaName(offInd);

        if( strncmp(regName, aPathStr, aPathStrLen) == 0 && 
            strlen(regName) == aPathStrLen)
            return offInd;

        offInd++;
    }

    return CInvalidReg;
}   

uint16_t GM_termPathMng::parseLocRegName(TParaTable::endpoint_t* aEp, char* aPathStr, uint32_t aPathStrLen)
{
    uint32_t len = aEp->length;

    int32_t offInd = 0;
    while(offInd < len)
    {
        const char* regName;
        mPT->getParaName(aEp->epId.baseInd + offInd, &regName);

        if( strncmp(regName, aPathStr, aPathStrLen) == 0 && 
            strlen(regName) == aPathStrLen)
            return offInd;

        offInd++;
    }
        
    while(offInd < len+4)
    {
        const char* regName = CEpNameDefs[offInd-len].paraName;

        if( strncmp(regName, aPathStr, aPathStrLen) == 0 && 
            strlen(regName) == aPathStrLen)
            return offInd;

        offInd++;
    }

    return CInvalidReg;
}   

GM_device* GM_termPathMng::parseUid(char* aPathStr, uint32_t aPathStrLen)
{
    if(aPathStrLen != 8)
        return 0;

    uint32_t uid;

    for(int i = 0; i < 8; i++)
    {
        char ch = aPathStr[i];
        if( ch >= '0' && ch <= '9')
        {
            uid = uid * 16 + (ch - '0');
        }
        else if ( ch >= 'a' && ch <= 'f')
        {
            uid = uid * 16 + (ch - 'a' + 10);
        }
        else
        {
            return 0;
        }
    }

    return mBM->findDev(uid);
}

GM_device* GM_termPathMng::parseDevName(char* aPathStr, uint32_t aPathStrLen)
{
    GM_device* aktDev = mBM->getDeviceLL();

    while(aktDev != 0)
    {
        char* devName = aktDev->getDevName();

        if( strncmp(devName, aPathStr, aPathStrLen) == 0 && 
            strlen(devName) == aPathStrLen)
            return aktDev;

        aktDev = aktDev->mNext;
    }

    return 0;
}

uint32_t GM_termPathMng::genPathString(pathRes_t *aPath, char* aStrBuf, uint32_t aBufLen)
{
    if(aBufLen == 0 || aStrBuf == 0)
        return 0;

    uint32_t ind = 0;
    aStrBuf[ind++] = '/';

    if(aPath->type == PT_INVALID)
        return 0;

    if(aPath->type == PT_ROOT)
    {
        aStrBuf[ind] = 0;
        return ind;
    }

    if(aBufLen - ind - 1 <= strlen(cPathTypeName[aPath->type]))
        return 0;

    strcpy(&aStrBuf[ind], cPathTypeName[aPath->type]);
    ind += strlen(cPathTypeName[aPath->type]);

    switch(aPath->type)
    {
        case PT_BUS:
            {
                // add bus no
                uint32_t busNo = aPath->bus;
                if(mBM->getBusNo() < busNo)
                    return 0;

                char noBuf[4];
                uint32_t noBufInd = 0;
                do
                {
                    if(noBufInd >= 4)
                        return 0;

                    noBuf[noBufInd++] = busNo%10 + '0';
                    busNo /= 10;
                }
                while(busNo != 0);

                for(int i = 0; i < noBufInd; i++)
                {
                    if(ind >= aBufLen - 1)
                        return 0;

                    aStrBuf[ind++] = noBuf[noBufInd - 1 -i];
                }

                // slave adr
                if(aPath->adr == CInvalidAdr)
                {
                    aStrBuf[ind] = 0;
                    return ind;
                }

                if(ind >= aBufLen - 1)
                    return 0;

                aStrBuf[ind++] = '/';

                ind += printSlaveName(aPath->adr, &aStrBuf[ind], aBufLen - 1 - ind);
            }
            break;

        case PT_LOC:
            // nothing todo
            break;

        case PT_UID:
            {
                // uid
                if(aPath->uid == CInvalidUid)
                {
                    aStrBuf[ind] = 0;
                    return ind;
                }

                if(ind >= aBufLen - 1)
                    return 0;

                aStrBuf[ind++] = '/';

                ind += printUid(aPath->uid, &aStrBuf[ind], aBufLen - 1 - ind);
            }
            break;

        case PT_DEV:
                {
                // uid
                if(aPath->uid == CInvalidUid)
                {
                    aStrBuf[ind] = 0;
                    return ind;
                }

                GM_device* dev = mBM->findDev(aPath->uid);
                if(ind >= aBufLen - 1 || dev == 0)
                    return 0;

                aStrBuf[ind++] = '/';

                uint32_t len = strlen(dev->getDevName());
                if(aBufLen - ind - 1 <= len)
                    return 0;

                strcpy(&aStrBuf[ind], dev->getDevName());
                ind += len;
            }
            break;    

        default:
            return 0;
    }

    
    if(aPath->baseInd == CInvalidReg)
    {
        aStrBuf[ind] = 0;
        return ind;
    }

    if(ind >= aBufLen - 1)
        return 0;

    aStrBuf[ind++] = '/';

    const char* nameStr;
    TEpBase* ep;
    TParaTable::endpoint_t* epLoc;

    if(aPath->type == PT_LOC)
    {
        epLoc = mPT->findEp(aPath->baseInd);
        if(epLoc == 0)
            return 0;

        nameStr = epLoc->epName;   
    }
    else
    {
        GM_device* dev;

        if(aPath->type == PT_BUS)
        {
            dev = mBM->findDev(aPath->bus, aPath->adr);
        }
        else
        {
            dev = mBM->findDev(aPath->uid);
        }

        if(dev == 0)
            return 0;

        ep = dev->findEp(aPath->baseInd);

        if(ep == 0)
            return 0;

        nameStr = ep->getEpName();
    }

    uint32_t len = strlen(nameStr);
    if(aBufLen - ind - 1 <= len)
        return 0;

    strcpy(&aStrBuf[ind], nameStr);
    ind += len;

    if(aPath->offInd == CInvalidReg)
    {
        aStrBuf[ind] = 0;
        return ind;
    }

    if(ind > aBufLen - 1)
        return 0;

    aStrBuf[ind++] = '/';

    if(aPath->type == PT_LOC)
    {
        mPT->getParaName(aPath->offInd + aPath->baseInd, &nameStr);
    }
    else
    {    
        uint32_t listLen = ep->getParaListLen();
        if(listLen <= aPath->offInd)
            return 0;

        nameStr = ep->getParaName(aPath->offInd);
    }

    len = strlen(nameStr);
    if(aBufLen - ind - 1 <= len)
        return 0;

    strcpy(&aStrBuf[ind], nameStr);
    ind += len;

    aStrBuf[ind] = 0;
    return ind;
    
}

uint32_t GM_termPathMng::printSlaveName(uint32_t aSlaveInd, char* aStr, uint32_t aStrLen)
{
    // add slave no
    return snprintf(aStr, aStrLen, "%s%u", cSlaveStr, aSlaveInd);
}

uint32_t GM_termPathMng::printUid(uint32_t aUid, char* aStr, uint32_t aStrLen)
{
    if(aStrLen < 9)
        return 0;

    for(int i = 0; i < 8; i++)
    {
        uint8_t chTmp = (aUid & 0xF0000000) >> 28;
        if(chTmp > 9)
            aStr[i] = chTmp - 10 + 'a';
        else
            aStr[i] = chTmp + '0';

        aUid <<= 4;
    }
    return 8;
}

void GM_termPathMng::getPathObj(char* aPath, uint32_t aPathLen, TPathEle* aEle)
{
    aEle->deInit();

    pathRes_t resPath = pathParse(aPath, aPathLen);


    if(resPath.type == PT_INVALID || resPath.type == PT_ROOT)
        return; 

    if(resPath.baseInd != CInvalidReg)
    {
        switch(resPath.type)
        {
            case PT_BUS:
                {
                    GM_device* dev = mBM->findDev(resPath.bus, resPath.adr);
                    if(dev == 0)
                        return; 

                    TEpBase* ep = dev->findEp(resPath.baseInd);

                    if(ep == 0 || (resPath.offInd != CInvalidReg && resPath.offInd >= ep->getParaListLen())) 
                        return;

                    aEle->init(ep, resPath.offInd);
                }
                break;

                case PT_DEV:
                case PT_UID:
                {
                    GM_device* dev = mBM->findDev(resPath.uid);
                    if(dev == 0)
                        return; 

                    TEpBase* ep = dev->findEp(resPath.baseInd);

                    if(ep == 0 || (resPath.offInd != CInvalidReg && resPath.offInd >= ep->getParaListLen())) 
                        return;
                        
                    aEle->init(ep, resPath.offInd);
                }
                break;

                case PT_LOC:
                {
                    TParaTable::endpoint_t* ep = mPT->findEp(resPath.baseInd);

                    if(ep == 0)
                        return;

                    if(resPath.offInd != CInvalidReg && resPath.offInd >= ep->length + 4) 
                        return;

                    aEle->init(ep, mPT, resPath.offInd);
                }
                break;
        }
    }
}