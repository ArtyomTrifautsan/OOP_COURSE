#pragma once

#include <iostream>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <cstdint>
#include <string>
#include <utility>



template <typename T>
void serialize(const T& obj, std::ostream& os);

template <typename T>
void deserialize(T& obj, std::istream& is);


template <typename ContainerType, typename = void>
struct has_begin_end : std::false_type{};

template <typename ContainerType>
struct has_begin_end<ContainerType, 
                    std::void_t<decltype(std::declval<ContainerType>().begin(), (std::declval<ContainerType>().end()))>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_size : std::false_type{};

template <typename ContainerType>
struct has_size<ContainerType, std::void_t<decltype(std::declval<ContainerType>().size())>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_clear : std::false_type{};

template <typename ContainerType>
struct has_clear<ContainerType, std::void_t<decltype(std::declval<ContainerType>().clear())>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_push_back : std::false_type{};

template <typename ContainerType>
struct has_push_back<ContainerType, std::void_t<decltype(std::declval<ContainerType>().push_back(std::declval<typename ContainerType::value_type>()))>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_insert : std::false_type{};

template <typename ContainerType>
struct has_insert<ContainerType, std::void_t<decltype(std::declval<ContainerType>().insert(std::declval<typename ContainerType::value_type>()))>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_before_begin : std::false_type{};

template <typename ContainerType>
struct has_before_begin<ContainerType, std::void_t<decltype(std::declval<ContainerType>().before_begin())>>
    : std::true_type{};



template <typename ContainerType, typename = void>
struct has_insert_after : std::false_type{};

template <typename ContainerType>
struct has_insert_after<ContainerType, std::void_t<decltype(std::declval<ContainerType>().insert_after(
                                            std::declval<typename ContainerType::iterator>(), 
                                            std::declval<typename ContainerType::value_type>()
                                        ))>>
    : std::true_type{};


template <typename T, typename = void>
struct serializer
{
    static void apply(const T& obj, std::ostream& os)
    {
        std::cout << "Using sfinae serializer" << std::endl;

        const uint8_t* ptr = reinterpret_cast<const uint8_t *>(&obj);

        std::ostream_iterator<uint8_t> oi(os);

        std::copy(ptr, ptr + sizeof(T), oi);
    }
};


template <typename T, typename = void>
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





template <typename ContainerType>
struct serializer<ContainerType, std::enable_if_t<has_size<ContainerType>::value && has_begin_end<ContainerType>::value>>
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


template <typename ContainerType>
struct deserializer<ContainerType, std::enable_if_t<has_clear<ContainerType>::value && has_push_back<ContainerType>::value>>
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


template <typename ContainerType>
struct deserializer<ContainerType, std::enable_if_t<has_size<ContainerType>::value && has_insert<ContainerType>::value>>
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


template <typename ContainerType>
struct serializer<ContainerType, std::enable_if_t<(!has_size<ContainerType>::value) && has_begin_end<ContainerType>::value>>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for forward_list container" << std::endl;

        // Write the len of the container
        uint32_t len = std::distance(c.begin(), c.end());
        serialize(len, os);

        // Write objects
        for (const auto& obj : c)
            serialize(obj, os);
    }
};


template <typename ContainerType>
struct deserializer<ContainerType, std::enable_if_t<has_clear<ContainerType>::value && has_insert_after<ContainerType>::value && has_before_begin<ContainerType>::value>>
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