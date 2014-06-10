/***
 * Demonstrike Core
 */

#pragma once

#include <stdlib.h>
#include "../Common.h"
#include "HMAC.h"
#include "../RC4Engine.h"
#include "BigNumber.h"
#include <vector>

class WowCrypt 
{
public:
    WowCrypt();
    ~WowCrypt();

    void Init(uint8 *K);
    void DecryptRecv(uint8 * data, size_t len);
    void EncryptSend(uint8 * data, size_t len);

    bool IsInitialized() { return _initialized; }

private:
    bool _initialized;
    RC4Engine _Decrypt;
    RC4Engine _Encrypt;
};
