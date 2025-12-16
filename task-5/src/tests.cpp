#include <gtest/gtest.h>
#include "coder.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>

// #include "your_cryptography_headers.hpp" // Замените на ваши заголовки

using namespace Cryptography;

// ============================================
// ТЕСТЫ ДЛЯ XORCipher (блочный шифр)
// ============================================

TEST(XORCipherTest, Creation) {
    std::unique_ptr<IBlockCipher> cipher = CipherFactory::create_cipher(CipherType::XORCipher);

    EXPECT_EQ(cipher->block_size(), 16);
    EXPECT_EQ(cipher->key_size(), 16);

    // Проверка установки ключа
    std::vector<uint8_t> key(16, 0x01);
    EXPECT_NO_THROW(cipher->set_key(key));

    // Проверка исключения при неправильном размере ключа
    std::vector<uint8_t> wrong_key(8, 0x01);
    EXPECT_THROW(cipher->set_key(wrong_key), std::invalid_argument);
}
// Тесты для XORCipher
TEST(XORCipherTest, BasicEncryptionDecryption) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0xAA);  // Ключ: все байты 0xAA
    cipher->set_key(key);

    std::string test_data = "Hello, World!123";  // Ровно 16 байт
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    // Шифрование
    cipher->encrypt_block(input, encrypted);
    
    // Проверка, что данные зашифровались
    std::string encrypted_str = encrypted.str();
    EXPECT_EQ(encrypted_str.size(), 16);
    EXPECT_NE(encrypted_str, test_data);

    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    cipher->decrypt_block(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}
TEST(XORCipherTest, PaddingShortBlock) {
    // Теперь XORCipher не занимается дополнением, используем режим
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0x55);
    cipher->set_key(key);

    // Используем ECB режим для тестирования дополнения
    auto ecb = ModeFactory::create_mode(ModeType::ECB, std::move(cipher));
    
    std::string test_data = "Short";  // 5 байт
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    ecb->encrypt(input, encrypted);
    
    // В режиме ECB с дополнением должно быть 16 байт
    EXPECT_EQ(encrypted.str().size(), 16);

    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    ecb->decrypt(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}

TEST(XORCipherTest, DoubleXorReturnsOriginal) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key = Coder::generate_key(16);
    cipher->set_key(key);

    // Используем ECB режим
    auto ecb = ModeFactory::create_mode(ModeType::ECB, std::move(cipher));
    
    std::string test_data = "Test data for XOR!";
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    ecb->encrypt(input, encrypted);
    
    encrypted.clear();
    encrypted.seekg(0);
    ecb->decrypt(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}

// Добавим тест для прямого использования XORCipher (без дополнения)
TEST(XORCipherTest, DirectEncryptionWithoutPadding) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0xAA);
    cipher->set_key(key);

    // Тестируем шифр напрямую с полным блоком
    std::string test_data(16, 'X');  // Ровно 16 байт
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    // Шифрование
    cipher->encrypt_block(input, encrypted);
    
    // Должно быть 16 байт
    EXPECT_EQ(encrypted.str().size(), 16);
    EXPECT_NE(encrypted.str(), test_data);

    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    cipher->decrypt_block(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}

// Тесты для ECB режима
TEST(ECBModeTest, MultiBlockEncryption) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0xFF);
    cipher->set_key(key);

    auto ecb = ModeFactory::create_mode(ModeType::ECB, std::move(cipher));

    // Данные больше одного блока
    std::string test_data(32, 'A');  // 32 байта = 2 блока
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    ecb->encrypt(input, encrypted);
    
    // Проверка размера зашифрованных данных
    EXPECT_EQ(encrypted.str().size(), 32);  // Должно быть 2 блока по 16 байт

    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    ecb->decrypt(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}

TEST(ECBModeTest, SameBlocksProduceSameOutput) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key = Coder::generate_key(16);
    cipher->set_key(key);

    auto ecb = ModeFactory::create_mode(ModeType::ECB, std::move(cipher));

    // Два одинаковых блока
    std::string block(16, 'X');
    std::string test_data = block + block;  // Два одинаковых блока
    std::stringstream input, encrypted;
    input << test_data;

    ecb->encrypt(input, encrypted);
    std::string encrypted_str = encrypted.str();
    
    // В ECB одинаковые блоки дают одинаковый зашифрованный текст
    EXPECT_EQ(encrypted_str.substr(0, 16), encrypted_str.substr(16, 16));
}

