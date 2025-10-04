#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>

// #include <serialize_concepts.hpp>
#include <serialize_sfinae.hpp>


// template <typename T>
// void test_associative_container(const T& value)
// {
//     std::string filename = "test.ser";  // переделать на stringstream
//     std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
//     serialize(value, ofs);

//     ofs.close();

//     std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
//     ifs >> std::noskipws;

//     T deserialized_value{};
//     deserialize(deserialized_value, ifs);

//     ifs.close();
//     std::remove(filename.c_str());

//     EXPECT_EQ(value, deserialized_value);
//     // EXPECT_NQ(value, deserialized_value);
// }


// std::map tests
TEST(TestMap, MapIntToIntEmpty) {
    std::map<int, int> m{};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<int, int> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({100, 100});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapIntToIntSimple) {
    std::map<int, int> m = {{1, 10}, {3, 30}, {2, 20}};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<int, int> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({100, 100});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapStringToInt) {
    std::map<std::string, int> m = {
        {"alpha", 1},
        {"gamma", 3},
        {"beta", 2},
        {"alpha", 99}
    };
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<std::string, int> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({"alpha2", 100});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapIntToString) {
    std::map<int, std::string> m = {
        {1, "one"},
        {2, "two\0three"},
        {3, ""}
    };
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<int, std::string> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({100, "100"});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapStringWithNullKey) {
    std::map<std::string, int> m = {
        {"", 0},
        {"a\0b", 1},
        {"c", 2}
    };
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<std::string, int> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({"100", 100});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapDoubleToVector) {
    std::map<double, std::vector<int>> m = {
        {1.1, {1, 2, 3}},
        {2.2, {}},
        {3.3, {100, 200}}
    };
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<double, std::vector<int>> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({3.9, {100, 250}});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapComplexKeys) {
    std::map<std::pair<int, int>, std::string> m = {
        {{1, 2}, "a"},
        {{3, 4}, "b"},
        {{0, 0}, "c"}
    };
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<std::pair<int, int>, std::string> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({{10, 200}, "aaaaa"});
    EXPECT_NE(m, deserialized_m);
}

TEST(TestMap, MapLarge) {
    std::map<int, double> m;
    for (int i = 0; i < 500; ++i) {
        m[i] = i * 0.1 + 1.0;
    }
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(m, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::map<int, double> deserialized_m{};

    deserialize(deserialized_m, iss);

    EXPECT_EQ(m, deserialized_m);

    m.insert({1000, 170.0});
    EXPECT_NE(m, deserialized_m);
}


// std::set tests
TEST(TestSet, SetIntEmpty) {
    std::set<int> s{};

    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<int> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(1000);
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetIntSimple) {
    std::set<int> s = {5, 1, 9, 3, 7};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<int> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(1000);
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetIntWithDuplicates) {
    std::set<int> s = {1, 1, 2, 2, 3};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<int> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(1000);
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetString) {
    std::set<std::string> s = {"zebra", "apple", "banana", "apple"};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<std::string> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert("1000");
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetDouble) {
    std::set<double> s = {3.14, 2.71, -1.0, 0.0, 1e-10};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<double> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(1000.0);
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetChar) {
    std::set<char> s = {'z', 'a', '\0', 'b', 'A'};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<char> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert('q');
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetLongLong) {
    std::set<long long> s = {LLONG_MIN, -1, 0, 1, LLONG_MAX};
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<long long> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(1000);
    EXPECT_NE(s, deserialized_s);
}

TEST(TestSet, SetLarge) {
    std::set<int> s;
    for (int i = 0; i < 1000; ++i) {
        s.insert(i * 3 % 997); // чтобы были уникальные значения
    }
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::set<int> deserialized_s{};

    deserialize(deserialized_s, iss);

    EXPECT_EQ(s, deserialized_s);

    s.insert(-1);
    EXPECT_NE(s, deserialized_s);
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
    
    std::ostringstream oss(std::stringstream::binary);

    serialize(super_container, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    SuperContainer deserialized_super_container{};

    deserialize(deserialized_super_container, iss);

    EXPECT_EQ(super_container, deserialized_super_container);
}

TEST(TestSuperContainers, LargeStructure) {
    SuperContainer super_container;

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

        super_container.push_back(map);
    }

    std::ostringstream oss(std::stringstream::binary);

    serialize(super_container, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    SuperContainer deserialized_super_container{};

    deserialize(deserialized_super_container, iss);

    EXPECT_EQ(super_container, deserialized_super_container);
}