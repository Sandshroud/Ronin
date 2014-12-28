/***
 * Demonstrike Core
 */

#pragma once

#include <stdlib.h>
#include <openssl/md5.h>
#include "../Common.h"

class MD5Hash
{
public:
    MD5Hash();
    ~MD5Hash();

    void UpdateData(const uint8 *dta, int len);
    void UpdateData(const std::string &str);

    void Initialize();
    void Finalize();

    uint8 *GetDigest(void) { return mDigest; };
    int GetLength(void) { return MD5_DIGEST_LENGTH; };

private:
    MD5_CTX mC;
    uint8 mDigest[MD5_DIGEST_LENGTH];
};
