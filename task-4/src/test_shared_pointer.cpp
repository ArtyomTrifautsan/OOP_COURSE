#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "shared_pointer.hpp"


/*
Received feedback from the teacher - commit
*/


namespace {

    template <typename Type>
    struct ArrayDeleter
    {
        void operator()(Type* ptr)
        {
            delete[] ptr;
        }
    };

    struct MemoryChecker
    {
        MemoryChecker() { ++m_ctors; }
        ~MemoryChecker() { ++m_dtors; }
        inline static int m_ctors = 0;
        inline static int m_dtors = 0;
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
    
}



//==============Constructors==============

TEST(SharedPTRConstructors, DefaultConstructorForInt)
{
    Pointers::SharedPTR<int> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(SharedPTRConstructors, DefaultConstructorForArrayType)
{
    Pointers::SharedPTR<int[5], ArrayDeleter<int[5]>> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(SharedPTRConstructors, DefaultConstructorForUserDefinedType)
{
    Pointers::SharedPTR<Point> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(SharedPTRConstructors, ConstructFromNullRawPointer)
{
    Pointers::SharedPTR<int> p{nullptr};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(SharedPTRConstructors, ConstructFromRawPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(*p, 10);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(SharedPTRConstructors, MoveConstructorWithEmptyPointer)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
}


TEST(SharedPTRConstructors, MoveConstructorWithNonEmptyPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(*p2, 10);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(SharedPTRConstructors, MoveConstructorMemoryManagment)
{
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(SharedPTRConstructors, CopyConstructorForEmptypointer)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR<int> p2 = p;

    // Проверяем первый указатель
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);

    // проверяем второй указатель
    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(SharedPTRConstructors, CopyConstructorForNonEmptyPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR<int> p2 = p;

    // Проверяем первый указатель
    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(*p, 10);
    EXPECT_EQ(p.count_refs(), 2);

    // проверяем второй указатель
    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(*p2, 10);
    EXPECT_EQ(p2.count_refs(), 2);

    // Проверяем что изменение одного указателя влияет на изменение другого
    *p2 = 20;

    EXPECT_EQ(*p, 20);
}


TEST(SharedPTRConstructors, CopyConstructorMemoryManagment)
{
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}



//==============Assigments==============

TEST(SharedPTRAssigments, MoveAssigmentEmptyPointerToEmptyPointer)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR<int> p2{};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(SharedPTRAssigments, MoveAssigmentValuePointerToEmptyPointer)
{
    /*
    (Assigments, MoveAssignedToEmpty)
    В названии теста ValuePointer имеется ввиду что указатель имеет какое-то не nullptr значение
    */

    int* raw_p = new int(51);

    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR<int> p2{};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 1);
    EXPECT_EQ(*p2, 51);
}


TEST(SharedPTRAssigments, MoveAssigmentValuePointerToEmptyPointerMemoryManagment)
{
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    MemoryChecker* raw_p = new MemoryChecker{};
    Pointers::SharedPTR<MemoryChecker> p{};

    Pointers::SharedPTR<MemoryChecker> p2{};

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


// TEST(SharedPTRAssigments, MoveAssigmentValuePointerToEmptyPointerMemoryManagment)
// {
//     MemoryChecker* raw_p = new MemoryChecker{};
//     Pointers::SharedPTR<MemoryChecker> p{};

//     Pointers::SharedPTR<MemoryChecker> p2{};

//     p2 = std::move(p);

//     EXPECT_EQ(p2.get(), raw_p);
//     EXPECT_EQ(p2.count_refs(), 1);
// }



//==============Destructor==============

TEST(SharedPTRDestructor, WithCustomDeleter)
{
    bool deleted = false;

    {
        auto* raw = new int(100);
        Pointers::SharedPTR<int, CustomDeleter> p(raw, CustomDeleter{&deleted});

        EXPECT_EQ(*p, 100);
        EXPECT_FALSE(deleted);
    }

    EXPECT_TRUE(deleted);
}


TEST(SharedPTRDestructor, SinglePointer)
{
    MemoryChecker::m_dtors = 0;

    {
        auto* raw = new int(100);
        Pointers::SharedPTR<MemoryChecker> p(new MemoryChecker{});

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(SharedPTRDestructor, TwoPointers)
{
    MemoryChecker::m_dtors = 0;

    {
        int* raw = new int(100);
        Pointers::SharedPTR<MemoryChecker> p(new MemoryChecker{});

        {
            Pointers::SharedPTR<MemoryChecker> p2 = p;

            EXPECT_EQ(MemoryChecker::m_dtors, 0);
        }

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(SharedPTRDestructor, ReverseOrderOfRemoving)
{
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR<MemoryChecker> p{};

        {
            int* raw = new int(100);
            Pointers::SharedPTR<MemoryChecker> p2(new MemoryChecker{});

            p = p2;

            EXPECT_EQ(MemoryChecker::m_dtors, 0);
        }

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(SharedPTRDestructor, ForArrayType)
{
    MemoryChecker::m_dtors = 0;

    {
        MemoryChecker* raw = new MemoryChecker[5]();
        Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p(raw);
        // Pointers::SharedPTR<MemoryChecker> p(raw);

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 5);
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
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR p{new MemoryChecker()};

        EXPECT_EQ(p.count_refs(), 1);
        EXPECT_EQ(MemoryChecker::m_ctors, 1);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);

        p = new MemoryChecker();

        EXPECT_EQ(MemoryChecker::m_ctors, 2);
        EXPECT_EQ(MemoryChecker::m_dtors, 1);
    }

    EXPECT_EQ(MemoryChecker::m_ctors, 2);
    EXPECT_EQ(MemoryChecker::m_dtors, 2);
}


TEST(AssigmentRawPointer, TwiseAssign)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};
    EXPECT_EQ(p.get(), raw_p);

    try {
        p = raw_p;
        FAIL() << "Ожидалось std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(
            "SharedPTR already have this pointer.",
            e.what()
        );
    }
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
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR p{new MemoryChecker()};

        EXPECT_EQ(MemoryChecker::m_ctors, 1);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);

        Pointers::SharedPTR another_p{new MemoryChecker()};

        EXPECT_EQ(MemoryChecker::m_ctors, 2);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);

        another_p = std::move(p);

        EXPECT_EQ(MemoryChecker::m_ctors, 2);
        EXPECT_EQ(MemoryChecker::m_dtors, 1);
    }

    EXPECT_EQ(MemoryChecker::m_ctors, 2);
    EXPECT_EQ(MemoryChecker::m_dtors, 2);
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
    MemoryChecker::m_ctors = 0; 
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR p{new MemoryChecker()};
        EXPECT_EQ(p.count_refs(), 1);

        EXPECT_EQ(MemoryChecker::m_ctors, 1);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);

        Pointers::SharedPTR another_p{new MemoryChecker()};
        EXPECT_EQ(another_p.count_refs(), 1);

        EXPECT_EQ(MemoryChecker::m_ctors, 2);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);

        another_p = p;

        EXPECT_EQ(MemoryChecker::m_ctors, 2);
        EXPECT_EQ(MemoryChecker::m_dtors, 1);
    }

