#pragma once

#include <iostream>
#include <vector>
#include <memory>


namespace Cryptography
{

    class IBlockCipher
    {
    public:
        virtual ~IBlockCipher() = default;
    
        virtual void encrypt_block(std::istream& data, std::ostream& encrypted) = 0;

        virtual void decrypt_block(std::istream& encrypted, std::ostream& data) = 0;

        virtual size_t block_size() const = 0;

        virtual size_t key_size() const = 0;

        virtual void set_key(const std::vector<uint8_t>& key) = 0;
    };


    enum class CipherType {
        XORCipher
    };


    std::unique_ptr<IBlockCipher> create_xor_cipher();


    class CipherFactory
    {
    public:
        static std::unique_ptr<IBlockCipher> create_cipher(CipherType type)
        {
            switch(type) {
                case CipherType::XORCipher:
                    return create_xor_cipher();
                default:
                    throw std::invalid_argument("Unknown cipher type");
            }
        }
    };

}