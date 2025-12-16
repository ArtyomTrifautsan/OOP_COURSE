#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "block_cipher.hpp"


namespace Cryptography
{

    class IMode
    {
    public:
        virtual ~IMode() = default;
    
        virtual void encrypt(std::istream& data, std::ostream& encrypted) = 0;

        virtual void decrypt(std::istream& encrypted, std::ostream& data) = 0;

        virtual size_t iv_size() const = 0;
        
        virtual void set_IV(const std::vector<uint8_t>& iv) = 0;
    };


    enum class ModeType {
        ECB,
        CBC
    };


    std::unique_ptr<IMode> create_ecb(std::unique_ptr<IBlockCipher> cipher);

    std::unique_ptr<IMode> create_cbc(std::unique_ptr<IBlockCipher> cipher);


    class ModeFactory
    {
    public:
        static std::unique_ptr<IMode> create_mode(ModeType type, std::unique_ptr<IBlockCipher> cipher)
        {
            switch(type) {
                case ModeType::ECB:
                    return create_ecb(std::move(cipher));
                case ModeType::CBC:
                    return create_cbc(std::move(cipher));
                default:
                    throw std::invalid_argument("Unknown mode type");
            }
        }
    };

}