    EXPECT_EQ(MemoryChecker::m_ctors, 2);
    EXPECT_EQ(MemoryChecker::m_dtors, 2);
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
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR p{new MemoryChecker()};
        EXPECT_EQ(p.count_refs(), 1);
    }

    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
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

    p1 = new int(5);

    EXPECT_EQ(p1.count_refs(), 1);

    Pointers::SharedPTR p2 = p1;
    EXPECT_EQ(p1.count_refs(), 2);

    {
        Pointers::SharedPTR p3 = p1;
        EXPECT_EQ(p1.count_refs(), 3);
    }

    EXPECT_EQ(p1.count_refs(), 2);

    p2 = nullptr;
    EXPECT_EQ(p1.count_refs(), 1);
}


TEST(Release, EmptyPointer)
{
    Pointers::SharedPTR<Point> p{};

    EXPECT_EQ(p.get(), nullptr);

    p.release();

    EXPECT_EQ(p.get(), nullptr);
}


// TEST(Release, PointerWithValue)
// {
//     bool delete_flag = false;
//     TestClass* raw_p = new TestClass(&delete_flag);
//     Pointers::SharedPTR p{raw_p};

//     EXPECT_EQ(p.get(), raw_p);
//     EXPECT_EQ(delete_flag, false);

//     p.release();

