#include <gtest/gtest.h>

#include "../trie/trie.hpp"


TEST(Trie, Trie)
{
    Containers::Trie<int> trie{};

    std::string s = "first";
    int a = 1;
    trie.insert(s, a);
}