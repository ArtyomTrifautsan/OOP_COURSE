#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>


/*
Точно добавить SFINAI
Возможно добавить концепты
*/


// ===DECLARATIONS===

template <typename T>
void serialize(const T& obj, std::ostream& os);

template <typename T>
void deserialize(T& obj, std::istream& is);


template <typename T>
void serialize(const std::vector<T>& vector, std::ostream& os);

template <typename T>
void deserialize(std::vector<T>& vector, std::istream& is);


// ===IMPLEMENTATIOS===

template <typename T>
struct serializer
{
    static void apply(const T& obj, std::ostream& os)
    {
        std::cout << "Using base serializer" << std::endl;

        const uint8_t* ptr = reinterpret_cast<const uint8_t *>(&obj);

        std::ostream_iterator<uint8_t> oi(os);

        std::copy(ptr, ptr + sizeof(T), oi);
    }
};

template <typename T>
struct deserializer
{
    static void apply(T& val, std::istream& is)
    {
        std::cout << "Using base deserializer" << std::endl;

        uint8_t* ptr = reinterpret_cast<uint8_t*>(&val);

        std::istream_iterator<uint8_t> ii(is);

        std::copy_n(ii, sizeof(T), ptr);
    }
};


// ===Supporting std::string===
/*
The idea behind serializing a string is that we write not only the bytes that 
represent the charachters of the string, but also the size of the string.
*/
template <>
struct serializer<std::string>
{
    static void apply(const std::string& str, std::ostream& os)
    {
        std::cout << "Using string serializer" << std::endl;

        // Write the len of the string
        const uint32_t len = static_cast<uint32_t>(str.size());
        serialize(len, os);

        // Write the data of the string
        std::ostream_iterator<uint8_t> oi(os, "");
        const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(str.data());
        std::copy(data_ptr, data_ptr + len, oi);
    }
};

template <>
struct deserializer<std::string>
{
    static void apply(std::string& str, std::istream& is)
    {
        std::cout << "Using string deserializer" << std::endl;

        // Read the len of the string
        uint32_t len = 0;
        deserialize(len, is);
        std::cout << "len == " << len << std::endl;

        // Read the characters
        str.clear();
        str.resize(len); // узнать что вообще происходит тут
        std::istream_iterator<char> ii(is);
        std::copy_n(ii, static_cast<size_t>(len), str.data());
    }
};


//===Supporting std::vector
template <typename T>
struct vector_serializer
{
    static void apply(const std::vector<T>& vector, std::ostream& os)
    {
        std::cout << "Using vector serializer" << std::endl;

        // Write the len of the string
        const uint32_t len = static_cast<uint32_t>(vector.size());
        serialize(len, os);

        // Write items of vector
        // std::ostream_iterator<uint8_t> oi(os, "");
        for (const T& obj : vector)
            serialize(obj, os);
    }
};

template <typename T>
struct vector_deserializer
{
    static void apply(std::vector<T>& vector, std::istream& is)
    {
        std::cout << "Using vector deserializer" << std::endl;

        vector.clear();

        // Read the len of array
        uint32_t len = 0;
        deserialize(len, is);

        // Read objects
        for (uint32_t i = 0; i < len; i++)
        {
            T obj{};
            deserialize(obj, is);
            vector.push_back(obj);
        }
    }
};


template <typename T>
void serialize(const T& obj, std::ostream& os)
{
    serializer<T>::apply(obj, os);
}

template <typename T>
void deserialize(T& obj, std::istream& is)
{
    deserializer<T>::apply(obj, is);
}


template <typename T>
void serialize(const std::vector<T>& vector, std::ostream& os)
{
    vector_serializer<T>::apply(vector, os);
}

template <typename T>
void deserialize(std::vector<T>& vector, std::istream& is)
{
    vector_deserializer<T>::apply(vector, is);
}