#pragma once

#include <iostream>


template <typename T>
struct serializer 
{
    static void apply(const T& obj, std::ostream& os);
};