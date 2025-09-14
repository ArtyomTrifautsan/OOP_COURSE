#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include <string>


//======================================CONCEPTS======================================
template<typename T>
concept String = std::is_same_v<T, std::string>;


template <typename ContainerType>
concept Container = !String<ContainerType> && requires(ContainerType& c)
{
    typename ContainerType::value_type;
    typename ContainerType::reference;
    typename ContainerType::const_reference;
    typename ContainerType::iterator;
    typename ContainerType::const_iterator;
    typename ContainerType::size_type;

    { c.begin() } -> std::same_as<typename ContainerType::iterator>;
    { c.end() } -> std::same_as<typename ContainerType::iterator>;
    { c.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
    { c.cend() } -> std::same_as<typename ContainerType::const_iterator>;
    { c.size() } -> std::same_as<typename ContainerType::size_type>;

    { c.clear() };
};

template <typename ContainerType>
concept SequentialContainer = Container<ContainerType> && requires(ContainerType& c, typename ContainerType::value_type v) 
{
    c.push_back(v);
};

template <typename ContainerType>
concept AssociativeContainer = Container<ContainerType> && requires(ContainerType& c, typename ContainerType::value_type v)
{
    typename ContainerType::key_type;
    { c.insert(v) } -> std::same_as<std::pair<typename ContainerType::iterator, bool>>;
};

template <typename ContainerType>
concept SequentialOrAssociativeContainer = SequentialContainer<ContainerType> || AssociativeContainer<ContainerType>;

template <typename ContainerType>
concept MapContainer = AssociativeContainer<ContainerType> && requires(ContainerType& c, typename ContainerType::key_type k, typename ContainerType::mapped_type v)
{
    { c.emplace(k, v) } -> std::same_as<std::pair<typename ContainerType::iterator, bool>>;
};




//===========================SERIALIZE/DESERIALIZE DECLARATIONS===========================

template <typename T>
void serialize(const T& obj, std::ostream& os);

template <typename T>
void deserialize(T& obj, std::istream& is);




//==================ALL SERIALIZER/DESERIALIZER TEMPLATES IMPLEMETATIONS==================

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

        if (len > 0)
        {
            // Write the data of the string
            std::ostream_iterator<uint8_t> oi(os, "");
            const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(str.data());
            std::copy(data_ptr, data_ptr + len, oi);
        }
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

        if (len > 0)
        {
            // Read the characters
            str.clear();
            str.resize(len);
            std::istream_iterator<char> ii(is);
            std::copy_n(ii, static_cast<size_t>(len), str.data());
        }
    }
};


/*
I try to make serializer and deserializer of std::pair that uses for reading and 
writing std::map keys and values. But this solution seemed difficult for me. So I 
realized separate serializer and deserializer for std::map using concept MapContainer


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
*/


// ===Sequential and associative containers===
template <SequentialOrAssociativeContainer ContainerType>
struct serializer<ContainerType>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for sequential or associative container" << std::endl;

        // Write the len of the container
        const uint32_t len = static_cast<uint32_t>(c.size());
        serialize(len, os);

        // Write objects
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

        // Read the len of container
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

template <AssociativeContainer ContainerType>
struct deserializer<ContainerType>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        // std::cout << "Using deserializer for associative container" << std::endl;

        c.clear();

        // Read the len of container
        uint32_t len = 0;
        deserialize(len, is);

        // Read objects
        for (uint32_t i = 0; i < len; i++)
        {
            typename ContainerType::value_type obj{};
            deserialize(obj, is);
            c.insert(obj);
        }
    }
};


template <MapContainer ContainerType>
struct serializer<ContainerType>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for map container" << std::endl;

        // Write the len of container
        const uint32_t len = static_cast<uint32_t>(c.size());
        serialize(len, os);

        // Write pairs of key/value
        for (const typename ContainerType::value_type& obj : c)
        {
            serialize(obj.first, os);
            serialize(obj.second, os);
        }
    }
};

template <MapContainer ContainerType>
struct deserializer<ContainerType>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        // std::cout << "Using deserializer for map container" << std::endl;

        c.clear();

        // Read the len of container
        uint32_t len = 0;
        deserialize(len, is);

        // Read pairs of key/value
        for (uint32_t i = 0; i < len; i++)
        {
            typename ContainerType::key_type key{};
            typename ContainerType::mapped_type value{};
            deserialize(key, is);
            deserialize(value, is);
            c.emplace(key, value);
        }
    }
};




//========================SERIALIZE/DESERIALIZE IMPLEMENTSTIONS===========================

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