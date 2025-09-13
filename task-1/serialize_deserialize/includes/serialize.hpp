#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include <vector>
#include <string>
#include <array>
// #include



template<typename T>
concept String = std::is_same_v<T, std::string>;


template <typename ContainerType>
concept Container = !String<ContainerType> && requires(ContainerType& a)
{
    typename ContainerType::value_type;
    typename ContainerType::reference;
    typename ContainerType::const_reference;
    typename ContainerType::iterator;
    typename ContainerType::const_iterator;
    typename ContainerType::size_type;

    { a.begin() } -> std::same_as<typename ContainerType::iterator>;
    { a.end() } -> std::same_as<typename ContainerType::iterator>;
    { a.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
    { a.cend() } -> std::same_as<typename ContainerType::const_iterator>;
    { a.size() } -> std::same_as<typename ContainerType::size_type>;

    { a.clear() };
};

template <typename ContainerType>
concept SequentialContainer = Container<ContainerType> && requires(ContainerType c, typename ContainerType::value_type v) 
{
    c.push_back(v);
};

template <typename ContainerType>
concept AssociativeContainer = Container<ContainerType> && requires()
{
    typename ContainerType::key_type;
};





// ===DECLARATIONS===

template <typename T>
void serialize(const T& obj, std::ostream& os);

template <typename T>
void deserialize(T& obj, std::istream& is);


// template <typename T>
// void serialize(const std::vector<T>& vector, std::ostream& os);

// template <typename T>
// void deserialize(std::vector<T>& vector, std::istream& is);

// template <typename ContainerType>
// requires SequentialContainer<ContainerType>
// void serialize(const ContainerType& c, std::ostream& os);

// template <typename ContainerType>
// requires SequentialContainer<ContainerType>
// void deserialize(ContainerType& c, std::istream& is);


// ===IMPLEMENTATIOS===

template <typename T>
struct serializer
{
    static void apply(const T& obj, std::ostream& os)
    {
        // std::cout << "Using base serializer" << std::endl;

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
        // std::cout << "Using base deserializer" << std::endl;

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
        // std::cout << "Using string serializer" << std::endl;

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
        // std::cout << "Using string deserializer" << std::endl;

        // Read the len of the string
        uint32_t len = 0;
        deserialize(len, is);
        // std::cout << "len == " << len << std::endl;

        // Read the characters
        str.clear();
        str.resize(len); // узнать что вообще происходит тут
        std::istream_iterator<char> ii(is);
        std::copy_n(ii, static_cast<size_t>(len), str.data());
    }
};


template <typename T1, typename T2>
struct serializer<std::pair<T1, T2>>
{
    static void apply(const std::pair<T1, T2>& pair, std::ostream& os)
    {
        // std::cout << "Using pair serializer" << std::endl;

        serialize(pair.first, os);
        serialize(pair.second, os);
    }
};

template <typename T1, typename T2>
struct deserializer<std::pair<T1, T2>>
{
    static void apply(std::pair<T1, T2>& pair, std::istream& is)
    {
        // std::cout << "Using pair deserializer" << std::endl;

        deserialize(pair.first, is);
        deserialize(pair.second, is);
    }
};


// ===Sequential containers===
template <SequentialContainer ContainerType>
struct serializer<ContainerType>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for sequential container" << std::endl;

        // Write the len of the string
        const uint32_t len = static_cast<uint32_t>(c.size());
        serialize(len, os);

        // Write items of vector
        for (const typename ContainerType::value_type& obj : c)
            serialize(obj, os);
    }
};

template <SequentialContainer ContainerType>
struct deserializer<ContainerType>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        // std::cout << "Using deserializer for sequential container" << std::endl;

        c.clear();

        // Read the len of array
        uint32_t len = 0;
        deserialize(len, is);

        // Read objects
        for (uint32_t i = 0; i < len; i++)
        {
            typename ContainerType::value_type obj{};
            deserialize(obj, is);
            c.push_back(obj);
        }
    }
};


/*
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
*/


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


// template <typename T>
// void serialize(const std::vector<T>& vector, std::ostream& os)
// {
//     vector_serializer<T>::apply(vector, os);
// }

// template <typename T>
// void deserialize(std::vector<T>& vector, std::istream& is)
// {
//     vector_deserializer<T>::apply(vector, is);
// }


// template <typename ContainerType>
// requires SequentialContainer<ContainerType>
// void serialize(const ContainerType& c, std::ostream& os)
// {
//     sequential_container_serializer<ContainerType>::apply(c, os);
// }

// template <typename ContainerType>
// requires SequentialContainer<ContainerType>
// void deserialize(ContainerType& c, std::istream& is)
// {
//     sequential_container_deserializer<ContainerType>::apply(c, is);
// }