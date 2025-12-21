#include <gtest/gtest.h>

#include <map>

#include "cryptography.hpp"
#include "serialize_sfinae.hpp"
#include "trie.hpp"


using namespace Cryptography;


// TEST(XORCipherTest, SingleBlockEncryption)
// {
//     auto cipher = cipher_factory(CipherType::XOR);
//     std::vector<uint8_t> key = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00};
//     cipher->set_key(key);

//     std::vector<uint8_t> plain = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
//     std::vector<uint8_t> expected = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
//     std::vector<uint8_t> encrypted(8);
    
//     cipher->encrypt_block(plain, encrypted);
//     EXPECT_EQ(encrypted, expected);

//     std::vector<uint8_t> decrypted(8);
//     cipher->decrypt_block(encrypted, decrypted);
//     EXPECT_EQ(decrypted, plain);
// }

// TEST(TEACipherTest, SingleBlockConsistency)
// {
//     auto cipher = cipher_factory(CipherType::TEA);
    
//     // Ключ 16 байт
//     std::vector<uint8_t> key = {
//         0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//         0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
//     };
//     cipher->set_key(key);

//     std::vector<uint8_t> plain = {'H', 'e', 'l', 'l', 'o', 'o', 'o', '!'};
//     std::vector<uint8_t> encrypted(8);
//     std::vector<uint8_t> decrypted(8);

//     cipher->encrypt_block(plain, encrypted);

//     EXPECT_NE(plain, encrypted);

//     cipher->decrypt_block(encrypted, decrypted);
//     EXPECT_EQ(plain, decrypted);
// }

// TEST(ModePropertiesTest, ECBIdenticalBlocks)
// {    
//     auto cipher = std::make_shared<XORCipher>();
//     cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    
//     auto ecb = std::make_unique<ECBMode>(cipher);

//     // Два одинаковых блока данных
//     std::stringstream input("1234567812345678"); 
//     std::stringstream output;
//     ecb->encrypt(input, output);

//     std::string res = output.str();
//     // В ECB два одинаковых входных блока дают два одинаковых выходных
//     EXPECT_EQ(res.substr(0, 8), res.substr(8, 8));
// }

// TEST(ModePropertiesTest, CBCIdenticalBlocks) {
    
//     auto cipher = std::make_shared<XORCipher>();
//     cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
    
//     // Используем IV
//     auto cbc = std::make_unique<CBCMode>(cipher, std::vector<uint8_t>(8, 0x00));
    
//     std::stringstream input("1234567812345678");
//     std::stringstream output;
//     cbc->encrypt(input, output);
    
//     std::string res = output.str();
//     // В CBC второй блок зависит от первого, поэтому они ДОЛЖНЫ быть разными
//     EXPECT_NE(res.substr(0, 8), res.substr(8, 8));
// }


//============================================================
//===                      ECB + XOR                       ===
//============================================================
TEST(ECBandXOR, PlainLessBlock)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("ABC");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 3 байта данных + 5 байт паддинга (0x05) = 8 байт
    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);
    
    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "ABC");
    EXPECT_EQ(decrypted.str().length(), 3);
}

TEST(ECBandXOR, PlainBiggerBlock)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("ABCDEfGhI");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 9 байт данных + 7 байт паддинга (0x07) = 16 байт
    EXPECT_EQ(encrypted.str().length(), 16);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "ABCDEfGhI");
    EXPECT_EQ(decrypted.str().length(), 9);
}

TEST(ECBandXOR, EmptyPlain)
{
    auto cipher = cipher_factory(CipherType::XOR);

    cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // добавится целый блок из байт 0x08
    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str().length(), 0);
    EXPECT_EQ(decrypted.str(), "");
}

TEST(ECBandXOR, PlainMultipleBlock)
{
    /*Тут проверяется случай когда длина данных кратна размеру блока шифрования.
      Проверяем что добавится ровно один блок паддинга.*/

    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("Hello, World! Hello, World! Hello, World! Hello!");    // 48 байт (кратно 8)
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 48 байт данных + 8 байт паддинга (0x07) = 56 байт
    EXPECT_EQ(encrypted.str().length(), 56);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "Hello, World! Hello, World! Hello, World! Hello!");
    EXPECT_EQ(decrypted.str().length(), 48);
}



//============================================================
//===                      CBC + XOR                       ===
//============================================================

TEST(CBCandXOR, PlainLessBlock)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({1, 2, 3, 4, 5, 6, 7, 8});
    // std::shared_ptr<IBlockCipher> shared_cipher = cipher;

    Coder coder;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, std::vector<uint8_t>(8, 0xAA)));

    std::string plain = "1HaZ";
    std::stringstream input(plain);
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream out;
    // Пересоздаем mode чтобы сбросить IV, нужно для дешифровки первого блока данных
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, std::vector<uint8_t>(8, 0xAA)));
    coder.decrypt(encrypted, out);

    EXPECT_EQ(out.str(), plain);
}

TEST(CBCandXOR, PlainBiggerBlock)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});

    Coder coder;
    std::vector<uint8_t> iv = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("I am Artyom");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 16);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str(), "I am Artyom");
    EXPECT_EQ(decrypted.str().length(), 11);
}