//     EXPECT_EQ(p.get(), nullptr);
//     EXPECT_EQ(delete_flag, true);
// }


// TEST(Reset, WithoutParameters)
// {
//     bool delete_flag = false;
//     TestClass* raw_p = new TestClass(&delete_flag);
//     Pointers::SharedPTR p{raw_p};

//     EXPECT_EQ(p.get(), raw_p);
//     EXPECT_EQ(delete_flag, false);

//     p.reset();

//     EXPECT_EQ(p.get(), nullptr);
//     EXPECT_EQ(delete_flag, true);
// }


// TEST(Reset, WithParameters)
// {
//     bool delete_flag = false;
//     TestClass* raw_p = new TestClass(&delete_flag);
//     Pointers::SharedPTR p{raw_p};

//     EXPECT_EQ(p.get(), raw_p);
//     EXPECT_EQ(delete_flag, false);

//     bool another_delete_flag = false;
//     TestClass* another_raw_p = new TestClass(&another_delete_flag);
//     p.reset(another_raw_p);

//     EXPECT_EQ(p.get(), another_raw_p);
//     EXPECT_EQ(delete_flag, true);
//     EXPECT_EQ(another_delete_flag, false);
// }


// TEST(Swap, Swap)
// {
//     bool delete_flag = false;
//     TestClass* raw_p = new TestClass(&delete_flag);
//     Pointers::SharedPTR p{raw_p};

//     bool another_delete_flag = false;
//     TestClass* another_raw_p = new TestClass(&another_delete_flag);
//     Pointers::SharedPTR another_p{another_raw_p};

//     p.swap(another_p);

//     EXPECT_EQ(p.get(), another_raw_p);
//     EXPECT_EQ(another_p.get(), raw_p);

//     EXPECT_EQ(delete_flag, false);
//     EXPECT_EQ(another_delete_flag, false);
// }


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


// TEST(MakeShared, Empty)
// {
//     Pointers::SharedPTR<int> p = Pointers::make_shared<int>();
//     EXPECT_EQ(p.get(), nullptr);
// }


TEST(MakeShared, WithParameters)
{
    Pointers::SharedPTR<Point> p = Pointers::make_shared<Point>(1, 2);
    EXPECT_EQ(p->x(), 1);
    EXPECT_EQ(p->y(), 2);
}



TEST(Intellect, Huge)
{
    struct GeniusStruct
    {
        bool method(const GeniusStruct* other) { return this == other; }
    };
    
    GeniusStruct g{};
    EXPECT_TRUE(g.method(&g));
}



// TEST(MakeShared, FromAnotherSharedPointer)
// {
//     int counter = 0;
//     Pointers::SharedPTR<TestClassWithCounter> p = Pointers::make_shared<TestClassWithCounter>(&counter);
//     EXPECT_EQ(counter, 1);

//     Pointers::SharedPTR<TestClassWithCounter> another_p = Pointers::make_shared<TestClassWithCounter>(p);
//     EXPECT_EQ(counter, 1);

//     EXPECT_EQ(p.count_refs(), 2);
//     EXPECT_EQ(p.get(), another_p.get());
// }