// Тесты для CBC режима
TEST(CBCModeTest, BasicCBCEncryption) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0x11);
    cipher->set_key(key);

    auto cbc = ModeFactory::create_mode(ModeType::CBC, std::move(cipher));
    
    // Установка IV
    std::vector<uint8_t> iv(16, 0x22);
    cbc->set_IV(iv);

    std::string test_data = "CBC Mode Test Data!";  // 19 байт
    std::stringstream input, encrypted, decrypted;
    input << test_data;

    cbc->encrypt(input, encrypted);
    
    // 19 байт + 13 байт дополнения = 32 байта (2 блока)
    EXPECT_EQ(encrypted.str().size(), 32);

    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    cbc->decrypt(encrypted, decrypted);
    
    EXPECT_EQ(decrypted.str(), test_data);
}

TEST(CBCModeTest, DifferentIVProducesDifferentCiphertext) {
    std::string test_data = "Same plaintext";
    
    // Создаем два шифратора с разными IV
    {
        auto cipher1 = CipherFactory::create_cipher(CipherType::XORCipher);
        cipher1->set_key(std::vector<uint8_t>(16, 0x33));
        auto cbc1 = ModeFactory::create_mode(ModeType::CBC, std::move(cipher1));
        cbc1->set_IV(std::vector<uint8_t>(16, 0x01));
        
        std::stringstream input1, encrypted1;
        input1 << test_data;
        cbc1->encrypt(input1, encrypted1);
        
        // Второй шифратор
        auto cipher2 = CipherFactory::create_cipher(CipherType::XORCipher);
        cipher2->set_key(std::vector<uint8_t>(16, 0x33));  // Тот же ключ
        auto cbc2 = ModeFactory::create_mode(ModeType::CBC, std::move(cipher2));
        cbc2->set_IV(std::vector<uint8_t>(16, 0x02));  // Другой IV
        
        std::stringstream input2, encrypted2;
        input2 << test_data;
        cbc2->encrypt(input2, encrypted2);
        
        // Разные IV должны давать разный зашифрованный текст
        EXPECT_NE(encrypted1.str(), encrypted2.str());
        
        // Проверяем, что оба можно расшифровать
        encrypted1.clear();
        encrypted1.seekg(0);
        std::stringstream decrypted1;
        cbc1->decrypt(encrypted1, decrypted1);
        EXPECT_EQ(decrypted1.str(), test_data);
    }
}

TEST(CBCModeTest, InvalidIVSizeThrows) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    auto cbc = ModeFactory::create_mode(ModeType::CBC, std::move(cipher));
    
    // Неправильный размер IV
    std::vector<uint8_t> wrong_iv(8, 0x00);
    EXPECT_THROW(cbc->set_IV(wrong_iv), std::invalid_argument);
}

// Тесты для Coder класса
TEST(CoderTest, CompleteWorkflow) {
    // Генерация ключа и IV
    auto key = Coder::generate_key(16);
    auto iv = Coder::generate_iv(16);
    
    // Создаем кодер с ключом и IV
    Coder coder(CipherType::XORCipher, ModeType::CBC, key, iv);
    
    std::string original_text = "This is a secret message that needs to be encrypted!";
    std::stringstream input, encrypted, decrypted;
    input << original_text;
    
    // Шифрование
    EXPECT_NO_THROW(coder.encrypt(input, encrypted));
    
    std::string encrypted_str = encrypted.str();
    EXPECT_GT(encrypted_str.size(), 0);
    EXPECT_NE(encrypted_str, original_text);
    
    // Проверяем, что размер кратен 16
    EXPECT_EQ(encrypted_str.size() % 16, 0);
    
    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    EXPECT_NO_THROW(coder.decrypt(encrypted, decrypted));
    
    EXPECT_EQ(decrypted.str(), original_text);
}

TEST(CoderTest, DifferentModesComparison) {
    auto key = Coder::generate_key(16);
    auto iv = Coder::generate_iv(16);
    
    std::string test_data = "AAAABBBBAAAABBBB";  // 16 байт (ровно 1 блок)
    
    // ECB режим (не требует IV)
    {
        Coder ecb_coder(CipherType::XORCipher, ModeType::ECB, key);
        
        std::stringstream input1, encrypted1;
        input1 << test_data;
        ecb_coder.encrypt(input1, encrypted1);
        
        std::string ecb_result = encrypted1.str();
        EXPECT_EQ(ecb_result.size() % 16, 0);  // Должно быть кратно 16
    }
    
    // CBC режим (требует IV)
    {
        Coder cbc_coder(CipherType::XORCipher, ModeType::CBC, key, iv);
        
        std::stringstream input2, encrypted2;
        input2 << test_data;
        cbc_coder.encrypt(input2, encrypted2);
        
        std::string cbc_result = encrypted2.str();
        EXPECT_EQ(cbc_result.size() % 16, 0);  // Должно быть кратно 16
    }
}

