/***
 * Demonstrike Core
 */

#pragma once

#include <stdlib.h>
#include "Common.h"
#include <openssl/hmac.h>
#include "Auth/BigNumber.h"

class HMACHash
{
    public:
        HMACHash();
        ~HMACHash();

        void UpdateFinalizeBigNumbers(BigNumber *bn0, ...);
        void UpdateBigNumber(BigNumber *bn0, ...);

        void UpdateData(const uint8 *dta, int len);
        void UpdateData(const std::string &str);

        void Initialize(uint32 len, uint8 *seed);
        void Finalize();

        uint8 *GetDigest(void) { return mDigest; };
        int GetLength(void) { return 20; };

        BigNumber GetBigNumber();

    private:
        HMAC_CTX mC;
        uint8 mDigest[20];
};
