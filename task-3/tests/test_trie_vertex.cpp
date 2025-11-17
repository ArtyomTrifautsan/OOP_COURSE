#include <gtest/gtest.h>

#include <map>
#include <unordered_map>

#include "../trie/trie.hpp"


//============================Test Trie interface============================
// Здесь я проверяю все методы Trie, что они вызываются и верно отрабатывают


TEST(InterfaceOfTrie, DefaultConstructor)
{
    Containers::Trie<int> trie{};
    EXPECT_EQ(trie.size(), 0);
}


TEST(InterfaceOfTrie, ConstructorWithIterators)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    EXPECT_EQ(trie.size(), 3);
}


TEST(InterfaceOfTrie, CopyConstructor)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{trie};
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(InterfaceOfTrie, MoveConstructor)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{std::move(trie)};
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(InterfaceOfTrie, CopyAssignOperator)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{++m.begin(), m.end()};
    EXPECT_EQ(copy_trie.size(), 2);
    copy_trie = trie;
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(InterfaceOfTrie, MoveAssignOperator)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{++m.begin(), m.end()};
    EXPECT_EQ(copy_trie.size(), 2);
    copy_trie = std::move(trie);
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(InterfaceOfTrie, BeginEnd)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    EXPECT_NE(trie.begin(), trie.end());
}


TEST(InterfaceOfTrie, ConstBeginEnd)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    const Containers::Trie<int> trie(m.begin(), m.end());
    EXPECT_NE(trie.begin(), trie.end());
}


TEST(InterfaceOfTrie, Empty)
{
    const Containers::Trie<int> empty_trie{};
    EXPECT_EQ(empty_trie.empty(), true);

    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    const Containers::Trie<int> not_empty_trie(m.begin(), m.end());
    EXPECT_EQ(not_empty_trie.empty(), false);
}


TEST(InterfaceOfTrie, Size)
{
    const Containers::Trie<int> empty_trie{};
    EXPECT_EQ(empty_trie.size(), 0);

    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    const Containers::Trie<int> not_empty_trie(m.begin(), m.end());
    EXPECT_EQ(not_empty_trie.size(), 3);
}


TEST(InterfaceOfTrie, OperatorBrackets)
{
    Containers::Trie<int> empty_trie{};
    empty_trie["1"] = 10;
    EXPECT_EQ(empty_trie["1"], 10);

    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> not_empty_trie(m.begin(), m.end());
    EXPECT_EQ(not_empty_trie["1"], 10);
    EXPECT_EQ(not_empty_trie.size(), 3);
    not_empty_trie["1"] = -10;
    EXPECT_EQ(not_empty_trie["1"], -10);
    EXPECT_EQ(not_empty_trie.size(), 3);    // Проверяю что после вызова оператора[] число элементов не изменилось
}


TEST(InterfaceOfTrie, InsertUsingKey)
{
    Containers::Trie<int> trie{};
    auto res = trie.insert("1", 10);
    auto value = (*(res.first)).second;
    EXPECT_EQ(value, 10);
    EXPECT_EQ(res.second, true);

    res = trie.insert("1", -10);
    value = (*(res.first)).second;
    EXPECT_EQ(value, -10);
    EXPECT_EQ(res.second, false);
}


TEST(InterfaceOfTrie, InsertUsingIterators)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{};
    trie.insert(m.begin(), m.end());

    EXPECT_EQ(trie.size(), 3);
}


TEST(InterfaceOfTrie, EraseUsingIterators)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto it = trie.begin();
    trie.erase(it);

    EXPECT_EQ(trie.size(), 2);
}


TEST(InterfaceOfTrie, EraseUsingKey)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto it = trie.begin();
    trie.erase("1");

    EXPECT_EQ(trie.size(), 2);
}


TEST(InterfaceOfTrie, EraseRange)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"20", 20}, {"21", 21}, {"3", 30}, {"31", 30}, {"4", 40}, {"5", 50}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto first = trie.find("2");
    auto last = trie.find("31");
    trie.erase(first, last);

    EXPECT_EQ(trie.size(), 4);
}


TEST(InterfaceOfTrie, Swap)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie1{m.begin(), m.end()};
    Containers::Trie<int> trie2{};

    EXPECT_EQ(trie1.size(), 3);
    EXPECT_EQ(trie2.size(), 0);

    trie1.swap(trie2);

    EXPECT_EQ(trie1.size(), 0);
    EXPECT_EQ(trie2.size(), 3);
}


TEST(InterfaceOfTrie, Clear)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    EXPECT_EQ(trie.size(), 3);

    trie.clear();

    EXPECT_EQ(trie.size(), 0);
}


TEST(InterfaceOfTrie, Find)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto it = trie.find("1");
    EXPECT_EQ((*it).second, 10);

    auto it2 = trie.find("no");
    EXPECT_EQ(it2, trie.end());
}


