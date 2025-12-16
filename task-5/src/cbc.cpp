#include <iostream>
#include <vector>
#include <memory>
#include <sstream>

#include "mode.hpp"


namespace Cryptography
{

    class XOR_CBC_Mode : public IMode
    {
    private:
        std::unique_ptr<IBlockCipher> cipher_;
        std::vector<uint8_t> iv_;
        const size_t BLOCK_SIZE_;
        bool is_last_block_ = false;
        
        void xor_vectors(std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
            for(size_t i = 0; i < a.size(); i++) {
                a[i] ^= b[i];
            }
        }

        void remove_padding(std::vector<uint8_t>& block) {
            if (is_last_block_) {
                uint8_t padding_value = block.back();
                if(padding_value > 0 && padding_value <= BLOCK_SIZE_) {
                    // Проверяем, что все байты дополнения корректны
                    bool valid_padding = true;
                    for(size_t i = BLOCK_SIZE_ - padding_value; i < BLOCK_SIZE_; i++) {
                        if(block[i] != padding_value) {
                            valid_padding = false;
                            break;
                        }
                    }
                    if(valid_padding) {
                        block.resize(BLOCK_SIZE_ - padding_value);
                    }
                }
            }
        }

    public:
        XOR_CBC_Mode(std::unique_ptr<IBlockCipher> cipher) 
            : cipher_(std::move(cipher)), BLOCK_SIZE_(cipher_->block_size()) {}
        
        size_t iv_size() const override { return BLOCK_SIZE_; }
        
        void set_IV(const std::vector<uint8_t>& iv) override {
            if(iv.size() != iv_size()) {
                throw std::invalid_argument("Invalid IV size");
            }
            iv_ = iv;
        }
        
        void encrypt(std::istream& data, std::ostream& encrypted) override {
            data.clear();
            data.seekg(0);
            
            std::vector<uint8_t> current_iv = iv_;
            std::vector<uint8_t> block(BLOCK_SIZE_);
            
            while(true) {
                data.read(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                size_t bytes_read = data.gcount();
                
                if(bytes_read == 0) break;
                
                is_last_block_ = (bytes_read < BLOCK_SIZE_) || data.eof();
                
                // Дополняем только последний блок
                if(is_last_block_ && bytes_read < BLOCK_SIZE_) {
                    uint8_t padding_value = BLOCK_SIZE_ - bytes_read;
                    std::fill(block.begin() + bytes_read, block.end(), padding_value);
                }
                
                // CBC: XOR с IV/предыдущим зашифрованным блоком
                xor_vectors(block, current_iv);
                
                // Шифруем
                std::stringstream block_stream, encrypted_stream;
                block_stream.write(reinterpret_cast<char*>(block.data()), BLOCK_SIZE_);
                cipher_->encrypt_block(block_stream, encrypted_stream);
                
                std::string encrypted_block = encrypted_stream.str();
                encrypted.write(encrypted_block.data(), encrypted_block.size());
                
                // Обновляем IV для следующего блока
                current_iv.assign(encrypted_block.begin(), encrypted_block.end());
                
                if(is_last_block_) break;
            }
        }
        
        void decrypt(std::istream& encrypted, std::ostream& data) override {
            encrypted.clear();
            encrypted.seekg(0);
            
            std::vector<uint8_t> current_iv = iv_;
            std::vector<uint8_t> encrypted_block(BLOCK_SIZE_);
            
            // Определяем, сколько всего блоков
            encrypted.seekg(0, std::ios::end);
            size_t total_size = encrypted.tellg();
            encrypted.seekg(0, std::ios::beg);
            
            size_t total_blocks = total_size / BLOCK_SIZE_;
            if (total_size % BLOCK_SIZE_ != 0) {
                throw std::runtime_error("Encrypted data size is not multiple of block size");
            }
            
            for(size_t block_num = 0; block_num < total_blocks; block_num++) {
                encrypted.read(reinterpret_cast<char*>(encrypted_block.data()), BLOCK_SIZE_);
                size_t bytes_read = encrypted.gcount();
                
                if(bytes_read != BLOCK_SIZE_) {
                    throw std::runtime_error("Failed to read complete block");
                }
                
                is_last_block_ = (block_num == total_blocks - 1);
                
                // Сохраняем зашифрованный блок для следующей итерации
                std::vector<uint8_t> saved_block = encrypted_block;
                
                // Дешифруем
                std::stringstream encrypted_stream, decrypted_stream;
                encrypted_stream.write(reinterpret_cast<char*>(encrypted_block.data()), BLOCK_SIZE_);
                cipher_->decrypt_block(encrypted_stream, decrypted_stream);
                
                std::string decrypted_str = decrypted_stream.str();
                if(decrypted_str.size() != BLOCK_SIZE_) {
                    throw std::runtime_error("Decryption failed: wrong block size");
                }
                
                std::vector<uint8_t> temp_block(decrypted_str.begin(), decrypted_str.end());
                
                // CBC: XOR с IV/предыдущим зашифрованным блоком
                xor_vectors(temp_block, current_iv);
                
                // Обновляем IV
                current_iv = saved_block;
                
                // Удаляем дополнение для последнего блока
                if (is_last_block_) {
                    remove_padding(temp_block);
                }
                
                // Записываем данные
                data.write(reinterpret_cast<char*>(temp_block.data()), temp_block.size());
            }
        }
    };


    std::unique_ptr<IMode> create_cbc(std::unique_ptr<IBlockCipher> cipher)
    {
        return std::make_unique<XOR_CBC_Mode>(std::move(cipher));
    }

}