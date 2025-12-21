#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>


namespace Cryptography
{

    class EncryptKeyLengthError : public std::runtime_error {
    public:
        explicit EncryptKeyLengthError(const std::string& message) 
            : std::runtime_error(message) {}
    };


    namespace {
        class Padding {
        public:
            static void add_pkcs7(std::vector<uint8_t>& block, size_t current_size, size_t block_size)
            {
                uint8_t padding_val = static_cast<uint8_t>(block_size - current_size);
                for (size_t i = current_size; i < block_size; ++i)
                {
                    block[i] = padding_val;
                }
            }

            static size_t get_pkcs7_count(const std::vector<uint8_t>& last_block)
            {
                if (last_block.empty()) return 0;
                uint8_t padding_val = last_block.back();

                if (padding_val == 0 || padding_val > last_block.size()) return 0;
                return static_cast<size_t>(padding_val);
            }
        };
    }



    //============================================================
    //===                       Ciphers                        ===
    //============================================================

    class IBlockCipher
    {
    public:
        virtual ~IBlockCipher() = default;
        virtual void encrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) = 0;
        virtual void decrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) = 0;
        virtual size_t block_size() const = 0;
        virtual size_t key_size() const = 0;
        virtual void set_key(const std::vector<uint8_t>& key) = 0;
    };

    namespace {
        class XORCipher : public IBlockCipher
        {
            std::vector<uint8_t> m_key;
            size_t m_block_size = 8;
            size_t m_key_size = 8;
            public:
                size_t block_size() const override { return m_block_size; }
                size_t key_size() const override { return m_key_size; }
                
                void set_key(const std::vector<uint8_t>& key) override
                {
                    if (key.size() != key_size())
                    {
                        std::string message = "Ivalid length of the encrypt key.";
                        message += "Key length = ";
                        message += std::to_string(key.size());
                        message += "Expected key length = ";
                        message += std::to_string(key_size());
                        throw EncryptKeyLengthError(message);
                    }

                    m_key = key;
                    // if (m_key.size() < block_size()) m_key.resize(block_size(), 0x00);
                }

                void encrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) override
                {
                    for (size_t i = 0; i < block_size(); ++i) out[i] = in[i] ^ m_key[i];
                }

                void decrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) override
                {
                    encrypt_block(in, out);
                }
        };

        class TEACipher : public IBlockCipher
        {
            std::vector<uint32_t> m_key; // Ключ из 4-х 32-битных чисел (128 бит)
            const uint32_t DELTA = 0x9e3779b9;
            const size_t ROUNDS = 32;
            size_t m_block_size = 8;
            size_t m_key_size = 16;

        public:
            TEACipher()
            {
                m_key.assign(4, 0);
            }

            size_t block_size() const override { return m_block_size; }
            size_t key_size() const override { return m_key_size; }

            void set_key(const std::vector<uint8_t>& key) override
            {
                // TEA требует 16 байт ключа.
                if (key.size() != key_size())
                {
                    std::string message = "Ivalid length of the encrypt key.";
                    message += "Key length = ";
                    message += std::to_string(key.size());
                    message += "Expected key length = ";
                    message += std::to_string(key_size());
                    throw EncryptKeyLengthError(message);
                }

                std::vector<uint8_t> k = key;
                k.resize(16, 0);

                m_key.assign(4, 0);
                for (int i = 0; i < 4; ++i) {
                    // Упаковываем 4 байта в одно 32-битное число
                    m_key[i] = (static_cast<uint32_t>(k[i * 4]) << 24) |
                            (static_cast<uint32_t>(k[i * 4 + 1]) << 16) |
                            (static_cast<uint32_t>(k[i * 4 + 2]) << 8) |
                            (static_cast<uint32_t>(k[i * 4 + 3]));
                }
            }

            void encrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) override
            {
                uint32_t v0 = (static_cast<uint32_t>(in[0]) << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
                uint32_t v1 = (static_cast<uint32_t>(in[4]) << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
                uint32_t sum = 0;

                for (size_t i = 0; i < ROUNDS; i++)
                {
                    sum += DELTA;
                    v0 += ((v1 << 4) + m_key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + m_key[1]);
                    v1 += ((v0 << 4) + m_key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + m_key[3]);
                }

                unpack(v0, v1, out);
            }

            void decrypt_block(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) override
            {
                uint32_t v0 = (static_cast<uint32_t>(in[0]) << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
                uint32_t v1 = (static_cast<uint32_t>(in[4]) << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
                uint32_t sum = DELTA * ROUNDS; // Начальная сумма для обратного цикла

                for (size_t i = 0; i < ROUNDS; i++)
                {
                    v1 -= ((v0 << 4) + m_key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + m_key[3]);
                    v0 -= ((v1 << 4) + m_key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + m_key[1]);
                    sum -= DELTA;
                }

                unpack(v0, v1, out);
            }

        private:
            void unpack(uint32_t v0, uint32_t v1, std::vector<uint8_t>& out)
            {
                out[0] = (v0 >> 24) & 0xFF; out[1] = (v0 >> 16) & 0xFF;
                out[2] = (v0 >> 8) & 0xFF;  out[3] = v0 & 0xFF;
                out[4] = (v1 >> 24) & 0xFF; out[5] = (v1 >> 16) & 0xFF;
                out[6] = (v1 >> 8) & 0xFF;  out[7] = v1 & 0xFF;
            }
        };
    }


    enum class CipherType {
        XOR,
        TEA
    };

    std::shared_ptr<IBlockCipher> cipher_factory(CipherType type)
    {
        switch (type)
        {
        case CipherType::XOR:
            return std::make_shared<XORCipher>();
            break;

        case CipherType::TEA:
            return std::make_shared<TEACipher>();

        default:
            return nullptr;
            break;
        }
    }



    //============================================================
    //===                        Modes                         ===
    //============================================================

    class IMode
    {
    protected:
        std::shared_ptr<IBlockCipher> m_cipher;

    public:
        IMode(std::shared_ptr<IBlockCipher> cipher) : m_cipher(cipher) {}
        virtual ~IMode() = default;
    
        virtual void encrypt(std::istream& data, std::ostream& encrypted) = 0;
        virtual void decrypt(std::istream& encrypted, std::ostream& data) = 0;
    };


    namespace {
        class ECBMode : public IMode
        {
        public:
            using IMode::IMode;

            void encrypt(std::istream& is, std::ostream& os) override
            {
                size_t bsize = m_cipher->block_size();
                std::vector<uint8_t> buffer(bsize), output(bsize);

                while (is.read(reinterpret_cast<char*>(buffer.data()), bsize))
                {
                    m_cipher->encrypt_block(buffer, output);
                    os.write(reinterpret_cast<char*>(output.data()), bsize);
                }

                // Обработка последнего блока (Padding всегда добавляется)
                size_t read = is.gcount();
                Padding::add_pkcs7(buffer, read, bsize);
                m_cipher->encrypt_block(buffer, output);
                os.write(reinterpret_cast<char*>(output.data()), bsize);
            }

            void decrypt(std::istream& is, std::ostream& os) override
            {
                size_t bsize = m_cipher->block_size();
                std::vector<uint8_t> buffer(bsize), output(bsize);
                std::vector<uint8_t> prev_output;

                while (is.read(reinterpret_cast<char*>(buffer.data()), bsize))
                {
                    if (!prev_output.empty())
                    {
                        os.write(reinterpret_cast<char*>(prev_output.data()), bsize);
                    }
                    prev_output.resize(bsize);
                    m_cipher->decrypt_block(buffer, prev_output);
                }

                if (!prev_output.empty())
                {
                    size_t pad_count = Padding::get_pkcs7_count(prev_output);
                    if (pad_count < bsize)
                    {
                        os.write(reinterpret_cast<char*>(prev_output.data()), bsize - pad_count);
                    }
                }
            }
        };

        class CBCMode : public IMode
        {
            std::vector<uint8_t> m_iv; // Вектор инициализации

        public:
            CBCMode(std::shared_ptr<IBlockCipher> cipher, std::vector<uint8_t> iv) 
                : IMode(cipher), m_iv(iv)
            {
                m_iv.resize(m_cipher->block_size(), 0);
            }

            void encrypt(std::istream& is, std::ostream& os) override
            {
                size_t bsize = m_cipher->block_size();
                std::vector<uint8_t> buffer(bsize), output(bsize), prev_block = m_iv;

                while (is.read(reinterpret_cast<char*>(buffer.data()), bsize))
                {
                    for(size_t i = 0; i < bsize; ++i) buffer[i] ^= prev_block[i];
                    m_cipher->encrypt_block(buffer, output);
                    os.write(reinterpret_cast<char*>(output.data()), bsize);
                    prev_block = output;
                }

                size_t remaining = is.gcount(); 
                Padding::add_pkcs7(buffer, remaining, bsize);
                
                for(size_t i=0; i<bsize; ++i) buffer[i] ^= prev_block[i];
                m_cipher->encrypt_block(buffer, output);
                os.write(reinterpret_cast<char*>(output.data()), bsize);
            }

            void decrypt(std::istream& is, std::ostream& os) override
            {
                size_t bsize = m_cipher->block_size();
                std::vector<uint8_t> buffer(bsize), output(bsize), prev_xor_block = m_iv;
                
                // Нам нужно буферизировать вывод, чтобы не записать паддинг из последнего блока
                std::vector<uint8_t> last_decrypted_block;

                while (is.read(reinterpret_cast<char*>(buffer.data()), bsize))
                {
                    if (!last_decrypted_block.empty())
                    {
                        os.write(reinterpret_cast<char*>(last_decrypted_block.data()), bsize);
                    }
                    
                    last_decrypted_block.resize(bsize);
                    m_cipher->decrypt_block(buffer, last_decrypted_block);
                    
                    for(size_t i = 0; i < bsize; ++i)
                    {
                        last_decrypted_block[i] ^= prev_xor_block[i];
                    }
                    prev_xor_block = buffer;
                }

                // Убираем паддинг из самого последнего блока
                if (!last_decrypted_block.empty())
                {
                    size_t pad_count = Padding::get_pkcs7_count(last_decrypted_block);
                    if (pad_count <= bsize)
                    {
                        os.write(reinterpret_cast<char*>(last_decrypted_block.data()), bsize - pad_count);
                    }
                }
            }
        };
    }


    enum class ModeType {
        ECB,
        CBC
    };

    template <typename... Argv>
    std::shared_ptr<IMode> mode_factory(ModeType type, Argv&&... args)
    {
        switch (type)
        {
        case ModeType::ECB:
            if constexpr (std::is_constructible_v<ECBMode, Argv...>)
                return std::make_shared<ECBMode>(std::forward<Argv>(args)...);
            else
                return nullptr; 

        case ModeType::CBC:
            if constexpr (std::is_constructible_v<CBCMode, Argv...>)
                return std::make_shared<CBCMode>(std::forward<Argv>(args)...);
            else
                return nullptr;

        default:
            return nullptr;
        }
    }

    



    //============================================================
    //===                        Coder                         ===
    //============================================================
    class Coder
    {
        std::shared_ptr<IMode> m_mode;
    public:
        void set_strategy(std::shared_ptr<IMode> mode)
        {
            m_mode = std::move(mode);
        }

        void encrypt(std::istream& data, std::ostream& encrypted)
        {
            if (m_mode) m_mode->encrypt(data, encrypted);
        }

        void decrypt(std::istream& encrypted, std::ostream& data)
        {
            if (m_mode) m_mode->decrypt(encrypted, data);
        }
    };
}