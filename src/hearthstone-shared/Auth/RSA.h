/***
 * Demonstrike Core
 */

#pragma once

#include <stdlib.h>
#include "../Common.h"
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

class RSAHash
{
public:
    RSAHash();
    ~RSAHash();

    void Initialize();

    std::string RSAEncrypt(const std::string &str);
    std::string RSADecrypt(const std::string &str);

private:
    RSA* RSAInternal;
};
