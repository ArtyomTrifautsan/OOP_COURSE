#include <iostream>
#include <vector>
#include <memory>
#include <sstream>

#include "mode.hpp"


namespace Cryptography
{

    class XOR_ECB_Mode : public IMode
    {
    private:
        std::unique_ptr<IBlockCipher> cipher_;
        const size_t BLOCK_SIZE_;
        
    public:
        XOR_ECB_Mode(std::unique_ptr<IBlockCipher> cipher) 
            : cipher_(std::move(cipher)), BLOCK_SIZE_(cipher_->block_size()) {}
        
        void encrypt(std::istream& data, std::ostream& encrypted) override {
            data.clear();
            data.seekg(0);
            
            std::vector<uint8_t> block(BLOCK_SIZE_);
            
            // Определяем размер данных
            data.seekg(0, std::ios::end);
            size_t total_size = data.tellg();
            data.seekg(0, std::ios::beg);
            
            size_t full_blocks = total_size / BLOCK_SIZE_;
            size_t last_block_size = total_size % BLOCK_SIZE_;
            
            // Обрабатываем полные блоки
            for(size_t i = 0; i < full_blocks; i++) {
                data.read(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                std::stringstream block_stream, encrypted_stream;
                block_stream.write(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                cipher_->encrypt_block(block_stream, encrypted_stream);
                encrypted << encrypted_stream.str();
            }
            
            // Обрабатываем последний неполный блок
            if(last_block_size > 0) {
                block.assign(BLOCK_SIZE_, 0);
                data.read(reinterpret_cast<char*>(block.data()), last_block_size);
                
                // Дополняем последний блок
                uint8_t padding_value = BLOCK_SIZE_ - last_block_size;
                std::fill(block.begin() + last_block_size, block.end(), padding_value);
                
                std::stringstream block_stream, encrypted_stream;
                block_stream.write(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                cipher_->encrypt_block(block_stream, encrypted_stream);
                encrypted << encrypted_stream.str();
            }
        }
        
        void decrypt(std::istream& encrypted, std::ostream& data) override {
            encrypted.clear();
            encrypted.seekg(0);
            
            std::vector<uint8_t> block(BLOCK_SIZE_);
            
            // Определяем размер зашифрованных данных
            encrypted.seekg(0, std::ios::end);
            size_t total_size = encrypted.tellg();
            encrypted.seekg(0, std::ios::beg);
            
            if(total_size % BLOCK_SIZE_ != 0) {
                throw std::runtime_error("Encrypted data size is not multiple of block size");
            }
            
            size_t total_blocks = total_size / BLOCK_SIZE_;
            
            // Обрабатываем все блоки кроме последнего
            for(size_t i = 0; i < total_blocks - 1; i++) {
                encrypted.read(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                std::stringstream block_stream, decrypted_stream;
                block_stream.write(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                cipher_->decrypt_block(block_stream, decrypted_stream);
                data << decrypted_stream.str();
            }
            
            // Обрабатываем последний блок (с удалением дополнения)
            if(total_blocks > 0) {
                encrypted.read(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                std::stringstream block_stream, decrypted_stream;
                block_stream.write(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                cipher_->decrypt_block(block_stream, decrypted_stream);
                
                std::string decrypted = decrypted_stream.str();
                
                // Удаляем дополнение
                if(decrypted.size() == BLOCK_SIZE_) {
                    uint8_t padding_value = static_cast<uint8_t>(decrypted.back());
                    if(padding_value > 0 && padding_value <= BLOCK_SIZE_) {
                        // Проверяем корректность дополнения
                        bool valid_padding = true;
                        for(size_t j = BLOCK_SIZE_ - padding_value; j < BLOCK_SIZE_; j++) {
                            if(decrypted[j] != padding_value) {
                                valid_padding = false;
                                break;
                            }
                        }
                        if(valid_padding) {
                            decrypted.resize(BLOCK_SIZE_ - padding_value);
                        }
                    }
                }
                
                data << decrypted;
            }
        }

        void set_IV(const std::vector<uint8_t>& iv) override {
            // ECB не использует IV
        }

        size_t iv_size() const override { return 0; }
    };


    std::unique_ptr<IMode> create_ecb(std::unique_ptr<IBlockCipher> cipher)
    {
        return std::make_unique<XOR_ECB_Mode>(std::move(cipher));
    }

}