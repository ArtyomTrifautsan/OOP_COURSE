#include "includes/serialize.hpp"

#include <iterator>
#include <algorithm>


template <typename T>
void serializer<T>::apply(const T& obj, std::ostream& os)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&obj);

    std::ostream_iterator<uint8_t> oi(os);

    std::copy(ptr, ptr + sizeof(T), oi);
}