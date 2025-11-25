#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <vector>
#include <utility>

#include "../trie/trie.hpp"


/*
Список всего того что я хочу проверить:
1) Верный порядок элементов при доступе через итератор
2) Своевременный вызов деструкторов элементов
3) Уничтожение ненужных node при вызове erase
4) Можно ли добавить значение с пустым ключом
5) Оператор operator[] реально способен перезаписать значение

Обязательно проверить случаи когда m_root == nullptr
*/


//============================Test Trie interface============================


namespace {

    struct Rectangle
    {
    int x, y, w, h;
    };

    struct Circle
    {
    Circle() { radius = 0; }
    Circle(int _r) { radius = _r; }
    int radius;
    };

    struct MemoryCheckClass
    {
        MemoryCheckClass() { ctors += 1; }
        MemoryCheckClass(const MemoryCheckClass& other) { copy_ctors += 1; }
        MemoryCheckClass(MemoryCheckClass&& other) { move_ctors += 1; }
        ~MemoryCheckClass() { dtors += 1; }
        inline static int ctors = 0;
        inline static int copy_ctors = 0;
        inline static int move_ctors = 0;
        inline static int dtors = 0;
    };

}


TEST(TrieConstructors, DefaultConstructor)
{
    Containers::Trie<int> trie1{};
    EXPECT_EQ(trie1.size(), 0);
    EXPECT_EQ(trie1.empty(), true);

    Containers::Trie<std::vector<int>> trie2{};
    EXPECT_EQ(trie2.size(), 0);
    EXPECT_EQ(trie2.empty(), true);

    Containers::Trie<Rectangle> trie3{};
    EXPECT_EQ(trie3.size(), 0);
    EXPECT_EQ(trie3.empty(), true);
}


TEST(TrieConstructors, CheckMemoryUsingForDefaultConstructor)
{
    /*
        Суть этого теста - проверить, что при создании пустого trie не 
        происходит создания хранимых значений (например значений по умолчанию)
    */

    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    Containers::Trie<MemoryCheckClass> trie{};

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);
}


TEST(TrieConstructors, ConstructorWithIterators)
{
    std::map<std::string, int> m1 = {};
    Containers::Trie<int> trie1{m1.begin(), m1.end()};
    EXPECT_EQ(trie1.size(), 0);
    EXPECT_EQ(trie1.empty(), true);

    std::map<std::string, int> m2 = {{"a", 1}};
    Containers::Trie<int> trie2{m2.begin(), m2.end()};
    EXPECT_EQ(trie2.size(), 1);
    EXPECT_EQ(trie2.empty(), false);

    std::map<std::string, int> m3 = {{"a", 1}, {"b", 2}, {"c", 3}};
    Containers::Trie<int> trie3{m3.begin(), m3.end()};
    EXPECT_EQ(trie3.size(), 3);
    EXPECT_EQ(trie3.empty(), false);
}


TEST(TrieConstructors, CheckMemoryUsingForConstructorWithIterators)
{
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    Containers::Trie<MemoryCheckClass> trie{m.begin(), m.end()};

    /*
    Здесь я проверяю, что хранимые значения только копируются и в нужном количестве.
    Никаких посторонних перемещений, созданий объектов и никаких удалений.
    */
    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 3);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);
}


// TEST(TrieConstructors, HugeTrie)
// {
//     std::map<std::string, int> m = {};

//     const unsigned int N = 1'000'000;
//     for (int i = 0; i < N; i++)
//         m[std::to_string(i)] = i;

//     Containers::Trie<int> trie{m.begin(), m.end()};
//     EXPECT_EQ(trie.size(), N);
// }


TEST(TrieConstructors, CopyConstructorForEmptyTrie)
{
    std::map<std::string, int> empty_m = {};
    Containers::Trie<int> empty_trie{empty_m.begin(), empty_m.end()};

    Containers::Trie<int> copy_trie{empty_trie};

    EXPECT_EQ(copy_trie.size(), 0);
}


