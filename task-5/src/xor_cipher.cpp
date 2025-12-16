#include <array>
#include <cstring>
#include <algorithm>

#include "block_cipher.hpp"


namespace Cryptography
{

    class XORCipher : public IBlockCipher
    {
    private:
        std::vector<uint8_t> key_;
        const size_t BLOCK_SIZE_ = 16;
        const size_t KEY_SIZE_ = 16;
        
    public:
        XORCipher() = default;
        
        void set_key(const std::vector<uint8_t>& key) override {
            if(key.size() != key_size()) {
                throw std::invalid_argument("XORCipher: key must be 16 bytes (128 bits)");
            }
            key_ = key;
        }
        
        size_t block_size() const override { return BLOCK_SIZE_; }
        size_t key_size() const override { return KEY_SIZE_; }
        
        void encrypt_block(std::istream& data, std::ostream& encrypted) override {
            std::vector<uint8_t> block(block_size());
            data.read(reinterpret_cast<char*>(block.data()), block_size());
            
            size_t bytes_read = data.gcount();
            if(bytes_read == 0) return;
            
            // Если прочитано меньше, чем размер блока, дополнять НЕ нужно
            // Дополнение должно быть сделано до вызова encrypt_block
            
            // Простое XOR-шифрование
            for(size_t i = 0; i < bytes_read; i++) {
                block[i] ^= key_[i % key_size()];
            }
            
            // Записываем столько, сколько прочитали
            encrypted.write(reinterpret_cast<char*>(block.data()), bytes_read);
        }
        
        void decrypt_block(std::istream& encrypted, std::ostream& data) override {
            std::vector<uint8_t> block(block_size());
            encrypted.read(reinterpret_cast<char*>(block.data()), block_size());
            
            size_t bytes_read = encrypted.gcount();
            if(bytes_read == 0) return;
            
            // Дешифрование - обратный XOR
            for(size_t i = 0; i < bytes_read; i++) {
                block[i] ^= key_[i % key_size()];
            }
            
            // Записываем столько, сколько прочитали
            data.write(reinterpret_cast<char*>(block.data()), bytes_read);
        }
    };


    std::unique_ptr<IBlockCipher> create_xor_cipher()
    {
        return std::make_unique<XORCipher>();
    }

}