TEST(CBCandXOR, EmptyPlain)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str().length(), 0);
    EXPECT_EQ(decrypted.str(), "");
}

TEST(CBCandXOR, PlainMultipleBlock)
{
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    
    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("Hello, World! Hello, World! Hello, World! Hello!");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 56);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str(), "Hello, World! Hello, World! Hello, World! Hello!");
    EXPECT_EQ(decrypted.str().length(), 48);
}



//============================================================
//===                      ECB + TEA                       ===
//============================================================
TEST(ECBandTEA, PlainLessBlock)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("ABC");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 3 байта данных + 5 байт паддинга (0x05) = 8 байт
    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);
    
    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "ABC");
    EXPECT_EQ(decrypted.str().length(), 3);
}

TEST(ECBandTEA, PlainBiggerBlock)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});
    
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("ABCDEfGhI");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 9 байт данных + 7 байт паддинга (0x07) = 16 байт
    EXPECT_EQ(encrypted.str().length(), 16);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "ABCDEfGhI");
    EXPECT_EQ(decrypted.str().length(), 9);
}

TEST(ECBandTEA, EmptyPlain)
{
    auto cipher = cipher_factory(CipherType::TEA);

    cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});

    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // добавится целый блок из байт 0x08
    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str().length(), 0);
    EXPECT_EQ(decrypted.str(), "");
}

TEST(ECBandTEA, PlainMultipleBlock)
{
    /*Тут проверяется случай когда длина данных кратна размеру блока шифрования.
      Проверяем что добавится ровно один блок паддинга.*/

    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});
    
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));

    std::stringstream input("Hello, World! Hello, World! Hello, World! Hello!");    // 48 байт (кратно 8)
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    // 48 байт данных + 8 байт паддинга (0x07) = 56 байт
    EXPECT_EQ(encrypted.str().length(), 56);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // паддинг должен уйти автоматически
    EXPECT_EQ(decrypted.str(), "Hello, World! Hello, World! Hello, World! Hello!");
    EXPECT_EQ(decrypted.str().length(), 48);
}



//============================================================
//===                      CBC + TEA                       ===
//============================================================

TEST(CBCandTEA, PlainLessBlock)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({1, 2, 3, 4, 5, 6, 7, 8,
                    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    // std::shared_ptr<IBlockCipher> shared_cipher = cipher;

    Coder coder;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, std::vector<uint8_t>(8, 0xAA)));

    std::string plain = "1HaZ";
    std::stringstream input(plain);
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream out;
    // Пересоздаем mode чтобы сбросить IV, нужно для дешифровки первого блока данных
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, std::vector<uint8_t>(8, 0xAA)));
    coder.decrypt(encrypted, out);

    EXPECT_EQ(out.str(), plain);
}

TEST(CBCandTEA, PlainBiggerBlock)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});

    Coder coder;
    std::vector<uint8_t> iv = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("I am Artyom");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 16);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str(), "I am Artyom");
    EXPECT_EQ(decrypted.str().length(), 11);
}

TEST(CBCandTEA, EmptyPlain)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});

    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 8);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str().length(), 0);
    EXPECT_EQ(decrypted.str(), "");
}

TEST(CBCandTEA, PlainMultipleBlock)
{
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE});

    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));

    std::stringstream input("Hello, World! Hello, World! Hello, World! Hello!");
    std::stringstream encrypted;
    coder.encrypt(input, encrypted);

    EXPECT_EQ(encrypted.str().length(), 56);

    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    EXPECT_EQ(decrypted.str(), "Hello, World! Hello, World! Hello, World! Hello!");
    EXPECT_EQ(decrypted.str().length(), 48);
}



//============================================================
//===             Invalid encrypt key length               ===
//============================================================

TEST(InvalidEncryptKeyLength, XORkeyBigger)
{
    auto cipher = cipher_factory(CipherType::XOR);
    std::vector<uint8_t> invalid_key = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};

    EXPECT_THROW(cipher->set_key(invalid_key), EncryptKeyLengthError);
}

TEST(InvalidEncryptKeyLength, XORkeyLess)
{
    auto cipher = cipher_factory(CipherType::XOR);
    std::vector<uint8_t> invalid_key = {0xAA, 0xBB};

    EXPECT_THROW(cipher->set_key(invalid_key), EncryptKeyLengthError);
}

TEST(InvalidEncryptKeyLength, TEAkeyBigger)
{
    auto cipher = cipher_factory(CipherType::TEA);
    std::vector<uint8_t> invalid_key = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                                        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
                                        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};

    EXPECT_THROW(cipher->set_key(invalid_key), EncryptKeyLengthError);
}

TEST(InvalidEncryptKeyLength, TEAkeyLess)
{
    auto cipher = cipher_factory(CipherType::TEA);
    std::vector<uint8_t> invalid_key = {0xAA, 0xBB, 0xBE, 0xEF, 0xCA};

    EXPECT_THROW(cipher->set_key(invalid_key), EncryptKeyLengthError);
}