TEST(TrieConstructors, CopyConstructorForNonEmptyTrie)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    Containers::Trie<int> copy_trie{trie};

    EXPECT_EQ(copy_trie.size(), 2);
}


TEST(TrieConstructors, CheckMemoryUsingForCopyConstructor)
{
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    Containers::Trie<MemoryCheckClass> trie{m.begin(), m.end()};

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    Containers::Trie<MemoryCheckClass> other_trie{trie};

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 3);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);
}


TEST(TrieConstructors, CheckCorrectCopy)
{
    /*
    Проверяем что после копирования, изменения в исходном дереве не влияют
    на копию и наоборот
    */
}


// TEST(TrieConstructors, CopyConstructorForHugeTrie)
// {
//     std::map<std::string, int> m = {};

//     const unsigned int N = 100'000;
//     for (int i = 0; i < N; i++)
//         m[std::to_string(i)] = i;

//     Containers::Trie<int> trie{m.begin(), m.end()};

//     Containers::Trie<int> copy_trie{trie};

//     EXPECT_EQ(copy_trie.size(), N);
// }


TEST(TrieConstructors, MoveConstructor)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{std::move(trie)};
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(TrieDestructor, CheckMemoryFreeInEmptyTrie)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);
}


TEST(TrieDestructor, CheckMemoryFreeInTrieWithValues)
{
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{m.begin(), m.end()};
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 3);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 3);
}


TEST(TrieAssignOperators, CopyAssignOperator)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{++m.begin(), m.end()};
    EXPECT_EQ(copy_trie.size(), 2);
    copy_trie = trie;
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(TrieAssignOperators, CheckMemoryAllocInCopyAssignOperator)
{
    /*
    Суть теста: при присваивании самого себя никаких новых объектов создаваться
    не будет. А если к trie присваиваем какой-то other_trie, то в trie данные 
    удаляются (проверка вызова деструкторов), и в него копируются данные из 
    other_trie (проверка вызова конструкторов копирования).
    */
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    Containers::Trie<MemoryCheckClass> trie(m.begin(), m.end());

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    trie = trie;

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);

    Containers::Trie<MemoryCheckClass> other_trie(m.begin(), m.end());

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    other_trie = trie;

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 3);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 3);
}


TEST(TrieAssignOperators, MoveAssignOperator)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie(m.begin(), m.end());
    Containers::Trie<int> copy_trie{++m.begin(), m.end()};
    EXPECT_EQ(copy_trie.size(), 2);
    copy_trie = std::move(trie);
    EXPECT_EQ(copy_trie.size(), 3);
}


TEST(TrieAssignOperators, CheckMemoryAllocInMoveAssignOperator)
{
    /*
    Суть теста: не происходит создания новых объектов. Только перемещение
    уже существующих. И проверяется самоприсваивание.
    */
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"b", MemoryCheckClass{}}, {"c", MemoryCheckClass{}}};

    /*
    Здесь везде проверяются поля ctors, чтобы удостовериться,
    что никакие объекты не создаются
    */
    Containers::Trie<MemoryCheckClass> trie(m.begin(), m.end());

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    trie = std::move(trie);

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);

    Containers::Trie<MemoryCheckClass> other_trie(m.begin(), m.end());

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    other_trie = std::move(trie);

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 3);
}


TEST(TrieSquareBrackets, ReadExistKey)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    EXPECT_EQ(trie["a"], 1);
    EXPECT_EQ(trie["b"], 2);
    EXPECT_EQ(trie["c"], 3);
}


