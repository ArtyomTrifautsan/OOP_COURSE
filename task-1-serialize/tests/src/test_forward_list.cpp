#include <gtest/gtest.h>

#include <sstream>
#include <forward_list>
#include <string>
#include <unordered_map>

// #include <serialize_concepts.hpp>
// #include <serialize_sfinae.hpp>
#include "serialize.hpp"


TEST(TestForwardList, EmptyForwardListInt)
{
    std::forward_list<int> src_forward_list;

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<int> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}


TEST(TestForwardList, ForwardListWithInt)
{
    std::forward_list<int> src_forward_list {1, 2, 5, 4, 3};

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<int> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}


TEST(TestForwardList, ForwardListWithString)
{
    std::forward_list<std::string> src_forward_list {"first", "Artyom", "", "", "best", "C++"};

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<std::string> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}


TEST(TestForwardList, ForwardListWithSpecifiedString)
{
    std::forward_list<std::string> src_forward_list {"first", "Artyom", "", "", "best", "C++", "\0", "\n\0", "adw\t", "\0ycuwn"};

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<std::string> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}


TEST(TestForwardList, UserDefinedType)
{
    struct Point
    {
        int x, y;

        bool operator==(const Point& other) const noexcept
        {
            return x == other.x && y == other.y;
        }
        // bool operator!=(const Point& other) const noexcept
        // {
        //     return !(*this == other);
        // }
    };

    std::forward_list<Point> src_forward_list{};
    src_forward_list.push_front({10, 20});
    src_forward_list.push_front({30, 40});
    src_forward_list.push_front({50, 60});

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<Point> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}


TEST(TestForwardList, ComplexvalueType)
{
    std::forward_list<std::unordered_map<std::string, int>> src_forward_list{};
    std::unordered_map<std::string, int> my_unordered_map_1 = {
        {"first", 1},
        {"second", 2},
        {"third", 3}
    };
    src_forward_list.push_front(my_unordered_map_1);
    src_forward_list.push_front(my_unordered_map_1);
    std::unordered_map<std::string, int> my_unordered_map_2 = {
        {"no-first", 100},
        {"no-second", 200},
        {"no-third", 300}
    };
    src_forward_list.push_front(my_unordered_map_2);

    std::ostringstream oss(std::stringstream::binary);

    serialize(src_forward_list, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::forward_list<std::unordered_map<std::string, int>> deserialized_forward_list{};

    deserialize(deserialized_forward_list, iss);

    EXPECT_EQ(src_forward_list, deserialized_forward_list);
}