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
concept StandartContainer = !String<ContainerType> && requires(ContainerType& c)
{
    { c.begin() } -> std::same_as<typename ContainerType::iterator>;
    { c.end() } -> std::same_as<typename ContainerType::iterator>;
    { c.size() } -> std::same_as<typename ContainerType::size_type>;

    { c.clear() };
};


template <typename ContainerType>
concept SequentialContainer = StandartContainer<ContainerType> && requires(ContainerType& c, typename ContainerType::value_type v) 
{
    c.push_back(v);
};


template <typename ContainerType>
concept AssociativeContainer = StandartContainer<ContainerType> && requires(ContainerType& c, typename ContainerType::value_type v)
{
    typename ContainerType::key_type;
    { c.insert(v) };
};


template <typename ContainerType>
concept ForwardListContainer = !String<ContainerType> && requires(ContainerType& c, typename ContainerType::value_type v, typename ContainerType::iterator it)
{
    { c.begin() } -> std::same_as<typename ContainerType::iterator>;
    { c.end() } -> std::same_as<typename ContainerType::iterator>;
    { c.before_begin() } -> std::same_as<typename ContainerType::iterator>;
    { c.clear() };
    { c.insert_after(it, v) };
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
        // std::cout << "Using concepts serializer" << std::endl;

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
*/

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
struct deserializer<std::pair<const T1, T2>>
{
    static void apply(std::pair<const T1, T2>& pair, std::istream& is)
    {
        // std::cout << "Using pair deserializer" << std::endl;
        T1 volatile_key{};
        deserialize(volatile_key, is);
        T1* pair_first_ptr = const_cast<T1*>(&(pair.first));
        *pair_first_ptr = volatile_key;
        deserialize(pair.second, is);
    }
};



// ===Sequential and associative containers===
template <StandartContainer ContainerType>
struct serializer<ContainerType>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for standart container" << std::endl;

        // Write the len of the container
        const uint32_t len = static_cast<uint32_t>(c.size());
        serialize(len, os);

        // Write objects
        for (const auto& obj : c)
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


template <ForwardListContainer ContainerType>
struct serializer<ContainerType>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for forward_list container" << std::endl;

        // Write the len of the container
        // uint32_t len = 0;
        // for (const auto& obj : c)
        //     ++len;
        uint32_t len = static_cast<uint32_t>(std::distance(c.begin(), c.end()));
        serialize(len, os);

        // Write objects
        for (const auto& obj : c)
            serialize(obj, os);
    }
};


template <ForwardListContainer ContainerType>
struct deserializer<ContainerType>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        // std::cout << "Using deserializer for forward_list container" << std::endl;

        c.clear();

        // Read the len of container
        uint32_t len = 0;
        deserialize(len, is);

        // Read objects
        auto it = c.before_begin();
        for (uint32_t i = 0; i < len; i++)
        {
            typename ContainerType::value_type obj{};
            deserialize(obj, is);
            c.insert_after(it, obj);
            ++it;
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