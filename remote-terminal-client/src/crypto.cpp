// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "crypto.h"

Crypto::Crypto(std::string key)
{
    this->cryptoKey = key;
}

std::string Crypto::DecryptString(const std::string &encryptedText)
{
    if(encryptedText.length() == 0)
        return "";

    std::string plainText;

    byte key[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];

    memset(iv, 0x00, AES::BLOCKSIZE);
    memset(key, 0x00, AES::MAX_KEYLENGTH);

    // Generate Cipher, Key, and GCM
    StringSource(reinterpret_cast<const char *>(this->cryptoKey.data()), true,
                  new HashFilter(*(new SHA512), new ArraySink(key, AES::MAX_KEYLENGTH)));

    try
    {
        GCM<AES>::Decryption Decryptor;
        Decryptor.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
        StringSource(encryptedText, true,
                      new HexDecoder(new AuthenticatedDecryptionFilter(Decryptor, new StringSink(plainText))));
    }
    catch(CryptoPP::Exception & e)
    {
        qDebug() << e.what();
    }
    return plainText;
}

std::string Crypto::EncryptString(const std::string& plainText)
{
    if(plainText.length() == 0)
        return "";

    std::string cipherText;

    byte key[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];

    memset(iv, 0x00, AES::BLOCKSIZE);
    memset(key, 0x00, AES::MAX_KEYLENGTH);

    // Generate Cipher, Key, and GCM
    StringSource(reinterpret_cast<const char *>(this->cryptoKey.data()), true,
                  new HashFilter(*(new SHA512), new ArraySink(key, AES::MAX_KEYLENGTH)));
    try
    {
        GCM<AES>::Encryption Encryptor;
        Encryptor.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
        StringSource(plainText, true, new AuthenticatedEncryptionFilter(Encryptor,
                                                  new HexEncoder(new StringSink(cipherText))));
    }
    catch(CryptoPP::Exception& e)
    {
        qDebug() << e.what();
    }
    return cipherText;
}
