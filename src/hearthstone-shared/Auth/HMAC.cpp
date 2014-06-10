/***
 * Demonstrike Core
 */

#include "HMAC.h"
#include <stdarg.h>

HMACHash::HMACHash()
{
}

void HMACHash::Initialize(uint32 len, uint8 *seed)
{
    HMAC_CTX_init(&mC);
    HMAC_Init_ex(&mC, seed, len, EVP_sha1(), NULL);
}

HMACHash::~HMACHash()
{
    HMAC_CTX_cleanup(&mC);
}

void HMACHash::UpdateBigNumber(BigNumber *bn0, ...)
{
    va_list v;
    BigNumber *bn;

    va_start(v, bn0);
    bn = bn0;
    while (bn)
    {
        UpdateData(bn->AsByteArray(), bn->GetNumBytes());
        bn = va_arg(v, BigNumber *);
    }
    va_end(v);
}

void HMACHash::UpdateData(const uint8 *data, int length)
{
    HMAC_Update(&mC, data, length);
}

void HMACHash::UpdateData(const std::string &str)
{
    UpdateData((uint8 *)str.c_str(), (int)str.length());
}

void HMACHash::Finalize()
{
    uint32 length = 0;
    HMAC_Final(&mC, (uint8*)mDigest, &length);
}
