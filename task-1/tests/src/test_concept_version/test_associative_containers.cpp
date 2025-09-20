#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>

#include <serialize_concepts.hpp>


template <typename T>
void test_associative_container(const T& value)
{
    std::string filename = "test.ser";  // переделать на stringstream
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
    serialize(value, ofs);

    ofs.close();

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    T deserialized_value{};
    deserialize(deserialized_value, ifs);

    ifs.close();
    std::remove(filename.c_str());

    EXPECT_EQ(value, deserialized_value);
    // EXPECT_NQ(value, deserialized_value);
}


// std::map tests
TEST(TestMap, MapIntToIntEmpty) {
    std::map<int, int> m{};
    test_associative_container(m);
}

TEST(TestMap, MapIntToIntSimple) {
    std::map<int, int> m = {{1, 10}, {3, 30}, {2, 20}};
    test_associative_container(m);
}

TEST(TestMap, MapStringToInt) {
    std::map<std::string, int> m = {
        {"alpha", 1},
        {"gamma", 3},
        {"beta", 2},
        {"alpha", 99}
    };
    test_associative_container(m);
}

TEST(TestMap, MapIntToString) {
    std::map<int, std::string> m = {
        {1, "one"},
        {2, "two\0three"},
        {3, ""}
    };
    test_associative_container(m);
}

TEST(TestMap, MapStringWithNullKey) {
    std::map<std::string, int> m = {
        {"", 0},
        {"a\0b", 1},
        {"c", 2}
    };
    test_associative_container(m);
}

TEST(TestMap, MapDoubleToVector) {
    std::map<double, std::vector<int>> m = {
        {1.1, {1, 2, 3}},
        {2.2, {}},
        {3.3, {100, 200}}
    };
    test_associative_container(m);
}

TEST(TestMap, MapComplexKeys) {
    std::map<std::pair<int, int>, std::string> m = {
        {{1, 2}, "a"},
        {{3, 4}, "b"},
        {{0, 0}, "c"}
    };
    test_associative_container(m);
}

TEST(TestMap, MapLarge) {
    std::map<int, double> m;
    for (int i = 0; i < 500; ++i) {
        m[i] = i * 0.1 + 1.0;
    }
    test_associative_container(m);
}


// std::set tests
TEST(TestSet, SetIntEmpty) {
    std::set<int> s{};
    test_associative_container(s);
}

TEST(TestSet, SetIntSimple) {
    std::set<int> s = {5, 1, 9, 3, 7};
    test_associative_container(s);
}

TEST(TestSet, SetIntWithDuplicates) {
    std::set<int> s = {1, 1, 2, 2, 3};
    test_associative_container(s);
}

TEST(TestSet, SetString) {
    std::set<std::string> s = {"zebra", "apple", "banana", "apple"};
    test_associative_container(s);
}

TEST(TestSet, SetDouble) {
    std::set<double> s = {3.14, 2.71, -1.0, 0.0, 1e-10};
    test_associative_container(s);
}

TEST(TestSet, SetChar) {
    std::set<char> s = {'z', 'a', '\0', 'b', 'A'};
    test_associative_container(s);
}

TEST(TestSet, SetLongLong) {
    std::set<long long> s = {LLONG_MIN, -1, 0, 1, LLONG_MAX};
    test_associative_container(s);
}

TEST(TestSet, SetLarge) {
    std::set<int> s;
    for (int i = 0; i < 1000; ++i) {
        s.insert(i * 3 % 997); // чтобы были уникальные значения
    }
    test_associative_container(s);
}


// Complex structure
using SuperContainer = std::vector<
    std::unordered_map<
        std::string,
        std::vector<
            std::unordered_set<long long>
        >
    >
>;

TEST(TestSuperContainers, MultipleMapsWithDifferentKeys) {
    SuperContainer super_container;

    // Map 1
    std::unordered_map<std::string, std::vector<std::unordered_set<long long>>> map1;
    map1["first"] = {{1, 2}, {3}};
    map1["second"] = {{100, 200, 300}};
    super_container.push_back(map1);

    // Map 2
    std::unordered_map<std::string, std::vector<std::unordered_set<long long>>> map2;
    map2["x"] = {{}};
    map2["y"] = {{-5, -10}, {}, {1000000}};
    super_container.push_back(map2);

    test_associative_container(super_container);
}

TEST(TestSuperContainers, LargeStructure) {
    SuperContainer sc;

    for (int i = 0; i < 5; ++i) {
        std::unordered_map<std::string, std::vector<std::unordered_set<long long>>> map;
        std::string key = "map_" + std::to_string(i) + "\0data";

        for (int j = 0; j < 3; ++j) {
            std::unordered_set<long long> s;
            for (int k = 0; k < 10; ++k) {
                s.insert((long long)(i * 1000 + j * 100 + k));
            }
            map[key].push_back(s);
        }

        sc.push_back(map);
    }

    test_associative_container(sc);
}