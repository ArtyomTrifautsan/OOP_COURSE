#include <serialize_concepts.hpp>

#include <iterator>
#include <algorithm>
#include <cstdint>


// template <typename T>
// struct serializer 
// {
//     static void apply(const T& obj, std::ostream& os)
//     {
//         const uint8_t* ptr = reinterpret_cast<const uint8_t *>(&obj);

//         std::ostream_iterator<uint8_t> oi(os);

//         std::copy(ptr, ptr + sizeof(T), oi);
//     }
// };


// template <typename T>
// struct deserializer
// {
//     static void apply(T& val, std::istream& is)
//     {
//         uint8_t* ptr = reinterpret_cast<uint8_t*>(&val);

//         std::istream_iterator<uint8_t> ii(is);

//         std::copy_n(ii, sizeof(T), ptr);
//     }
// };


// template <typename T>
// void serialize(const T& obj, std::ostream& os)
// {
//     serializer<T>::aplly(obj, os);
// }


// template <typename T>
// void deserialize(T& obj, std::istream& is)
// {
//     // На сайте написано return deserializer<T>::apply(obj, is);
//     // но зачем? Это же не имеет смысла.
//     deserializer<T>::apply(obj, is);
// }