/*
Гвоздь программы. Я создал свой Trie, сериализовал с помощью своего serialize,
и зашифровал всеми способами.
*/
TEST(EncryptTrie, ECBandXOR)
{
    // 1. Создаем префиксное дерево
    Containers::Trie<int> trie{};
    for (int i = 0; i < 10; i++)
        trie.insert(std::to_string(i), i);

    // 2. Сериализуем его
    std::ostringstream oss(std::stringstream::binary);
    serialize(trie, oss);

    // 3. Шифруем
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));
    std::stringstream plain(oss.str());
    std::stringstream encrypted;
    coder.encrypt(plain, encrypted);

    // 4. Дешифруем
    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // 5. Десериализуем
    std::istringstream iss(decrypted.str(), std::stringstream::binary);
    iss >> std::noskipws;
    Containers::Trie<int> trie2{};
    deserialize(trie2, iss);

    // 6. Сравниваем с исходным
    EXPECT_EQ(trie.size(), trie2.size());
    auto it = trie.begin();
    auto it2 = trie2.begin();
    for (size_t i = 0; i < trie.size(); ++i)
    {
        EXPECT_EQ(*it, *it2);
        ++it;
        ++it2;
    }
}

TEST(EncryptTrie, CBCandXOR)
{
    // 1. Создаем префиксное дерево
    Containers::Trie<int> trie{};
    for (int i = 0; i < 10; i++)
        trie.insert(std::to_string(i), i);

    // 2. Сериализуем его
    std::ostringstream oss(std::stringstream::binary);
    serialize(trie, oss);

    // 3. Шифруем
    auto cipher = cipher_factory(CipherType::XOR);
    cipher->set_key({0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    std::stringstream plain(oss.str());
    std::stringstream encrypted;
    coder.encrypt(plain, encrypted);

    // 4. Дешифруем
    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    // 5. Десериализуем
    std::istringstream iss(decrypted.str(), std::stringstream::binary);
    iss >> std::noskipws;
    Containers::Trie<int> trie2{};
    deserialize(trie2, iss);

    // 6. Сравниваем с исходным
    EXPECT_EQ(trie.size(), trie2.size());
    auto it = trie.begin();
    auto it2 = trie2.begin();
    for (size_t i = 0; i < trie.size(); ++i)
    {
        EXPECT_EQ(*it, *it2);
        ++it;
        ++it2;
    }
}

TEST(EncryptTrie, ECBandTEA)
{
    // 1. Создаем префиксное дерево
    Containers::Trie<int> trie{};
    for (int i = 0; i < 10; i++)
        trie.insert(std::to_string(i), i);

    // 2. Сериализуем его
    std::ostringstream oss(std::stringstream::binary);
    serialize(trie, oss);

    // 3. Шифруем
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({1, 2, 3, 4, 5, 6, 7, 8,
                    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    Coder coder;
    coder.set_strategy(mode_factory(ModeType::ECB, cipher));
    std::stringstream plain(oss.str());
    std::stringstream encrypted;
    coder.encrypt(plain, encrypted);

    // 4. Дешифруем
    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.decrypt(encrypted, decrypted);

    // 5. Десериализуем
    std::istringstream iss(decrypted.str(), std::stringstream::binary);
    iss >> std::noskipws;
    Containers::Trie<int> trie2{};
    deserialize(trie2, iss);

    // 6. Сравниваем с исходным
    EXPECT_EQ(trie.size(), trie2.size());
    auto it = trie.begin();
    auto it2 = trie2.begin();
    for (size_t i = 0; i < trie.size(); ++i)
    {
        EXPECT_EQ(*it, *it2);
        ++it;
        ++it2;
    }
}

TEST(EncryptTrie, CBCandTEA)
{
    // 1. Создаем префиксное дерево
    Containers::Trie<int> trie{};
    for (int i = 0; i < 10; i++)
        trie.insert(std::to_string(i), i);

    // 2. Сериализуем его
    std::ostringstream oss(std::stringstream::binary);
    serialize(trie, oss);

    // 3. Шифруем
    auto cipher = cipher_factory(CipherType::TEA);
    cipher->set_key({1, 2, 3, 4, 5, 6, 7, 8,
                    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11});
    Coder coder;
    std::vector<uint8_t> iv = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    std::stringstream plain(oss.str());
    std::stringstream encrypted;
    coder.encrypt(plain, encrypted);

    // 4. Дешифруем
    encrypted.seekg(0);
    std::stringstream decrypted;
    coder.set_strategy(mode_factory(ModeType::CBC, cipher, iv));
    coder.decrypt(encrypted, decrypted);

    // 5. Десериализуем
    std::istringstream iss(decrypted.str(), std::stringstream::binary);
    iss >> std::noskipws;
    Containers::Trie<int> trie2{};
    deserialize(trie2, iss);

    // 6. Сравниваем с исходным
    EXPECT_EQ(trie.size(), trie2.size());
    auto it = trie.begin();
    auto it2 = trie2.begin();
    for (size_t i = 0; i < trie.size(); ++i)
    {
        EXPECT_EQ(*it, *it2);
        ++it;
        ++it2;
    }
}
