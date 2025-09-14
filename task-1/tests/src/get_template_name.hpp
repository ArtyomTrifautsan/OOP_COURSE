#pragma once

#include <sstream>
#include <streambuf>

#include <serialize.hpp>


template<typename T>
std::string get_serializer_name(const T& value) {
    std::stringstream ss;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(ss.rdbuf());

    Serializer<T>::serialize(value); // Вызовет нужную специализацию

    std::cout.rdbuf(old_cout);
    return ss.str();
}