TEST(TrieSquareBrackets, WritExistKey)
{
    std::map<std::string, int> m = {{"a", 1}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    trie["a"] = -1;

    EXPECT_EQ(trie["a"], -1);
    EXPECT_EQ(trie.size(), 1);
}


TEST(TrieSquareBrackets, WritNewKey)
{
    std::map<std::string, int> m = {{"a", 1}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    trie["b"] = -1;

    EXPECT_EQ(trie["b"], -1);
    EXPECT_EQ(trie.size(), 2);
}


TEST(TrieIterators, BeginEndOfEmptyTrie)
{
    Containers::Trie<int> trie{};

    EXPECT_EQ(trie.begin(), trie.end());
}


TEST(TrieIterators, BeginEndOfNonEmptyTrie)
{
    std::map<std::string, int> m = {{"ten", 10}};
    Containers::Trie<int> trie(m.begin(), m.end());

    EXPECT_EQ(trie.begin()->first, "ten");
    EXPECT_EQ(trie.begin()->second, 10);
}


TEST(TrieIterators, CorrectSequence)
{
    Containers::Trie<int> trie{};

    // В произвольном порядке помещаю ключи со значениями
    trie.insert("aabc", 0);
    trie.insert("b", 0);
    trie.insert("aaa", 0);

    trie.insert("aa", 0);
    trie.insert("aaaaa", 0);
    trie.insert("bA", 0);

    trie.insert("aab", 0);
    trie.insert("ba", 0);
    trie.insert("aac", 0);

    std::vector<std::string> right_sequence = {"aa", "aaa", "aaaaa", "aab", "aabc", "aac", "b", "bA", "ba"};

    auto begin = trie.begin();
    auto end = trie.end();
    int n = 0;

    while(begin != end)
    {
        EXPECT_EQ(begin->first, right_sequence[n]);
        ++n;
        ++begin;
    }
}


TEST(TrieIterators, WriteValuesUsingIters)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    // Здесь мы записываем значения с помощью итераторов 
    auto it = trie.begin();

    while (it != trie.end())
    {
        (*it).second *= 2;
        ++it;
    }

    // А здесь проверяем что всё верно записалось
    EXPECT_EQ(trie["a"], 2);
    EXPECT_EQ(trie["b"], 4);
    EXPECT_EQ(trie["c"], 6);
}


TEST(TrieIterators, RangeBasedCycle)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    // С помощью range-based цикла меняем значения в trie
    for (auto& item : trie)
        item.second += 5;

    // А здесь проверяем что всё корректно поменялось
    EXPECT_EQ(trie["a"], 6);
    EXPECT_EQ(trie["b"], 7);
    EXPECT_EQ(trie["c"], 8);
}


TEST(TrieIterators, ConstantIters)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    const Containers::Trie<int> trie{m.begin(), m.end()};

    std::vector<std::pair<const std::string, int>> right_sequence = {{"a", 1}, {"b", 2}, {"c", 3}};

    auto it = trie.begin();
    size_t n = 0;
    while(it != trie.end())
    {
        EXPECT_EQ(it->first, right_sequence[n].first);
        EXPECT_EQ(it->second, right_sequence[n].second);

        ++it;
        ++n;
    }
}


