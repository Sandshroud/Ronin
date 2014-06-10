/***
 * Demonstrike Core
 */

#include "Threading.h"

void CallbackFP::operator()()
{
    mycallback();
}

void CallbackFP::execute()
{
    mycallback();
}

CallbackFP* CallbackFP::Create()
{
    return new CallbackFP(NULL);
}
