#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "shared_pointer.hpp"


// Этот класс нужен для тестов, в которых я проверяю, что объект уничтожился
class TestClass
{
public:
    TestClass() = default; 
    TestClass(bool* _flag) : m_delete_flag_ptr{_flag} {}
    ~TestClass()
    {
        *m_delete_flag_ptr = true;
        m_delete_flag_ptr = nullptr;
    }
private:
    bool* m_delete_flag_ptr = nullptr;
};


// Этот класс нужен для теста, в котором я проверяю, что объект конструируется только один раз,
// несмотря на создание нескольких shared_ptr
class TestClassWithCounter
{
public:
    TestClassWithCounter() = default; 
    TestClassWithCounter(int* _counter) : m_counter_ptr{_counter} { *m_counter_ptr += 1; }
    ~TestClassWithCounter() {}
private:
    int* m_counter_ptr = nullptr;
};


// Этот класс нужен для тестирования пользовательского deleter
class CustomDeleter
{
public:
    CustomDeleter() = default;
    CustomDeleter(bool* flag) : m_deleted{flag} {}
    void operator()(int* ptr)
    {
        delete ptr;
        *m_deleted = true;
    }
private:
    bool* m_deleted = nullptr;
};


// Класс для тестирвоания make_shared функции с перменным количеством параметров
class Point
{
public:
    Point(int _x, int _y) : m_x{_x}, m_y{_y} {}
    int x() const noexcept { return m_x; }
    int y() const noexcept { return m_y; }
private:
    int m_x = 0;
    int m_y = 0;
};



TEST(DefaultConstructor, DefaultConstructor)
{
    Pointers::SharedPTR<int> p{};

    EXPECT_EQ(p, nullptr);
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


// TEST(Constructor, EmptyIntArray)
// {
//     class SpecialDeleter
//     {
//     public:
//         void operator()(int* ptr) { delete[] ptr; }  
//     };
//     Pointers::SharedPTR<int[5], SpecialDeleter> p{};

//     EXPECT_EQ(p, nullptr);
//     EXPECT_EQ(p.get(), nullptr);
// }


TEST(ConstructorWithRawPointer, Nullptr)
{
    Pointers::SharedPTR<int> p{nullptr};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(ConstructorWithRawPointer, InitializedPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(MoveConstructor, EmptyPointer)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(MoveConstructor, PointerWithValue)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(CopyConstructor, EmptyPointer)
{
    Pointers::SharedPTR<int> p{};
    EXPECT_EQ(p.get(), nullptr);

    Pointers::SharedPTR p2 = p;

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(CopyConstructor, PointerWithValue)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);

    Pointers::SharedPTR p2 = p;

    EXPECT_EQ(p.get(), p2.get());
    EXPECT_EQ(p.count_refs(), 2);
}


TEST(AssigmentRawPointer, Nullptr)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    int* another_raw_p = new int(10);
    p = another_raw_p;
    EXPECT_EQ(p.get(), another_raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(AssigmentRawPointer, InitializedPtr)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    int* another_raw_p = new int(10);
    p = another_raw_p;
    EXPECT_EQ(p.get(), another_raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(AssigmentRawPointer, CheckDeletingMemory)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    bool another_delete_flag = false;
    TestClass* another_raw_p = new TestClass(&another_delete_flag);
    p = another_raw_p;

    EXPECT_EQ(delete_flag, true);
    EXPECT_EQ(another_delete_flag, false);

    EXPECT_EQ(p.get(), another_raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(AssigmentRawPointer, OwnRawPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    p = raw_p;

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(MoveAssigment, EmptyPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    int* another_raw_p = new int(10);
    Pointers::SharedPTR<int> p2{another_raw_p};
    EXPECT_EQ(p2.get(), another_raw_p);

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(MoveAssigment, PointerWithValue)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    int* another_raw_p = new int(10);
    Pointers::SharedPTR<int> p2{another_raw_p};
    EXPECT_EQ(p2.get(), another_raw_p);

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(MoveAssigment, CheckDeletingMemory)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    bool another_delete_flag = false;
    TestClass* another_raw_p = new TestClass(&another_delete_flag);
    Pointers::SharedPTR another_p{another_raw_p};
    EXPECT_EQ(another_p.get(), another_raw_p);

    another_p = std::move(p);

    EXPECT_EQ(delete_flag, false);
    EXPECT_EQ(another_delete_flag, false);
}


TEST(MoveAssigment, SelfMove)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    p = std::move(p);

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(CopyAssigment, EmptyPointer)
{
    Pointers::SharedPTR<int> p{};
    EXPECT_EQ(p.get(), nullptr);

    int* another_raw_p = new int(10);
    Pointers::SharedPTR<int> p2{another_raw_p};
    EXPECT_EQ(p2.get(), another_raw_p);

    p2 = p;

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);

    // А тут я проверяю что указатель p не поломался
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(CopyAssigment, PointerWithValue)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    int* another_raw_p = new int(10);
    Pointers::SharedPTR<int> p2{another_raw_p};
    EXPECT_EQ(p2.get(), another_raw_p);

    p2 = p;

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 2);

    // А тут я проверяю что указатель p не поломался
    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 2);
}


TEST(CopyAssigment, CheckDeletingMemory)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    bool another_delete_flag = false;
    TestClass* another_raw_p = new TestClass(&another_delete_flag);
    Pointers::SharedPTR another_p{another_raw_p};
    EXPECT_EQ(another_p.get(), another_raw_p);

    another_p = p;

    EXPECT_EQ(delete_flag, false);
    EXPECT_EQ(another_delete_flag, true);
}


TEST(CopyAssigment, SelfCopy)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    p = p;  // А вдруг компилятор выкидывает эту строчку кода???

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 1);
}