TEST(TrieIterators, PrefixIncrement)
{
    std::map<std::string, int> m = {{"a", 1}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto it = trie.begin();
    auto it2 = ++it;

    EXPECT_EQ(it, trie.end());
    EXPECT_EQ(it2, trie.end());
}


TEST(TrieIterators, PostfixIncrement)
{
    std::map<std::string, int> m = {{"a", 1}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto it = trie.begin();
    auto it2 = it++;

    EXPECT_EQ(it, trie.end());
    EXPECT_NE(it2, trie.end());
}


TEST(TrieFind, FindExistKey)
{
    std::map<std::string, Circle> m = {{"a", Circle{1}}, {"b", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    auto it = trie.find("a");

    EXPECT_EQ(it->second.radius, 1);
}


TEST(TrieFind, FindKeyThatNotInTrie)
{
    std::map<std::string, Circle> m = {{"a", Circle{1}}, {"b", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    auto it = trie.find("c");

    EXPECT_EQ(it, trie.end());
}


TEST(TrieFind, FindKeyThatPrefixOfOtherKey)
{
    /*
    Найти ключ, который является префиксом какого-то другого ключа.
    По сути это проверка, что мы дойдем до нужного ключа и не пойдем
    дальше.
    */
    std::map<std::string, Circle> m = {{"a", Circle{1}}, {"aaa", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    auto it = trie.find("a");

    EXPECT_EQ(it->second.radius, 1);
}


TEST(TrieFind, FindKeyThatHaveValueInPrefix)
{
    /*
    Найти ключ, в префиксе которого тоже есть хранимое значение
    */
    std::map<std::string, Circle> m = {{"a", Circle{1}}, {"aaa", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    auto it = trie.find("aaa");

    EXPECT_EQ(it->second.radius, 2);
}


TEST(TrieFind, FindNotExistKeyThatIsPrefixOfOtherKey)
{
    /*
    Ищем ключ, которого нет в дереве, но он является префиксом 
    какого-то другого ключа, который в девере есть
    */
    std::map<std::string, Circle> m = {{"aaa", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    EXPECT_EQ(trie.find("a"), trie.end());
}


TEST(TrieFind, FindNotExistKeyThatHasPrefixInTrie)
{
    /*
    Ищем ключ, которого нет в дереве, его префикс является ключом
    */
    std::map<std::string, Circle> m = {{"a", Circle{2}}};

    Containers::Trie<Circle> trie{m.begin(), m.end()};

    EXPECT_EQ(trie.find("aaa"), trie.end());
}


TEST(TrieInsert, InsertByKeyIntoEmptyTrie)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};

        trie.insert("a", MemoryCheckClass{});

        EXPECT_EQ(trie.size(), 1);

        EXPECT_EQ(MemoryCheckClass::ctors, 1);
        EXPECT_EQ(MemoryCheckClass::copy_ctors, 1);
        EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
        EXPECT_EQ(MemoryCheckClass::dtors, 1);
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 1);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 1);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 2);
}


TEST(TrieInsert, InsertByNewKey)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};

        trie.insert("a", MemoryCheckClass{});

        trie.insert("b", MemoryCheckClass{});

        EXPECT_EQ(trie.size(), 2);

        EXPECT_EQ(MemoryCheckClass::ctors, 2);
        EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
        EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
        EXPECT_EQ(MemoryCheckClass::dtors, 2);
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 2);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 4);
}


TEST(TrieInsert, InsertByKeyThatExist)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};

        trie.insert("a", MemoryCheckClass{});

        trie.insert("a", MemoryCheckClass{});

        EXPECT_EQ(trie.size(), 1);

        EXPECT_EQ(MemoryCheckClass::ctors, 2);
        EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
        EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
        EXPECT_EQ(MemoryCheckClass::dtors, 3);
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 2);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 4);
}


TEST(TrieInsert, InsertByKeyThatHavePrefixInTrie)
{
    /*
    Вставляем новое значение с ключом, начало которого уже имеется в дереве
    */

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};

        trie.insert("a", MemoryCheckClass{});

        trie.insert("aaa", MemoryCheckClass{});

        EXPECT_EQ(trie.size(), 2);

        EXPECT_EQ(MemoryCheckClass::ctors, 2);
        EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
        EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
        EXPECT_EQ(MemoryCheckClass::dtors, 2);
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 2);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 4);
}


TEST(TrieInsert, InsertByKeyThatPrefixOfOtherKey)
{
    /*
    Вставляем новое значение с ключом, который является префиксом другого, 
    уже находящегося в дереве, ключа.
    */

    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    {
        Containers::Trie<MemoryCheckClass> trie{};

        trie.insert("aaa", MemoryCheckClass{});

        trie.insert("a", MemoryCheckClass{});

        EXPECT_EQ(trie.size(), 2);

        EXPECT_EQ(MemoryCheckClass::ctors, 2);
        EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
        EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
        EXPECT_EQ(MemoryCheckClass::dtors, 2);
    }

    EXPECT_EQ(MemoryCheckClass::ctors, 2);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 2);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 4);
}


TEST(TrieInsert, CheckReturnedIteratorIsCorrect)
{
    Containers::Trie<int> trie{};

    auto res = trie.insert("a", 1);

    EXPECT_EQ(res.first->first, "a");
    EXPECT_EQ(res.first->second, 1);

    res = trie.insert("b", 2);

    EXPECT_EQ(res.first->first, "b");
    EXPECT_EQ(res.first->second, 2);

    res = trie.insert("a", 3);

    EXPECT_EQ(res.first->first, "a");
    EXPECT_EQ(res.first->second, 3);
}


