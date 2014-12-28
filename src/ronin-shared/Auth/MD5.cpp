/***
 * Demonstrike Core
 */

#include "MD5.h"

MD5Hash::MD5Hash()
{
    MD5_Init(&mC);
}

MD5Hash::~MD5Hash()
{

}

void MD5Hash::UpdateData(const std::string &str)
{
    UpdateData((const uint8*)str.data(), (int)str.length());
}

void MD5Hash::UpdateData(const uint8 *dta, int len)
{
    MD5_Update(&mC, dta, len);
}

void MD5Hash::Finalize()
{
    MD5_Final(mDigest, &mC);
}

void MD5Hash::Initialize()
{
    MD5_Init(&mC);
}

