// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "crypto.h"

Crypto::Crypto(std::string key)
{
    this->s_key = key;
}

std::string Crypto::decryptToString(const std::string &password)
{
    std::string plain;
    std::string encrypted = password;

    // Generate Cipher, Key, and CBC
    byte key[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];
    StringSource(reinterpret_cast<const char *>(s_key.data()), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)));

    memset(iv, 0x00, AES::BLOCKSIZE);

    try {
        CBC_Mode<AES>::Decryption Decryptor
                (key, sizeof(key), iv);
        StringSource( encrypted, true,
                      new HexDecoder(new StreamTransformationFilter(Decryptor,
                                                                     new StringSink(plain))));
    }
    catch(Exception& e){
        // e.what();
    }

    return plain;
}

std::string Crypto::encryptToString(const std::string& password)
{
    std::string plain = password;
    std::string ciphertext;

    // Generate Cipher, Key, and CBC
    byte key[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];
    StringSource(reinterpret_cast<const char *>(s_key.data()), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)));

    memset(iv, 0x00, AES::BLOCKSIZE);

    try {
        CBC_Mode<AES>::Encryption Encryptor(key, sizeof(key), iv);
        StringSource(plain, true, new StreamTransformationFilter(Encryptor,
                                                  new HexEncoder(new StringSink(ciphertext))));
    }
    catch(Exception& e){
        // e.what();
    }
    return ciphertext;
}