TEST(TrieInsert, CheckReturnedInsertFlagIsCorrect)
{
    Containers::Trie<int> trie{};

    auto res = trie.insert("a", 1);

    EXPECT_EQ(res.second, true);

    res = trie.insert("b", 2);

    EXPECT_EQ(res.second, true);

    res = trie.insert("a", 3);

    EXPECT_EQ(res.second, false);
}


TEST(TrieInsert, InsertByEmptyKey)
{
    Containers::Trie<int> trie{};

    trie.insert("", 1);

    EXPECT_EQ(trie.size(), 1);
}


TEST(TrieInsert, InsertByIterators)
{
    std::map<std::string, int> m = {{"a", 10}, {"b", 20}, {"c", 30}};

    Containers::Trie<int> trie{};

    trie.insert(m.begin(), m.end());

    EXPECT_EQ(trie.size(), 3);
}


TEST(TrieInsert, InsertEmptyRangeByIterators)
{
    std::map<std::string, int> m = {};

    Containers::Trie<int> trie{};

    trie.insert(m.begin(), m.end());

    EXPECT_EQ(trie.size(), 0);
}


TEST(TrieErase, EraseExistKey)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("a");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 2);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("a"), trie.end());
    EXPECT_EQ(trie["b"], 2);
    EXPECT_EQ(trie["c"], 3);
}


TEST(TrieErase, EraseKeyThatNotInTrie)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("d");

    EXPECT_EQ(trie.size(), 3);

    EXPECT_EQ(res, 0);
}


TEST(TrieErase, EraseKeyThatHavePrefixInTrie)
{
    /*
    Удаляем ключ, префикс которого тоже является ключом какого-то элемента. Здесь 
    удаляется "ab", а его префикс "a" - это ключ, там тоже хранится значение.
    */
    std::map<std::string, int> m = {{"a", 1}, {"ab", 2}, {"ac", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("ab");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 2);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("ab"), trie.end());
    EXPECT_EQ(trie["a"], 1);
    EXPECT_EQ(trie["ac"], 3);
}


TEST(TrieErase, EraseKeyThatIsPrefixOfOtherKey)
{
    /*
    Удаляем ключ, который является префиксом другого ключа. Здесь удаляется ключ "a", 
    который является префиксом для ключа "ab".
    */
    std::map<std::string, int> m = {{"a", 1}, {"ab", 2}, {"cac", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("a");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 2);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("a"), trie.end());
    EXPECT_EQ(trie["ab"], 2);
    EXPECT_EQ(trie["cac"], 3);
}


TEST(TrieErase, EraseKeyThatHaveSeveralPrefixesInTrie)
{
    /*
    Удаляем ключ, в префиксе которого есть несколько других ключей.
    */
    std::map<std::string, int> m = {{"a", 1}, {"aba", 2}, {"abac", 3}, {"abaad", 4}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("abaad");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 3);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("abaad"), trie.end());
    EXPECT_EQ(trie["a"], 1);
    EXPECT_EQ(trie["aba"], 2);
    EXPECT_EQ(trie["abac"], 3);
}


TEST(TrieErase, EraseKeyThatIsPefixOfOtherKeyAndHasKeyInPrefix)
{
    /*
    Здесь проверяется случай, когда удаляемый ключ имеет ключ в своем префиксе,
    а также сам является префиксом дурго ключа
    */
    std::map<std::string, int> m = {{"a", 1}, {"aba", 2}, {"abac", 3}, {"abaad", 4}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("aba");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 3);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("aba"), trie.end());
    EXPECT_EQ(trie["a"], 1);
    EXPECT_EQ(trie["abac"], 3);
    EXPECT_EQ(trie["abaad"], 4);
}


