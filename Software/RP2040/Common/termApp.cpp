#include "termApp.h"

TTermApp::TTermApp()
{

}

TTermApp::~TTermApp()
{

}

void TTermApp::printErr(errCode_T aEc)
{
    const char* errorTex = CErrorDesc[aEc];
    putString(errorTex, strlen(errorTex));
    putChar('\r');
    putChar('\n');
}