TEST(InterfaceOfTrie, ConstFind)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> const_trie{m.begin(), m.end()};

    auto const_it = const_trie.find("1");
    EXPECT_EQ((*const_it).second, 10);

    auto const_it2 = const_trie.find("no");
    EXPECT_EQ(const_it2, const_trie.end());
}



//============================Test using Trie============================
// Здесь я стараюсь всячески использовать Trie

namespace {

    struct MemoryCheckClass
    {
        MemoryCheckClass() { ctors += 1; }
        ~MemoryCheckClass() { dtors += 1; }
        inline static int ctors = 0;
        inline static int dtors = 0;
    };

}



// Проверяю использование Trie, созданного по умолчанию
TEST(TrieApplication, UsingDefaultConstructedTrie)
{
    Containers::Trie<int> trie{};

    EXPECT_EQ(trie.begin(), trie.end());
    EXPECT_EQ(trie.size(), 0);
    EXPECT_EQ(trie.empty(), true);

    trie.insert("first", 1);

    EXPECT_NE(trie.begin(), trie.end());
    EXPECT_EQ(trie.size(), 1);
    EXPECT_EQ(trie.empty(), false);
    EXPECT_EQ(trie["first"], 1);

    trie["second"] = 2;

    EXPECT_EQ(trie.size(), 2);
    EXPECT_EQ(trie["second"], 2);
}


// Проверка работы дерева c элементами
TEST(TrieApplication, UsingTrieCreatedByIterators)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());

    EXPECT_NE(trie.begin(), trie.end());
    EXPECT_EQ(trie.size(), 3);
    EXPECT_EQ(trie.empty(), false);

    trie.insert("fourth", 1);

    EXPECT_EQ(trie.size(), 4);
    EXPECT_EQ(trie["fourth"], 1);

    trie["fifrh"] = 2;

    EXPECT_EQ(trie.size(), 5);
    EXPECT_EQ(trie["fifrh"], 2);
}


// Проверка работы пустого дерева
TEST(TrieApplication, UsingClearedTrie)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());

    
}


// Проверка работы скопированного дерева
TEST(TrieApplication, UsingCopiedTrie)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{trie};
}


// Проверяю что метод clear() удаляет объекты, хранимые в Trie
TEST(TrieApplication, ClearTrie)
{
    {
        std::map<std::string, MemoryCheckClass> m = {
            {"a", MemoryCheckClass{}}, {"aa", MemoryCheckClass{}}, {"ab", MemoryCheckClass{}}, {"ac", MemoryCheckClass{}},
            {"b", MemoryCheckClass{}}, {"ba", MemoryCheckClass{}}, {"bb", MemoryCheckClass{}}, {"bc", MemoryCheckClass{}},
            {"c", MemoryCheckClass{}}, {"ca", MemoryCheckClass{}}, {"cb", MemoryCheckClass{}}, {"cc", MemoryCheckClass{}}
        };

        Containers::Trie<MemoryCheckClass> trie(m.begin(), m.end());

        /*
        Я не понимаю, почему эти три строчки работают не так, как я ожидаю?
        При вызове метода clear(), указатель m_root приравнивается к nullptr,
        т.е. все вершины моего trie должны удалиться, но они не удаляются. 
        
        Вообще я предполагаю, что это из-за того что я храню указатель на 
        parent. Возникает ситуация, когда родительская вершина имеет указатель
        на дочернюю вершину, и дочерняя вершина имеет указатель на родительскую
        вершину. И из-за этого они не могут удалиться.

        Как это решить? 2 способа:
        1) Переделать дерево так, чтобы в Vertex не было указателя parent
        2) В момент вызова clear(), у всех дочерних вершин указатель parent
        приравнивать к nullptr.
        */
        MemoryCheckClass::dtors = 0;
        trie.clear();
        EXPECT_EQ(MemoryCheckClass::dtors, 12);
    }

    EXPECT_EQ(MemoryCheckClass::dtors, 12);
}


/*
TEST(TrieApplication, IteratorReturnedByInsertMethod)
{
    Containers::Trie<int> trie{};

    auto res = trie.insert("a", 1);

    EXPECT_EQ(*(res.first), 1);
    EXPECT_EQ(res.second, true);
    EXPECT_NE(res.first, trie.end());
    EXPECT_EQ(++(res.first), trie.end());

    auto res2 = trie.insert("a2", 2);

    EXPECT_EQ(*(res2.first), 2);
    EXPECT_EQ(res2.second, true);
    EXPECT_NE(res2.first, trie.end());
    EXPECT_EQ(++(res2.first), trie.end());

    auto res3 = trie.insert("a1", 3);

    EXPECT_EQ(*(res3.first), 2);
    EXPECT_EQ(res3.second, true);
    EXPECT_NE(res3.first, trie.end());
    EXPECT_NE(++(res3.first), trie.end());
    EXPECT_EQ(++(res3.first), trie.end());
}
*/


// TEST()
// {
//     std::vector<int> v(1'000'000);
//     std::iota(v.begin(), v.end(), 1);
// }