TEST(TrieErase, EraseKeyThatIsPefixOfOtherKeysAndHasKeysInPrefix)
{
    /*
    Здесь проверяется случай, аналогичный прошлому тесту, только удаляемый ключ имеет 
    несколько ключей в своем префиксе, а также сам является префиксом нескольких других
    ключей
    */
    std::map<std::string, int> m = {{"a", 1}, {"aba", 2}, {"abac", 3}, {"abacd", 4}, {"abacda", 5}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("abac");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 4);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("abac"), trie.end());
    EXPECT_EQ(trie["a"], 1);
    EXPECT_EQ(trie["aba"], 2);
    EXPECT_EQ(trie["abacd"], 4);
    EXPECT_EQ(trie["abacda"], 5);
}


TEST(TrieErase, EraseNotExistKeyThatIsPrefixOfOtherKey)
{
    /*
    Случай, когда удаляемого ключа нет в дереве, но он является префиксом существующего
    ключа
    */
    std::map<std::string, int> m = {{"aaa", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("a");

    EXPECT_EQ(res, 0);

    EXPECT_EQ(trie.size(), 3);
}


TEST(TrieErase, EraseNotExistKeyThatHasKeyInPrefix)
{
    /*
    Случай, когда удаляемого ключа нет в дереве, но его префикс является ключом
    */
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("aaa");

    EXPECT_EQ(res, 0);

    EXPECT_EQ(trie.size(), 3);
}


TEST(TrieErase, EraseKeyInNonEmptySubtree)
{
    /*
    В поддереве с префиксом "aa" есть ключи "aab", "aabc", "aac", "aad". Тут
    проверяется, что после удаления одного из элементов поддерева (тут удаляется
    "aac") другие элементы останутся. Это важная проверка! 
    */
    std::map<std::string, int> m = {{"aa", 1}, {"aab", 2}, {"aabc", 3}, {"aac", 4}, {"aad", 5}, {"bb", 6}, {"bc", 7}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    size_t res = trie.erase("aac");

    // Здесь я проверяю сам факт удаления
    EXPECT_EQ(trie.size(), 6);

    EXPECT_EQ(res, 1);

    // Здесь я проверяю что удалился только нужный ключ, а остальные остались на месте
    EXPECT_EQ(trie.find("aac"), trie.end());
    EXPECT_EQ(trie["aa"], 1);
    EXPECT_EQ(trie["aab"], 2);
    EXPECT_EQ(trie["aabc"], 3);
    EXPECT_EQ(trie["aad"], 5);
    EXPECT_EQ(trie["bb"], 6);
    EXPECT_EQ(trie["bc"], 7);
}


TEST(TrieErase, EraseByIteratorToExistKey)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    trie.erase(trie.find("b"));

    EXPECT_EQ(trie.size(), 2);
}


TEST(TrieErase, EraseByEndIterator)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    EXPECT_THROW(trie.erase(trie.end()), std::runtime_error);

    try {
        trie.erase(trie.end());
        FAIL() << "Ожидалось std::runtime_error";
    } 
    catch (const std::runtime_error& e) {
        EXPECT_STREQ(
            "Invalid iterator: the end() iterator cannot be used as a value for position.",
            e.what()
        );
    }
}


TEST(TrieErase, EraseByPairIters)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    trie.erase(trie.find("b"), trie.find("e"));

    EXPECT_EQ(trie.size(), 2);
}


TEST(TrieErase, CheckMemoryFree)
{
    std::map<std::string, MemoryCheckClass> m = {{"a", MemoryCheckClass{}}, {"aa", MemoryCheckClass{}}, {"aaa", MemoryCheckClass{}}};

    Containers::Trie<MemoryCheckClass> trie{m.begin(), m.end()};

    MemoryCheckClass::dtors = 0;

    trie.erase("aa");

    EXPECT_EQ(MemoryCheckClass::dtors, 1);
}


TEST(TrieClear, ClearEmptyTrie)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    Containers::Trie<MemoryCheckClass> trie{};

    trie.clear();

    EXPECT_EQ(trie.size(), 0);

    EXPECT_EQ(trie.empty(), true);

    EXPECT_EQ(MemoryCheckClass::ctors, 0);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 0);
}


