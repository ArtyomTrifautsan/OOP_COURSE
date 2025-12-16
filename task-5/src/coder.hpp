#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <iterator>


#include "mode.hpp"
#include <iomanip>



namespace Cryptography
{

    // class Coder
    // {
    // public:
    //     Coder(CipherType cipher, ModeType mode);

    //     Coder(std::unique_ptr<IBlockCipher> cipher, ModeType mode);

    //     void encrypt(std::istream& data, std::ostream& encrypted);

    //     void decrypt(std::istream& encrypted, std::ostream& data);

    //     void set_key(const std::vector<uint8_t>& key);
    //     void set_iv(const std::vector<uint8_t>& iv);
        
    //     size_t block_size() const;
    //     size_t iv_size() const;
    // };



    class Coder
{
private:
    std::unique_ptr<IMode> mode_;
    
public:
    // Конструктор с генерацией ключа и IV
    Coder(CipherType cipher_type, ModeType mode_type, 
          const std::vector<uint8_t>& key,
          const std::vector<uint8_t>& iv = {}) {
        
        auto cipher = CipherFactory::create_cipher(cipher_type);
        cipher->set_key(key);
        mode_ = ModeFactory::create_mode(mode_type, std::move(cipher));
        
        if (!iv.empty()) {
            mode_->set_IV(iv);
        }
    }
    
    // Конструктор с существующим шифром
    Coder(std::unique_ptr<IBlockCipher> cipher, ModeType mode_type,
          const std::vector<uint8_t>& iv = {}) {
        
        mode_ = ModeFactory::create_mode(mode_type, std::move(cipher));
        
        if (!iv.empty()) {
            mode_->set_IV(iv);
        }
    }
    
    void encrypt(std::istream& data, std::ostream& encrypted) {
        if (!mode_) {
            throw std::runtime_error("Coder not properly initialized");
        }
        mode_->encrypt(data, encrypted);
    }
    
    void decrypt(std::istream& encrypted, std::ostream& data) {
        if (!mode_) {
            throw std::runtime_error("Coder not properly initialized");
        }
        mode_->decrypt(encrypted, data);
    }
    
    size_t block_size() const {
        return 16;  // Для XORCipher всегда 16
    }
    
    size_t key_size() const {
        return 16;  // Для XORCipher всегда 16
    }
    
    size_t iv_size() const {
        return mode_ ? mode_->iv_size() : 0;
    }
    
    // Генерация случайного ключа
    static std::vector<uint8_t> generate_key(size_t size) {
        std::vector<uint8_t> key(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for(auto& byte : key) {
            byte = static_cast<uint8_t>(dis(gen));
        }
        return key;
    }
    
    // Генерация случайного IV
    static std::vector<uint8_t> generate_iv(size_t size) {
        return generate_key(size);
    }
    
    // Утилита для преобразования строки в байты
    static std::vector<uint8_t> string_to_bytes(const std::string& str) {
        return std::vector<uint8_t>(str.begin(), str.end());
    }
    
    // Утилита для преобразования байтов в строку
    static std::string bytes_to_string(const std::vector<uint8_t>& bytes) {
        return std::string(bytes.begin(), bytes.end());
    }
    
    // Утилита для печати байтов в hex
    static void print_hex(const std::vector<uint8_t>& bytes, const std::string& label = "") {
        if(!label.empty()) {
            std::cout << label << ": ";
        }
        for(uint8_t byte : bytes) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
    }
};

}