TEST(CoderTest, UtilityFunctions) {
    // Тест преобразования строк в байты и обратно
    std::string test_str = "Hello, World!";
    auto bytes = Coder::string_to_bytes(test_str);
    std::string restored = Coder::bytes_to_string(bytes);
    EXPECT_EQ(test_str, restored);
    
    // Тест генерации ключа
    auto key1 = Coder::generate_key(16);
    auto key2 = Coder::generate_key(16);
    EXPECT_EQ(key1.size(), 16);
    EXPECT_EQ(key2.size(), 16);
    
    // С высокой вероятностью ключи должны быть разными
    EXPECT_NE(key1, key2);
    
    // Тест генерации IV
    auto iv = Coder::generate_iv(16);
    EXPECT_EQ(iv.size(), 16);
}

TEST(CoderTest, ConstructorWithExistingCipher) {
    auto cipher = CipherFactory::create_cipher(CipherType::XORCipher);
    std::vector<uint8_t> key(16, 0x99);
    cipher->set_key(key);
    
    auto iv = Coder::generate_iv(16);
    
    // Использование конструктора с существующим шифром
    Coder coder(std::move(cipher), ModeType::CBC, iv);
    
    EXPECT_EQ(coder.block_size(), 16);
    EXPECT_EQ(coder.key_size(), 16);
    EXPECT_EQ(coder.iv_size(), 16);
    
    // Тестируем работу
    std::string test_data = "Test message";
    std::stringstream input, encrypted, decrypted;
    input << test_data;
    
    EXPECT_NO_THROW(coder.encrypt(input, encrypted));
    encrypted.clear();
    encrypted.seekg(0);
    EXPECT_NO_THROW(coder.decrypt(encrypted, decrypted));
    
    EXPECT_EQ(decrypted.str(), test_data);
}

TEST(CoderTest, LargeDataEncryption) {
    auto key = Coder::generate_key(16);
    auto iv = Coder::generate_iv(16);
    
    Coder coder(CipherType::XORCipher, ModeType::CBC, key, iv);
    
    // Большие данные (10 блоков + 5 байт)
    std::string large_data(165, 'X');  // 165 байт = 10 блоков + 5 байт
    std::stringstream input, encrypted, decrypted;
    input << large_data;
    
    EXPECT_NO_THROW(coder.encrypt(input, encrypted));
    
    // Проверка размера (должен быть кратен 16)
    EXPECT_EQ(encrypted.str().size() % 16, 0);
    
    encrypted.clear();
    encrypted.seekg(0);
    EXPECT_NO_THROW(coder.decrypt(encrypted, decrypted));
    
    EXPECT_EQ(decrypted.str(), large_data);
}

TEST(CoderTest, ResetStreams) {
    auto key = Coder::generate_key(16);
    
    Coder coder(CipherType::XORCipher, ModeType::ECB, key);
    
    std::string test_data = "Test data";
    std::stringstream input, encrypted1, encrypted2;
    input << test_data;
    
    // Первое шифрование
    EXPECT_NO_THROW(coder.encrypt(input, encrypted1));
    
    // Сброс потока и повторное шифрование
    input.clear();
    input.seekg(0);
    EXPECT_NO_THROW(coder.encrypt(input, encrypted2));
    
    // Результаты должны быть одинаковыми
    EXPECT_EQ(encrypted1.str(), encrypted2.str());
}

TEST(CoderTest, EmptyDataEncryption) {
    auto key = Coder::generate_key(16);
    auto iv = Coder::generate_iv(16);
    
    Coder coder(CipherType::XORCipher, ModeType::CBC, key, iv);
    
    std::string empty_data = "";
    std::stringstream input, encrypted, decrypted;
    input << empty_data;
    
    // Шифрование пустых данных
    EXPECT_NO_THROW(coder.encrypt(input, encrypted));
    
    // Для пустых данных с дополнением должен получиться 1 блок
    EXPECT_EQ(encrypted.str().size(), 16);
    
    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    EXPECT_NO_THROW(coder.decrypt(encrypted, decrypted));
    
    EXPECT_EQ(decrypted.str(), empty_data);
}

TEST(CoderTest, WithoutIVForCBC) {
    auto key = Coder::generate_key(16);
    
    // CBC требует IV, но если не передать - должен работать с нулевым IV
    Coder coder(CipherType::XORCipher, ModeType::CBC, key);
    
    std::string test_data = "Test without IV";
    std::stringstream input, encrypted, decrypted;
    input << test_data;
    
    // Шифрование должно работать
    EXPECT_NO_THROW(coder.encrypt(input, encrypted));
    
    // Дешифрование
    encrypted.clear();
    encrypted.seekg(0);
    EXPECT_NO_THROW(coder.decrypt(encrypted, decrypted));
    
    EXPECT_EQ(decrypted.str(), test_data);
}