TEST(TrieClear, ClearNotEmptyTrie)
{
    MemoryCheckClass::ctors = 0;
    MemoryCheckClass::copy_ctors = 0;
    MemoryCheckClass::move_ctors = 0;
    MemoryCheckClass::dtors = 0;

    Containers::Trie<MemoryCheckClass> trie{};

    trie.insert("a", MemoryCheckClass{});

    trie.insert("aa", MemoryCheckClass{});
    trie.insert("ab", MemoryCheckClass{});
    trie.insert("ac", MemoryCheckClass{});

    trie.insert("aaa", MemoryCheckClass{});
    trie.insert("aab", MemoryCheckClass{});
    trie.insert("aac", MemoryCheckClass{});

    trie.insert("b", MemoryCheckClass{});

    trie.insert("ba", MemoryCheckClass{});
    trie.insert("bc", MemoryCheckClass{});

    trie.insert("bcc", MemoryCheckClass{});

    EXPECT_EQ(MemoryCheckClass::ctors, 11);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 11);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 11);

    trie.clear();

    EXPECT_EQ(MemoryCheckClass::ctors, 11);
    EXPECT_EQ(MemoryCheckClass::copy_ctors, 11);
    EXPECT_EQ(MemoryCheckClass::move_ctors, 0);
    EXPECT_EQ(MemoryCheckClass::dtors, 22);
}


TEST(TrieSwap, SwapTwoTries)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie1{m.begin(), m.end()};
    Containers::Trie<int> trie2{};

    trie1.swap(trie2);

    EXPECT_EQ(trie1.size(), 0);
    EXPECT_EQ(trie2.size(), 3);
}


TEST(TrieSwap, SelfSwap)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    trie.swap(trie);

    EXPECT_EQ(trie.size(), 3);
}


TEST(SubTrie, CreateSubTrie)
{
    std::map<std::string, int> m = {{"1", 10}, {"2", 20}, {"3", 30}};
    Containers::Trie<int> trie{m.begin(), m.end()};

    auto sub_trie = trie.GetSubTrie("1");

    EXPECT_EQ(sub_trie.size(), 1);
}


TEST(SubTrie, CreateSubTrieFromNonExistKey)
{
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};

    Containers::Trie<int> trie{m.begin(), m.end()};

    EXPECT_THROW(auto sub_trie = trie.GetSubTrie("1"), std::runtime_error);

    try {
        auto sub_trie = trie.GetSubTrie("1");
        FAIL() << "Ожидалось std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ(
            "Invalid key error: The key is not found in the trie.",
            e.what()
        );
    }
}


TEST(SubTrie, BeginEnd)
{
    /*
    Суть теста: проверяем, что begin и end задают нужный диапазон
    */
    Containers::Trie<int> trie{};

    trie.insert("aabc", 0);
    trie.insert("b", 0);
    trie.insert("aaa", 0);

    trie.insert("aa", 0);
    trie.insert("aaaaa", 0);
    trie.insert("bA", 0);

    trie.insert("aab", 0);
    trie.insert("ba", 0);
    trie.insert("aac", 0);

    trie.insert("a", 0);
    trie.insert("ab", 0);

    // должен стать таким: {"aa", "aaa", "aaaaa", "aab", "aabc", "aac"};
    std::vector<std::string> checked_items = {};
    std::vector<std::string> expect = {"aa", "aaa", "aaaaa", "aab", "aabc", "aac"};
    
    auto sub_trie = trie.GetSubTrie("aa");

    EXPECT_EQ(sub_trie.size(), 6);

    auto sub_trie_it = sub_trie.begin();

    while (sub_trie_it != sub_trie.end())
    {
        checked_items.push_back(sub_trie_it->first);
        ++sub_trie_it;
    }

    EXPECT_EQ(checked_items, expect);
}


// std::string s = "aaa";
// s[0] = char(255);

/*
Более комплексные тесты:
1) Проверить что после копирования/перемещения в trie работают все методы
2) Если удалить все значения последовательно с помощью erase(), то дерево будет работать как пустое
3) проверить работу каждого из 256 символов 
*/
