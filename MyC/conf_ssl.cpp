/*******************************************************************************************
*
* Copyright 2016, HiScene
*
* This file is HiScene's property. It contains HiScene's proprietary and confidential
* information and trade secret. The information and code contained in this file is only
* intended for authorized HiScene employees and customers.
*
* DO NOT DISTRIBUTE, DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION FROM
* HISCENE.
*
* If you are not an intended recipient of this file, you must not copy, distribute, modify,
* or take any action in reliance on it. If you have received this file in error, please
* immediately notify HiScene, and permanently delete the original and any copy of it and
* any printout thereof.
*
******************************************************************************************/
#include <assert.h>
#include "conf_ssl.h"
#ifdef _WIN32
#else
#include <libgen.h>         // for dirname
#endif // _WIN32
#include <random>

#ifdef ENABLE_ENCRYPT_CONF

#include <sys/stat.h>

CStreamConfSSL::CStreamConfSSL(const std::string& conf, CStreamConfSSL::mode m)
{
    m_bIsOpen = false;
    m_bAutoDeleteOut = false;

    m_ifTextOrg.open(conf.c_str(), std::ios::in | std::ios::binary);
    if (m_ifTextOrg.is_open()) {
        if (modeDecrypt == m) {
            decrypt();
        }
        else
        {
            encrypt();
        }
    } 
    else 
    {
        fprintf(stderr, "error open %s\n", conf.c_str());
    }
}

CStreamConfSSL::~CStreamConfSSL()
{
    if (m_bAutoDeleteOut) {
        remove(m_strFileOutName.c_str());
    }
}

bool CStreamConfSSL::is_open() const
{
    return m_bIsOpen;
}

bool CStreamConfSSL::save(const std::string& fname, bool autoDelete)
{
    std::ofstream of(fname.c_str());
    if (of.is_open()) {
        char line[1024] = { 0 };
        while (true)
        {
            int outl = std::stringstream::readsome(line, 1024);
            if (outl <= 0) break;
            of.write(line, outl);
        }
    } else {
        fprintf(stderr, "open file %s failed!\n", fname.c_str());
        return false;
    }
    m_bAutoDeleteOut = autoDelete;
    m_strFileOutName = fname;

    return true;
}

void CStreamConfSSL::createKey()
{
    // set key and iv
    for (int i = 0; i < 24; i++)
    {
        key[i] = i;
    }
    for (int i = 0; i < 8; i++)
    {
        iv[i] = i;
    }
}

bool CStreamConfSSL::encrypt()
{
    EVP_CIPHER_CTX ctx;
    unsigned char out[1024];    // buf for ciphering text
    int outl;
    unsigned char in[1024];     // original text
    int inl;
    int rv;
    
    createKey();
    // init. ctx
    EVP_CIPHER_CTX_init(&ctx);

    // set algorithm, key and iv
    rv = EVP_EncryptInit_ex(&ctx, EVP_des_ede3_cbc(), NULL, key, iv);
    if (rv != 1)
    {
        fprintf(stderr, "Error occus in init. context\n");
        return false;
    }

    for (;;)
    {
        inl = m_ifTextOrg.readsome((char*)in, 1024);
        if (inl <= 0) break;    // EOF

        rv = EVP_EncryptUpdate(&ctx, out, &outl, in, inl);
        if (rv != 1)
        {
            EVP_CIPHER_CTX_cleanup(&ctx);
            return false;
        }
        std::stringstream::write((char*)out, outl);
    }

    rv = EVP_EncryptFinal_ex(&ctx, out, &outl);
    if (rv != 1)
    {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    std::stringstream::write((char*)out, outl);

    EVP_CIPHER_CTX_cleanup(&ctx);

    m_bIsOpen = true;
    return true;       // success
}


bool CStreamConfSSL::decrypt()
{
    EVP_CIPHER_CTX ctx;
    unsigned char out[1024 + EVP_MAX_KEY_LENGTH];
    int outl;
    unsigned char in[1024];
    int inl;
    int rv;

    createKey();

    EVP_CIPHER_CTX_init(&ctx);

    rv = EVP_DecryptInit_ex(&ctx, EVP_des_ede3_cbc(), NULL, key, iv);
    if (rv != 1)
    {
        EVP_CIPHER_CTX_cleanup(&ctx);
        fprintf(stderr, "Error occus in init. context\n");
        return false;
    }

    for (;;)
    {
        inl = m_ifTextOrg.readsome((char*)in, 1024);
        if (inl <= 0) break;

        rv = EVP_DecryptUpdate(&ctx, out, &outl, in, inl);
        if (rv != 1)
        {
            EVP_CIPHER_CTX_cleanup(&ctx);
            fprintf(stderr, "Error occus in decrypt\n");
            return false;
        }
        std::stringstream::write((char*)out, outl);
    }

    rv = EVP_DecryptFinal_ex(&ctx, out, &outl);
    if (rv != 1)
    {
        fprintf(stderr, "Error occus in decrypt final\n");
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    std::stringstream::write((char*)out, outl);

    EVP_CIPHER_CTX_cleanup(&ctx);

    m_bIsOpen = true;
    return true;
}

#endif // ENABLE_ENCRYPT_CONF

char *random_uuid(char buf[37])
{
    const char *c = "89ab";
    char *p = buf;
    int n;
    for (n = 0; n < 16; ++n)
    {
        int b = rand() % 255;
        switch (n)
        {
        case 6:
            sprintf(p, "4%x", b % 15);
            break;
        case 8:
            sprintf(p, "%c%x", c[rand() % strlen(c)], b % 15);
            break;
        default:
            sprintf(p, "%02x", b);
            break;
        }

        p += 2;
        switch (n)
        {
        case 3:
        case 5:
        case 7:
        case 9:
            *p++ = '-';
            break;
        }
    }
    *p = 0;
    return buf;
}

std::string get_dirname(const char* path)
{
#ifdef _WIN32
    std::string strPath(path);
    size_t pos = strPath.find_last_of('\\');
    if (std::string::npos == pos ) {
        pos = strPath.find_last_of('/');
    }

    if (std::string::npos == pos) {
        return std::string("");
    }
    return strPath.substr(0, pos);

#else
    char szPath[512] = {0};
    sprintf(szPath, "%s", path);
    return std::string( dirname(szPath) );
#endif // 
}
