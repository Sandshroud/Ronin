/***
 * Demonstrike Core
 */

#include "RSA.h"

RSAHash::RSAHash()
{

}

RSAHash::~RSAHash()
{

}

uint8 ServerEncryptionKey[83]
= { 0x06, 0x02, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x52, 0x53, 0x41, 0x31, 0x00, 0x02, 0x00, 0x00, // 06 02 00 00 00 24 00 00 52 53 41 31 00 02 00 00
    0x01, 0x00, 0x01, 0x00, 0x37, 0xD8, 0x6A, 0xED, 0xA4, 0x9E, 0x34, 0x30, 0xE4, 0x18, 0xC1, 0x4B, // 01 00 01 00 37 D8 6A ED A4 9E 34 30 E4 18 C1 4B
    0xB1, 0x9F, 0xEE, 0x90, 0x62, 0x42, 0xC5, 0x68, 0xCB, 0x8C, 0xB7, 0x51, 0xAD, 0x57, 0x75, 0x16, // B1 9F EE 90 62 42 C5 68 CB 8C B7 51 AD 57 75 16
    0x92, 0x0A, 0x0E, 0xEB, 0xFA, 0xF8, 0x1B, 0x37, 0x49, 0x7C, 0xDD, 0x47, 0xDA, 0x5E, 0x02, 0x8D, // 92 0A 0E EB FA F8 1B 37 49 7C DD 47 DA 5E 02 8D
    0x96, 0x75, 0x21, 0x27, 0x59, 0x04, 0xAC, 0xB1, 0x0C, 0xB9, 0x23, 0x05, 0xCC, 0x82, 0xB8, 0xBF, // 96 75 21 27 59 04 AC B1 0C B9 23 05 CC 82 B8 BF
    0x04, 0x77, 0x62
};

// This small function creates a new memory BIO and reads the RSA key
RSA *GetPublicKey()
{
    BIO *bp = BIO_new_mem_buf(ServerEncryptionKey, 83);         // Create a new memory buffer BIO
    RSA *pubKey = PEM_read_bio_RSA_PUBKEY(bp, 0, 0, 0);         // And read the RSA key from it
    BIO_free(bp);
    return pubKey;
}

void RSAHash::Initialize()
{
    RSAInternal = GetPublicKey();
}

std::string RSAHash::RSAEncrypt(const std::string &str)
{
    const char* newKey = new char[255];
    RSA_private_encrypt((int)str.length(), (const unsigned char*)str.c_str(), (unsigned char*)newKey, RSAInternal, RSA_NO_PADDING);
    return std::string(newKey);
}

std::string RSAHash::RSADecrypt(const std::string &str)
{
    const char* newKey = new char[255];
    RSA_private_decrypt((int)str.length(), (const unsigned char*)str.c_str(), (unsigned char*)newKey, RSAInternal, RSA_NO_PADDING);
    return std::string(newKey);
}
