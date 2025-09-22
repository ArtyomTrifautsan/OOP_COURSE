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


// template<typename T> struct has_foo{
// private:
//     static int detect(...);
//     template<typename U> static decltype(std::declval<U>().foo(42)) detect(const U&);
// public:
//     static constexpr bool value = std::is_same<void, decltype(detect(std::declval<T>()))>::value;
// };


template <typename ContainerType>
struct has_begin_end
{
private:
/*обсудить как это работает пошагово.*/
    static void detect_begin(...);
    template<typename U> static decltype(std::declval<U>().begin()) detect_begin(const U&);

    static void detect_end(...);
    template<typename U> static decltype(std::declval<U>().end()) detect_end(const U&);

public:
    static constexpr bool has_begin_value = std::negation<std::is_same<void, decltype(detect_begin(std::declval<ContainerType>()))>>::value;
    static constexpr bool has_end_value = std::negation<std::is_same<void, decltype(detect_end(std::declval<ContainerType>()))>>::value;
    static constexpr bool value = has_begin_value && has_end_value;
};


template <typename ContainerType>
struct has_size
{
private:
    static void detect_size(...);
    template<typename U> static decltype(std::declval<U>().size()) detect_size(const U&);
public:
    static constexpr bool value = std::negation<std::is_same<void, decltype(detect_size(std::declval<ContainerType>()))>>::value;
};


template <typename ContainerType>
struct has_clear
{
private:
    static void* detect_clear(...);
    template<typename U> static decltype(std::declval<U>().clear()) detect_clear(const U&);
public:
    static constexpr bool value = std::negation<std::is_same<void*, decltype(detect_clear(std::declval<ContainerType>()))>>::value;
};


template <typename ContainerType>
struct has_push_back
{
private:
    static char detect_push_back(...);
    template<typename U> static decltype(std::declval<U>().push_back(std::declval<typename U::value_type>())) detect_push_back(const U&);
public:
    static constexpr bool value = std::negation<std::is_same<char, decltype(detect_push_back(std::declval<ContainerType>()))>>::value;
};


template <typename ContainerType>
struct has_insert
{
private:
    static void detect_insert(...);
    template<typename U> static decltype(std::declval<U>().insert(std::declval<typename U::value_type>())) detect_insert(const U&);
public:
    static constexpr bool value = std::negation<std::is_same<void, decltype(detect_insert(std::declval<ContainerType>()))>>::value;
};


template <typename ContainerType>
struct has_insert_after_before_begin
{
private:
    static void detect_insert_after(...);
    template<typename U> static decltype(std::declval<U>().insert_after(std::declval<typename U::iterator>(), std::declval<typename U::value_type>())) detect_insert_after(const U&);

    static void detect_before_begin(...);
    template<typename U> static decltype(std::declval<U>().before_begin()) detect_before_begin(const U&);
public:
    static constexpr bool has_insert_after = std::negation<std::is_same<void, decltype(detect_insert_after(std::declval<ContainerType>()))>>::value;
    static constexpr bool has_before_begin = std::negation<std::is_same<void, decltype(detect_before_begin(std::declval<ContainerType>()))>>::value;
    static constexpr bool value = has_insert_after && has_before_begin;
};


template <typename ContainerType>
using is_standart_container = std::enable_if_t<
    has_begin_end<ContainerType>::value &&
    has_clear<ContainerType>::value &&
    has_size<ContainerType>::value, 
    ContainerType
>;

template <typename ContainerType>
using is_sequential_container = std::enable_if_t<
    has_begin_end<ContainerType>::value && 
    has_clear<ContainerType>::value && 
    has_size<ContainerType>::value && 
    has_push_back<ContainerType>::value,
    ContainerType
>;

template <typename ContainerType>
using is_associative_container = std::enable_if_t<
    has_begin_end<ContainerType>::value && 
    has_clear<ContainerType>::value && 
    has_size<ContainerType>::value && 
    has_insert<ContainerType>::value,
    ContainerType
>;

template <typename ContainerType>
using is_forward_list = std::enable_if_t<
    has_clear<ContainerType>::value && 
    has_insert_after_before_begin<ContainerType>::value,
    ContainerType
>;

// static_assert(is_standart_container<std::vector<int>>*, "vector should be standard container");
// static_assert(is_sequential_container<std::vector<int>>*, "vector should be standard container");
// static_assert(is_standard_container<std::vector<int>>*, "vector should be standard container");
// static_assert(is_standard_container<std::vector<int>>*, "vector should be standard container");


template <typename T, typename U = void>
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


template <typename T, typename U = void>
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


template <>
struct serializer<std::string>
{
    static void apply(const std::string& str, std::ostream& os)
    {
        std::cout << "Using string serializer" << std::endl;

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
        std::cout << "Using string deserializer" << std::endl;

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
        std::cout << "Using pair serializer" << std::endl;

        serialize(pair.first, os);
        serialize(pair.second, os);
    }
};

template <typename T1, typename T2>
struct deserializer<std::pair<const T1, T2>>
{
    static void apply(std::pair<const T1, T2>& pair, std::istream& is)
    {
        std::cout << "Using pair deserializer" << std::endl;
        T1 volatile_key{};
        deserialize(volatile_key, is);
        T1* pair_first_ptr = const_cast<T1*>(&(pair.first));
        *pair_first_ptr = volatile_key;
        deserialize(pair.second, is);
    }
};





template <typename ContainerType>
struct serializer<ContainerType, is_standart_container<ContainerType>>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        std::cout << "Using serializer for standart container" << std::endl;

        // Write the len of the container
        const uint32_t len = static_cast<uint32_t>(c.size());
        serialize(len, os);

        // Write objects
        for (const auto& obj : c)
            serialize(obj, os);
    }
};


template <typename ContainerType>
struct deserializer<ContainerType, is_sequential_container<ContainerType>>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        std::cout << "Using deserializer for sequential container" << std::endl;

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
struct deserializer<ContainerType, is_associative_container<ContainerType>>
{
    static void apply(ContainerType& c, std::istream& is)
    {
        std::cout << "Using deserializer for associative container" << std::endl;

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
struct serializer<ContainerType, is_forward_list<ContainerType>>
{
    static void apply(const ContainerType& c, std::ostream& os)
    {
        // std::cout << "Using serializer for forward_list container" << std::endl;

        // Write the len of the container
        uint32_t len = 0;
        for (const auto& obj : c)
            ++len;
        serialize(len, os);

        // Write objects
        for (const auto& obj : c)
            serialize(obj, os);
    }
};


template <typename ContainerType>
struct deserializer<ContainerType, is_forward_list<ContainerType>>
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