TEST(Destructor, DefaultDeleter)
{
    bool delete_flag = false;

    {
        TestClass* raw_p = new TestClass(&delete_flag);
        Pointers::SharedPTR p{raw_p};
        EXPECT_EQ(p.get(), raw_p);
    }

    EXPECT_EQ(delete_flag, true);
}


TEST(Destructor, CustomDeleter)
{
    bool deleted_obj_flag = false;
    CustomDeleter d{&deleted_obj_flag};

    {
        int* raw_p = new int(10);
        Pointers::SharedPTR p{raw_p, d};
        EXPECT_EQ(p.get(), raw_p);
    }

    EXPECT_EQ(deleted_obj_flag, true);
}


TEST(Dereference, ByAsterisk)
{
    Pointers::SharedPTR p{new int(10)};
    EXPECT_EQ(*p, 10);
}


TEST(Dereference, ByArrow)
{
    std::vector<int>* v = new std::vector<int>();
    v->push_back(1);
    v->push_back(2);
    v->push_back(3);
    Pointers::SharedPTR p{v};
    EXPECT_EQ(p->size(), 3);
}


TEST(GettingPointer, GettingPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR p{raw_p};
    EXPECT_EQ(p.get(), raw_p);
}


TEST(OperatorBool, EmptyPointer)
{
    Pointers::SharedPTR<float> p{};
    EXPECT_EQ(p, false);
}


TEST(OperatorBool, PointerWithValue)
{
    Pointers::SharedPTR p{new float(5.0f)};
    EXPECT_EQ(p, true);
}


TEST(RefCounter, RefCounter)
{
    Pointers::SharedPTR<int> p1{};
    EXPECT_EQ(p1.count_refs(), 0);
    
    std::cout << 1 << ' ' << std::endl;

    p1 = new int(5);
    std::cout << 2 << ' ' << std::endl;
    EXPECT_EQ(p1.count_refs(), 1);
    std::cout << 3 << ' ' << std::endl;


    Pointers::SharedPTR p2 = p1;
    EXPECT_EQ(p1.count_refs(), 2);


    {
        Pointers::SharedPTR p3 = p1;
        EXPECT_EQ(p1.count_refs(), 3);
    }

    std::cout << 5 << ' ' << std::endl;

    EXPECT_EQ(p1.count_refs(), 2);

    std::cout << 6 << ' ' << std::endl;

    p2 = nullptr;
    EXPECT_EQ(p1.count_refs(), 1);
}


TEST(Release, EmptyPointer)
{
    Pointers::SharedPTR<TestClass> p{};

    EXPECT_EQ(p.get(), nullptr);

    p.release();

    EXPECT_EQ(p.get(), nullptr);
}


TEST(Release, PointerWithValue)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(delete_flag, false);

    p.release();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(delete_flag, true);
}


TEST(Reset, WithoutParameters)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(delete_flag, false);

    p.reset();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(delete_flag, true);
}


TEST(Reset, WithParameters)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(delete_flag, false);

    bool another_delete_flag = false;
    TestClass* another_raw_p = new TestClass(&another_delete_flag);
    p.reset(another_raw_p);

    EXPECT_EQ(p.get(), another_raw_p);
    EXPECT_EQ(delete_flag, true);
    EXPECT_EQ(another_delete_flag, false);
}


TEST(Swap, Swap)
{
    bool delete_flag = false;
    TestClass* raw_p = new TestClass(&delete_flag);
    Pointers::SharedPTR p{raw_p};

    bool another_delete_flag = false;
    TestClass* another_raw_p = new TestClass(&another_delete_flag);
    Pointers::SharedPTR another_p{another_raw_p};

    p.swap(another_p);

    EXPECT_EQ(p.get(), another_raw_p);
    EXPECT_EQ(another_p.get(), raw_p);

    EXPECT_EQ(delete_flag, false);
    EXPECT_EQ(another_delete_flag, false);
}


// TEST(Comparing, SameRawPointers)
// {
//     int* raw_p = new int(10);
//     Pointers::SharedPTR p{raw_p};
//     Pointers::SharedPTR another_p{raw_p};

//     EXPECT_NE(p, another_p);
// }


TEST(Comparing, CopiedPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR p{raw_p};
    Pointers::SharedPTR another_p{p};

    EXPECT_EQ(p, another_p);
}


TEST(MakeShared, Empty)
{
    Pointers::SharedPTR<int> p = Pointers::make_shared<int>();
    EXPECT_EQ(p.get(), nullptr);
}


TEST(MakeShared, WithParameters)
{
    Pointers::SharedPTR<Point> p = Pointers::make_shared<Point>(1, 2);
    EXPECT_EQ(p->x(), 1);
    EXPECT_EQ(p->y(), 2);
}


TEST(MakeShared, FromAnotherSharedPointer)
{
    int counter = 0;
    Pointers::SharedPTR<TestClassWithCounter> p = Pointers::make_shared<TestClassWithCounter>(&counter);
    EXPECT_EQ(counter, 1);

    Pointers::SharedPTR<TestClassWithCounter> another_p = Pointers::make_shared<TestClassWithCounter>(p);
    EXPECT_EQ(counter, 1);

    EXPECT_EQ(p.count_refs(), 2);
    EXPECT_EQ(p.get(), another_p.get());
}