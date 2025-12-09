#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "shared_pointer.hpp"


/*
Обратите пожалуйста внимание на тест (строка 572):
TEST(Assigments, MoveIdentical)

Сейчас он падает, поскольку count_ref() возвращает не 1, а 2.
То есть перемещение не уничтожает перемещаемый указатель.
Является ли это проблемой? Мне кажется, что это как минимум
неочевидное поведение.
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
        MemoryChecker(const MemoryChecker&) { ++m_copy_ctors; }
        MemoryChecker(MemoryChecker&&) { ++m_move_ctors; }
        ~MemoryChecker() { ++m_dtors; }
        // MemoryChecker& operator=(int* ) { ++m_copy_assign; return *this; }
        // inline static size_t m_copy_assign = 0;
        inline static size_t m_ctors = 0;
        inline static size_t m_copy_ctors = 0;
        inline static size_t m_move_ctors = 0;
        inline static size_t m_dtors = 0;

        static void ResetCounts() { m_ctors = m_copy_ctors = m_move_ctors = m_dtors = 0; }
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

    class ThrowingConstructor
    {
    public:
        ThrowingConstructor(bool should_throw)
        {
            MemoryChecker::m_ctors++;
            if (should_throw) {
                throw std::runtime_error("Constructor failed as requested.");
            }
        }
        ~ThrowingConstructor() { MemoryChecker::m_dtors++; }
    };

    // // Класс для тестирования кастомного аллокатора
    // template<typename T>
    // struct TrackingAllocator : public std::allocator<T>
    // {
    //     using size_type = size_t;
    //     using value_type = T;
    //     using propagate_on_container_copy_assignment = std::true_type;
    //     using propagate_on_container_move_assignment = std::true_type;
    //     using propagate_on_container_swap = std::true_type;

    //     inline static size_t allocations = 0;
    //     inline static size_t deallocations = 0;
    //     inline static size_t bytes_allocated = 0;
    //     inline static size_t bytes_deallocated = 0;

    //     TrackingAllocator() = default;

    //     template<class U>
    //     TrackingAllocator(const TrackingAllocator<U>&) noexcept {}

    //     T* allocate(size_t n)
    //     {
    //         allocations++;
    //         bytes_allocated += n * sizeof(T);
    //         return std::allocator<T>().allocate(n);
    //     }

    //     void deallocate(T* p, size_t n)
    //     {
    //         deallocations++;
    //         bytes_deallocated += n * sizeof(T);
    //         std::allocator<T>().deallocate(p, n);
    //     }

    //     static void ResetCounts() { allocations = deallocations = bytes_allocated = bytes_deallocated = 0; }

    //     // Требуется для rebind, если используется std::allocator_traits
    //     template<class U>
    //     struct rebind { using other = TrackingAllocator<U>; };
    // };

    struct AllocatorStats {
        inline static size_t allocations = 0;
        inline static size_t deallocations = 0;
        inline static size_t bytes_allocated = 0;
        inline static size_t bytes_deallocated = 0;
        static void ResetCounts() { allocations = deallocations = bytes_allocated = bytes_deallocated = 0; }
    };

    template<typename T>
    struct TrackingAllocator : public std::allocator<T>, public AllocatorStats // Наследуем счетчики
    {
        // ...
        TrackingAllocator() = default;

        template<class U>
        TrackingAllocator(const TrackingAllocator<U>& other) noexcept 
            : AllocatorStats(other) // Копируем статистику (не обязательно, так как статические)
        {}

        T* allocate(size_t n)
        {
            // Используем счетчики из AllocatorStats
            AllocatorStats::allocations++; 
            AllocatorStats::bytes_allocated += n * sizeof(T);
            return std::allocator<T>().allocate(n);
        }

        void deallocate(T* p, size_t n)
        {
            // Используем счетчики из AllocatorStats
            AllocatorStats::deallocations++; 
            AllocatorStats::bytes_deallocated += n * sizeof(T);
            std::allocator<T>().deallocate(p, n);
        }

        template<class U>
        struct rebind { using other = TrackingAllocator<U>; };
    };

    class NonTrivial
    {
    public:
        inline static size_t ctor_count = 0;
        inline static size_t dtor_count = 0;
        NonTrivial() { ctor_count++; }
        ~NonTrivial() { dtor_count++; }
    };
    
}



//==============Constructors==============

TEST(Constructors, DefaultInt)
{
    Pointers::SharedPTR<int> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Constructors, DefaultArrayType)
{
    Pointers::SharedPTR<int[5], ArrayDeleter<int[5]>> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Constructors, DefaultUserDefinedType)
{
    Pointers::SharedPTR<Point> p{};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Constructors, DefaultMemoryManagment)
{
    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR<MemoryChecker> p{};

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Constructors, RawPointerNull)
{
    Pointers::SharedPTR<int> p{nullptr};

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Constructors, RawPointerAssigned)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(*p, 10);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(Constructors, RawPointerMemoryManagment)
{
    MemoryChecker* raw = new MemoryChecker{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR p{raw};

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Constructors, RawPointerArrayType)
{
    MemoryChecker* raw = new MemoryChecker[10]{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Constructors, MoveEmpty)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
}


TEST(Constructors, MoveAssigned)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(*p2, 10);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(Constructors, MoveMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Constructors, CopyEmpty)
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


TEST(Constructors, CopyAssigned)
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


TEST(Constructors, CopyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    Pointers::SharedPTR p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}



//==============Assigments==============

TEST(Assigments, MoveEmptyToEmpty)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR<int> p2{};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(Assigments, MoveEmptyToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    Pointers::SharedPTR<MemoryChecker> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveEmptyToEmptyArrayType)
{
    Pointers::SharedPTR<MemoryChecker[], ArrayDeleter<MemoryChecker[]>> p{};

    Pointers::SharedPTR<MemoryChecker[], ArrayDeleter<MemoryChecker[]>> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveAssignedToEmpty)
{
    int* raw_p = new int(51);

    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR<int> p2{};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 1);
    EXPECT_EQ(*p2, 51);
}


TEST(Assigments, MoveAssignedToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR<MemoryChecker> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveAssignedToEmptyArrayType)
{
    MemoryChecker* raw = new MemoryChecker[7]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveEmptyToAssigned)
{
    Pointers::SharedPTR<int> p{};
    
    int* raw_p = new int(51);
    
    Pointers::SharedPTR<int> p2{raw_p};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(Assigments, MoveEmptyToAssignedMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    Pointers::SharedPTR<MemoryChecker> p2{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveEmptyToAssignedArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{};

    MemoryChecker* raw = new MemoryChecker[7]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveAssignedToAssigned)
{
    int* raw = new int(1);

    Pointers::SharedPTR<int> p{raw};
    
    int* raw2 = new int(2);
    
    Pointers::SharedPTR<int> p2{raw2};

    p2 = std::move(p);

    EXPECT_EQ(p2.get(), raw);
    EXPECT_EQ(p2.count_refs(), 1);
    EXPECT_EQ(*p2, 1);
}


TEST(Assigments, MoveAssignedToAssignedMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR<MemoryChecker> p2{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveAssignedToAssignedArrayType)
{
    MemoryChecker* raw = new MemoryChecker[14]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    MemoryChecker* raw2 = new MemoryChecker[14]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveBigArrayToSmallArray)
{
    MemoryChecker* raw = new MemoryChecker[14]{};
    MemoryChecker* raw2 = new MemoryChecker[7]{};

    // Присваиваем больший к меньшему
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveSmallArrayToBigArray)
{
    MemoryChecker* raw = new MemoryChecker[14]{};
    MemoryChecker* raw2 = new MemoryChecker[7]{};

    // Присваиваем меньший к большему
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p3{raw};
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p4{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p3 = std::move(p4);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, MoveSelf)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(Assigments, MoveIdentical)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{raw};

    Pointers::SharedPTR<MemoryChecker> p2{p};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = std::move(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    EXPECT_EQ(p2.get(), raw);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(Assigments, RawEmptyToEmpty)
{
    double* raw = nullptr;

    Pointers::SharedPTR<double> p{};

    p = raw;

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Assigments, RawEmptyToEmptyMemoryManagment)
{
    MemoryChecker* raw = nullptr;

    Pointers::SharedPTR<MemoryChecker> p{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}



TEST(Assigments, RawAssignedToEmpty)
{
    double* raw = new double{5.0};

    Pointers::SharedPTR<double> p{};

    p = raw;

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
    EXPECT_EQ(*p, 5.0);
}



TEST(Assigments, RawAssignedToEmptyMemoryManagment)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, RawEmptyToAssigned)
{
    double* raw = nullptr;

    Pointers::SharedPTR<double> p{new double{5.0}};

    p = raw;

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Assigments, RawEmptyToAssignedMemoryManagment)
{
    MemoryChecker* raw = nullptr;

    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Assigments, RawEmptyToAssignedArrayType)
{
    MemoryChecker* raw = nullptr;

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[8]{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 8);

    EXPECT_EQ(p.get(), nullptr);
}


TEST(Assigments, RawAssignedToAssigned)
{
    double* raw = new double{3.0};

    Pointers::SharedPTR<double> p{new double{5.0}};

    p = raw;

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
    EXPECT_EQ(*p, 3.0);
}


TEST(Assigments, RawAssignedToAssignedMemoryManagment)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Assigments, RawAssignedToAssignedArrayType)
{
    MemoryChecker* raw = new MemoryChecker[8]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[15]{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = raw;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 15);

    EXPECT_EQ(p.get(), raw);
}


TEST(Assigments, RawTwiseAssign)
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


TEST(Assigments, CopyEmptyToEmpty)
{
    Pointers::SharedPTR<int> p{};

    Pointers::SharedPTR<int> p2{};

    p2 = p;

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(Assigments, CopyEmptyToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    Pointers::SharedPTR<MemoryChecker> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, CopyEmptyToEmptyArrayType)
{
    Pointers::SharedPTR<MemoryChecker[], ArrayDeleter<MemoryChecker[]>> p{};

    Pointers::SharedPTR<MemoryChecker[], ArrayDeleter<MemoryChecker[]>> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, CopyAssignedToEmpty)
{
    int* raw_p = new int(51);

    Pointers::SharedPTR<int> p{raw_p};

    Pointers::SharedPTR<int> p2{};

    p2 = p;

    EXPECT_EQ(p.get(), raw_p);
    EXPECT_EQ(p.count_refs(), 2);
    EXPECT_EQ(*p, 51);

    EXPECT_EQ(p2.get(), raw_p);
    EXPECT_EQ(p2.count_refs(), 2);
    EXPECT_EQ(*p2, 51);
}


TEST(Assigments, CopyAssignedToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR<MemoryChecker> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, CopyAssignedToEmptyArrayType)
{
    MemoryChecker* raw = new MemoryChecker[7]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Assigments, CopyEmptyToAssigned)
{
    Pointers::SharedPTR<int> p{};
    
    int* raw_p = new int(51);
    
    Pointers::SharedPTR<int> p2{raw_p};

    p2 = p;

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(Assigments, CopyEmptyToAssignedMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    Pointers::SharedPTR<MemoryChecker> p2{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Assigments, CopyEmptyToAssignedArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{};

    MemoryChecker* raw = new MemoryChecker[7]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 7);
}


TEST(Assigments, CopyAssignedToAssigned)
{
    int* raw = new int(1);

    Pointers::SharedPTR<int> p{raw};
    
    int* raw2 = new int(2);
    
    Pointers::SharedPTR<int> p2{raw2};

    p2 = p;

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 2);
    EXPECT_EQ(*p, 1);

    EXPECT_EQ(p2.get(), raw);
    EXPECT_EQ(p2.count_refs(), 2);
    EXPECT_EQ(*p2, 1);

    // проверяем, что обнуление указателя не влияет на другие указатели, 
    // обладающие тем же объектом

    p2 = nullptr;

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
    EXPECT_EQ(*p, 1);

    // проверяем, что обнуление указателя, от которого копируемся, не 
    // влияет на другие указатели, обладающие тем же объектом

    Pointers::SharedPTR<int> p3{};
    EXPECT_EQ(p3.get(), nullptr);

    p3 = p;

    p = nullptr;

    EXPECT_EQ(p3.get(), raw);
    EXPECT_EQ(p3.count_refs(), 1);
    EXPECT_EQ(*p3, 1);
}


TEST(Assigments, CopyAssignedToAssignedMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    Pointers::SharedPTR<MemoryChecker> p2{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Assigments, CopyAssignedToAssignedArrayType)
{
    MemoryChecker* raw = new MemoryChecker[14]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    MemoryChecker* raw2 = new MemoryChecker[14]{};

    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 14);
}


TEST(Assigments, CopyBigArrayToSmallArray)
{
    MemoryChecker* raw = new MemoryChecker[14]{};
    MemoryChecker* raw2 = new MemoryChecker[7]{};

    // Присваиваем больший к меньшему
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p2{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 7);
}


TEST(Assigments, CopySmallArrayToBigArray)
{
    MemoryChecker* raw = new MemoryChecker[14]{};
    MemoryChecker* raw2 = new MemoryChecker[7]{};

    // Присваиваем меньший к большему
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p3{raw};
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p4{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p3 = p4;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 14);
}


TEST(Assigments, CopySelf)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(Assigments, CopyIdentical)
{
    MemoryChecker* raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{raw};

    Pointers::SharedPTR<MemoryChecker> p2{p};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2 = p;

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    EXPECT_EQ(p2.get(), raw);
    EXPECT_EQ(p2.count_refs(), 2);

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 2);
}





//==============Destructor==============

TEST(Destructor, WithCustomDeleter)
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


TEST(Destructor, SinglePointer)
{
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR<MemoryChecker> p(new MemoryChecker{});

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, MemoryChecker::m_dtors);
}


TEST(Destructor, TwoPointers)
{
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR<MemoryChecker> p(new MemoryChecker{});

        {
            Pointers::SharedPTR<MemoryChecker> p2 = p;

            EXPECT_EQ(MemoryChecker::m_dtors, 0);
        }

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Destructor, ReverseOrderOfRemoving)
{
    MemoryChecker::m_dtors = 0;

    {
        Pointers::SharedPTR<MemoryChecker> p{};

        {
            Pointers::SharedPTR<MemoryChecker> p2(new MemoryChecker{});

            p = p2;

            EXPECT_EQ(MemoryChecker::m_dtors, 0);
        }

        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    }

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Destructor, ForArrayType)
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



//=================Access==================
TEST(Dereference, ByAsterisk)
{
    Pointers::SharedPTR p{new int(10)};
    EXPECT_EQ(*p, 10);
}


TEST(Dereference, ByAsteriskNullptr)
{
    Pointers::SharedPTR<int> p{};

    try {
        *p;
        FAIL() << "Ожидалось std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ(
            "Dereferencing nullptr: SharedPTR is nullptr.",
            e.what()
        );
    }
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


TEST(Dereference, ByArrowNullptr)
{
    std::vector<int>* v = nullptr;
    Pointers::SharedPTR p{v};

    try {
        p->push_back(1);
        FAIL() << "Ожидалось std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ(
            "Dereferencing nullptr: SharedPTR is nullptr.",
            e.what()
        );
    }
}


TEST(Dereference, GettingNullptr)
{
    Pointers::SharedPTR<int> p{};
    EXPECT_EQ(p.get(), nullptr);
}


TEST(Dereference, GettingPointer)
{
    int* raw_p = new int(10);
    Pointers::SharedPTR p{raw_p};
    EXPECT_EQ(p.get(), raw_p);
}


TEST(Dereference, RefCounter)
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


TEST(OperatorBool, ExplicitCast)
{
    Pointers::SharedPTR<int> p{};
    EXPECT_FALSE(static_cast<bool>(p));

    p = new int(10);
    EXPECT_TRUE(static_cast<bool>(p));
}


TEST(OperatorBool, InIf)
{
    Pointers::SharedPTR<int> p{};
    bool entered = false;
    if (p) entered = true;
    EXPECT_FALSE(entered);

    p = new int(1);
    if (p) entered = true;
    EXPECT_TRUE(entered);
}


TEST(Comparison, EqualSameObject)
{
    int* raw = new int(1);
    Pointers::SharedPTR<int> p1{raw};
    Pointers::SharedPTR<int> p2 = p1;

    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);
}

TEST(Comparison, EqualBothNull)
{
    Pointers::SharedPTR<int> p1{}, p2{};
    EXPECT_TRUE(p1 == p2);
}

TEST(Comparison, NotEqualDifferentObjects)
{
    Pointers::SharedPTR<int> p1{new int(1)};
    Pointers::SharedPTR<int> p2{new int(2)};
    EXPECT_FALSE(p1 == p2);
}

TEST(Comparison, NotEqualOneNull)
{
    Pointers::SharedPTR<int> p1{}, p2{new int(1)};
    EXPECT_FALSE(p1 == p2);
}




//=================Modifies==================

TEST(Swap, EmptyWithEmpty)
{
    Pointers::SharedPTR<int> p1{}, p2{};
    p1.swap(p2);
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2.get(), nullptr);
}


TEST(Swap, EmptyWithEmptyMemoryManagment)
{
    Pointers::SharedPTR<int> p1{}, p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p1.swap(p2);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Swap, EmptyWithAssigned)
{
    Pointers::SharedPTR<int> p1{};

    auto raw = new int(42);
    Pointers::SharedPTR<int> p2{raw};

    p1.swap(p2);

    EXPECT_EQ(p1.get(), raw);
    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p1.count_refs(), 1);
    EXPECT_EQ(p2.count_refs(), 0);
}


TEST(Swap, EmptyWithAssignedMemoryManagment)
{
    auto raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p1{raw}, p2{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p1.swap(p2);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Swap, AssignedWithAssigned)
{
    auto raw1 = new int(1);
    auto raw2 = new int(2);
    Pointers::SharedPTR<int> p1{raw1}, p2{raw2};

    p1.swap(p2);

    EXPECT_EQ(p1.get(), raw2);
    EXPECT_EQ(p2.get(), raw1);
    EXPECT_EQ(*p1, 2);
    EXPECT_EQ(*p2, 1);
    EXPECT_EQ(p1.count_refs(), 1);
    EXPECT_EQ(p2.count_refs(), 1);
}


TEST(Swap, AssignedWithAssignedMemoryManagment)
{
    auto raw1 = new MemoryChecker{};

    auto raw2 = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p1{raw1}, p2{raw2};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p1.swap(p2);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Swap, Self)
{
    Pointers::SharedPTR<int> p{new int(5)};
    p.swap(p);
    EXPECT_EQ(*p, 5);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(Swap, SelfMemoryManagment)
{
    auto raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.swap(p);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Swap, Identical)
{
    auto raw = new int(1);
    Pointers::SharedPTR<int> p1{raw}, p2{p1};

    p1.swap(p2);

    EXPECT_EQ(p1.get(), raw);
    EXPECT_EQ(p2.get(), raw);
    EXPECT_EQ(*p1, 1);
    EXPECT_EQ(*p2, 1);
    EXPECT_EQ(p1.count_refs(), 2);
    EXPECT_EQ(p2.count_refs(), 2);
}


TEST(Swap, IdenticalMemoryManagment)
{
    auto raw = new MemoryChecker{};

    Pointers::SharedPTR<MemoryChecker> p1{raw}, p2{p1};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p1.swap(p2);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Release, Empty)
{
    Pointers::SharedPTR<Point> p{};

    p.release();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Release, EmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Release, Assigned)
{
    auto raw = new int(1);
    Pointers::SharedPTR<int> p{raw};

    p.release();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Release, AssignedMemoryManagment)
{
    auto raw = new MemoryChecker{};
    Pointers::SharedPTR<MemoryChecker> p{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Release, AssignedArrayType)
{
    auto raw = new MemoryChecker[50]{};
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{raw};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 50);
}


TEST(Release, ManyOwners)
{
    auto raw = new int(1);
    Pointers::SharedPTR<int> p1{raw}, p2{p1}, p3{p1};

    p2.release();

    EXPECT_EQ(p1.get(), raw);
    EXPECT_EQ(p1.count_refs(), 2);
    EXPECT_EQ(*p1, 1);

    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_EQ(p2.count_refs(), 0);

    EXPECT_EQ(p3.get(), raw);
    EXPECT_EQ(p3.count_refs(), 2);
    EXPECT_EQ(*p3, 1);

    p1.release();

    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p1.count_refs(), 0);

    EXPECT_EQ(p3.get(), raw);
    EXPECT_EQ(p3.count_refs(), 1);
    EXPECT_EQ(*p3, 1);

    p3.release();

    EXPECT_EQ(p3.get(), nullptr);
    EXPECT_EQ(p3.count_refs(), 0);
}


TEST(Release, ManyOwnersMemoryManagment)
{
    auto raw = new MemoryChecker{};
    Pointers::SharedPTR<MemoryChecker> p1{raw}, p2{p1}, p3{p1};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p2.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    p1.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);

    p3.release();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Release, DoubleRelease)
{
    auto raw = new int(1);
    Pointers::SharedPTR<int> p{raw};

    p.release();
    p.release();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Reset, EmptyToEmpty)
{
    Pointers::SharedPTR<int> p{};

    p.reset();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Reset, EmptyToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Reset, EmptyToEmptyArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Reset, EmptyToAssigned)
{
    Pointers::SharedPTR<int> p{};

    int* raw = new int{10};

    p.reset(raw);

    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(p.count_refs(), 1);
    EXPECT_EQ(*p, 10);
}


TEST(Reset, EmptyToAssignedMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{};

    MemoryChecker* raw = new MemoryChecker{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset(raw);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Reset, EmptyToAssignedArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{};

    auto raw = new MemoryChecker[21]{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset(raw);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(Reset, AssignedToEmpty)
{
    Pointers::SharedPTR<int> p{new int{10}};

    p.reset();

    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.count_refs(), 0);
}


TEST(Reset, AssignedToEmptyMemoryManagment)
{
    Pointers::SharedPTR<MemoryChecker> p{new MemoryChecker{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Reset, AssignedToEmptyArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[21]{}};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset();

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 21);
}


TEST(Reset, AssignedToAssigned)
{
    int* raw1 = new int{1};

    Pointers::SharedPTR<int> p{raw1};

    int* raw2 = new int{2};

    p.reset(raw2);

    EXPECT_EQ(p.get(), raw2);
    EXPECT_EQ(p.count_refs(), 1);
    EXPECT_EQ(*p, 2);
}


TEST(Reset, AssignedToAssignedMemoryManagment)
{
    auto raw1 = new MemoryChecker{};

    Pointers::SharedPTR p{raw1};

    auto raw2 = new MemoryChecker{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset(raw2);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(Reset, AssignedToAssignedArrayType)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[21]{}};

    auto raw = new MemoryChecker[21]{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0; 

    p.reset(raw);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 21);
}


TEST(Reset, ArraySmallToBig)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[10]{}};

    auto raw = new MemoryChecker[20]{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0; 

    p.reset(raw);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 10);
}


TEST(Reset, ArrayBigToSmall)
{
    Pointers::SharedPTR<MemoryChecker, ArrayDeleter<MemoryChecker>> p{new MemoryChecker[30]{}};

    auto raw = new MemoryChecker[15]{};

    MemoryChecker::m_ctors = 0;
    MemoryChecker::m_copy_ctors = 0;
    MemoryChecker::m_move_ctors = 0;
    MemoryChecker::m_dtors = 0;

    p.reset(raw);

    EXPECT_EQ(MemoryChecker::m_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_dtors, 30);
}


TEST(Reset, SamePointerThrowsException)
{
    int* raw = new int(123);
    Pointers::SharedPTR<int> p(raw);
    
    try {
        p.reset(raw);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ("SharedPTR already have this pointer.", e.what());
    }
    
    // Указатель должен остаться валидным
    EXPECT_EQ(p.get(), raw);
    EXPECT_EQ(*p, 123);
    EXPECT_EQ(p.count_refs(), 1);
}


TEST(MakeShared, WithParameters)
{
    Pointers::SharedPTR<Point> p = Pointers::make_shared<Point>(1, 2);
    EXPECT_EQ(p->x(), 1);
    EXPECT_EQ(p->y(), 2);
}












//========================================================================
//===                       allocate_shared()                          ===
//========================================================================
TEST(AllocateShared, Initialization_SimpleType)
{
    // Используем Pointers::allocate_shared, так как оно в Pointers
    auto p = Pointers::allocate_shared<int>({}, 42);

    ASSERT_NE(p.get(), nullptr);
    EXPECT_EQ(*p, 42);
    EXPECT_EQ(p.count_refs(), 1);
}

TEST(AllocateShared, Initialization_ComplexObject)
{
    auto p = Pointers::allocate_shared<Point>({}, 10, 20);

    ASSERT_NE(p.get(), nullptr);
    EXPECT_EQ(p->x(), 10);
    EXPECT_EQ(p->y(), 20);
    EXPECT_EQ(p.count_refs(), 1);
}

TEST(AllocateShared, RefCounting_Basic)
{
    auto p1 = Pointers::allocate_shared<int>({}, 100);

    EXPECT_EQ(p1.count_refs(), 1);

    {
        auto p2 = Pointers::allocate_shared<int>({}, 0);
        p2 = p1;
        EXPECT_EQ(p1.count_refs(), 2);
        EXPECT_EQ(p2.count_refs(), 2);
    } // p2 уничтожается

    EXPECT_EQ(p1.count_refs(), 1);
}

TEST(AllocateShared, DestructorCallOnLastRef)
{
    MemoryChecker::ResetCounts();

    {
        auto p = Pointers::allocate_shared<MemoryChecker>({});
        EXPECT_EQ(MemoryChecker::m_ctors, 1);
        EXPECT_EQ(MemoryChecker::m_dtors, 0);
    } // p уничтожается

    EXPECT_EQ(MemoryChecker::m_dtors, 1);
}


TEST(AllocateShared, PerfectForwarding)
{
    auto p1 = Pointers::allocate_shared<int>({}, 123);
    EXPECT_EQ(*p1, 123);
}


TEST(AllocateShared, PerfectForwardingMemoryManagment)
{
    MemoryChecker::ResetCounts();

    auto p1 = Pointers::allocate_shared<MemoryChecker>({});

    // Должно быть только прямое конструирование
    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    EXPECT_EQ(MemoryChecker::m_copy_ctors, 0);
    EXPECT_EQ(MemoryChecker::m_move_ctors, 0);

    MemoryChecker::ResetCounts();
}


TEST(AllocateShared, ExceptionSafety)
{
    MemoryChecker::ResetCounts();

    // Используем ThrowingConstructor из анонимного namespace
    EXPECT_THROW(
        Pointers::allocate_shared<ThrowingConstructor>({}, true),
        std::runtime_error
    );

    // ThrowingConstructor инкрементирует MemoryChecker::m_ctors
    EXPECT_EQ(MemoryChecker::m_ctors, 1);
    // Деструктор не должен был вызываться
    EXPECT_EQ(MemoryChecker::m_dtors, 0);
}


TEST(AllocateShared, CustomAllocator)
{
    using MyType = double;
    using Alloc = TrackingAllocator<MyType>; // Используем TrackingAllocator из анонимного namespace

    Alloc::ResetCounts();

    // 1. Создаем конкретный экземпляр аллокатора для отслеживания
    Alloc alloc_instance; 

    {
        // 2. Передаем этот экземпляр
        auto p = Pointers::allocate_shared<MyType>(alloc_instance, 3.14);

        ASSERT_NE(p.get(), nullptr);
        
        // Теперь EXPECT_EQ проверяет статические счетчики, 
        // которые были инкрементированы в allocate_shared через копию
        // alloc_instance или через rebind, но все равно через статические поля.
        EXPECT_EQ(Alloc::allocations, 1); 
        EXPECT_EQ(Alloc::deallocations, 0);
    } // p уничтожается

    EXPECT_EQ(Alloc::deallocations, 1);
    EXPECT_EQ(Alloc::bytes_allocated, Alloc::bytes_deallocated);
}


TEST(AllocateShared, UsesDestructorOnly)
{
    NonTrivial::ctor_count = 0;
    NonTrivial::dtor_count = 0;

    {
        // Используем NonTrivial из анонимного namespace
        auto p = Pointers::allocate_shared<NonTrivial>({});
        EXPECT_EQ(NonTrivial::ctor_count, 1);
        EXPECT_EQ(NonTrivial::dtor_count, 0);
    } // p уничтожается

    EXPECT_EQ(NonTrivial::dtor_count, 1);
}