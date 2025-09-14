#include <iostream>
#include <fstream>
#include <algorithm> // for std::equal
#include <string>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <serialize.hpp>


template <typename T>
void print_bytes(const T& obj)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&obj);
    std::cout << "Bytes (size: " << sizeof(T) << "): ";
    for (size_t i = 0; i < sizeof(T); ++i) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<int>(bytes[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

// template <>
// void print_bytes(const std::string& str)
// {
//     const uint32_t len = static_cast<uint32_t>(str.size());
//     const uint8_t* len_ptr = reinterpret_cast<const uint8_t*>(&len);
//     std::cout << "Bytes (size: " << sizeof(len) + len << "): ";
//     for (size_t i = 0; i < sizeof(len); ++i) {
//         std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
//                   << static_cast<int>(len_ptr[i]) << " ";
//     }

//     const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(str.data());
//     for (size_t i = 0; i < len; ++i) {
//         std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
//                   << static_cast<int>(data_ptr[i]) << " ";
//     }

//     std::cout << std::dec << std::endl;
// }


int main(int argc, char const *argv[])
{
    std::ofstream ofs("test.ser", std::ofstream::out | std::ofstream::binary);

    char hello[7] = "Hello!";
    std::string hello_str = "hello";
    std::vector<int> numbers = {1, 2, 3, 5, 4};
    std::map<std::string, int> my_map = {
        {"first", 100},
        {"second", 200},
        {"fourth", 400},
        {"seventh", 700},
    };
    std::set<float> my_set;
    my_set.insert(1.0);
    my_set.insert(2.0);
    my_set.insert(3.0);
    my_set.insert(-1.0);
    my_set.insert(-2.0);
    my_set.insert(-3.0);

    std::cout << "===For char[7]: ";
    print_bytes(hello);
    std::cout << "===For std::string: ";
    print_bytes(hello_str);
    std::cout << "===For std::vector: ";
    print_bytes(numbers);
    std::cout << "===For std::map: ";
    print_bytes(my_map);
    std::cout << "===For std::set: ";
    print_bytes(my_set);

    // std::cout << "===For char[7]: ";
    serialize(hello, ofs);
    // std::cout << "===For std::string: ";
    serialize(hello_str, ofs);
    // std::cout << "===For std::vector: ";
    serialize(numbers, ofs);
    std::cout << "serialize ===For std::map: ";
    serialize(my_map, ofs);
    std::cout << "serialize ===For std::set: ";
    serialize(my_set, ofs);

    ofs.close();

    std::ifstream ifs("test.ser", std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    char hello2[7];
    std::string hello_str_2 = "1";
    std::vector<int> numbers2;
    std::map<std::string, int> my_map2;
    std::set<float> my_set2;

    deserialize(hello2, ifs);
    deserialize(hello_str_2, ifs);
    deserialize(numbers2, ifs);
    std::cout << "deserialize ===For std::map: ";
    deserialize(my_map2, ifs);
    std::cout << "deserialize ===For std::set: ";
    deserialize(my_set2, ifs);
    
    std::cout << "===For char[7]: ";
    print_bytes(hello2);
    std::cout << "===For std::string: ";
    print_bytes(hello_str_2);
    std::cout << "===For std::vector: ";
    print_bytes(numbers2);
    std::cout << "===For std::map: ";
    print_bytes(my_map2);
    std::cout << "===For std::set: ";
    print_bytes(my_set2);

    ifs.close();

    // if (hello_str == hello_str_2) std::cout << "strings equal!!!" << std::endl;

    std::cout << "hello2: ";
    for (auto item : hello2) std::cout << item;
    std::cout << std::endl;

    std::cout << "hello_str_2: " << hello_str_2 << std::endl;

    std::cout << "Numbers2: ";
    for (auto item : numbers2) std::cout << " " << item;
    std::cout << std::endl;

    std::cout << "my_map2: ";
    for (auto item : my_map2) std::cout << " " << item.first << "=" << item.second;
    std::cout << std::endl;

    std::cout << "my_set2: ";
    for (auto item : my_set2) std::cout << " " << item;
    std::cout << std::endl;

    return 0;
}
