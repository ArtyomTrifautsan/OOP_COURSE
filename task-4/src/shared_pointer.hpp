#pragma once

#include <utility>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <new>


namespace Pointers {

    // template <typename Type, typename AllocatorType, typename... Args>
    // requires (!std::is_array_v<Type>)
    // auto allocate_shared(const AllocatorType& alloc, Args&&... args);

    namespace detail
    {

        // template <typename Type, typename TDeleter = std::default_delete<Type>, typename TAllocator = std::allocator<Type>>
        


        template <typename Type>
        struct ObjectDestructor
        {
            void operator()(Type* ptr) { ptr->~Type(); }
        };


        template <typename Type>
        struct ArrayDestructor
        {
            void operator()(Type* ptr, size_t N)
            {
                for (size_t i = 0; i < N; i++)
                    ptr[N - 1 - i].~Type();
            }
        };


        static size_t align_up(size_t size, size_t alignment) {
            return (size + alignment - 1) & ~(alignment - 1);
        }

    }


    // template <typename Type, typename... Args>
    // requires (!std::is_array_v<Type>)
    // auto make_shared(Args&&... args);

    

    template<typename Type, typename TDeleter = std::default_delete<Type>>
    class SharedPTR {
    public:
        using t_SharedPTR = SharedPTR<Type, TDeleter>;
        using PureType = std::conditional_t<std::is_array_v<Type>, std::remove_extent_t<Type>, Type>;

    public:
        class ControlBlock
        {
        public:

            ControlBlock(PureType* _obj, TDeleter del, bool _monolithic, bool _array_type)
                : m_obj{_obj}, m_deleter{del}, m_monolithic{_monolithic}, m_array_type{_array_type}, m_ref_counter{1} {}
            ~ControlBlock() = default;

            void add_ref() noexcept { ++m_ref_counter; }
            void remove_ref() noexcept { --m_ref_counter; }

            Type* obj() noexcept { return m_obj; }
            TDeleter deleter() noexcept { return m_deleter; }
            size_t refs() const noexcept { return m_ref_counter; }
            bool monolithic() const noexcept { return m_monolithic; }
            bool array_type() const noexcept { return m_array_type; }

        private:
            Type* m_obj = nullptr;
            TDeleter m_deleter;
            size_t m_ref_counter = 0;
            bool m_monolithic = false;
            bool m_array_type = false;
        };

    private:
        // Этот конструктор используется функцией make_shared()
        SharedPTR(ControlBlock* cb, bool is_array_type) : m_control_block{cb}
        {
            
        }

    public: // Constructors and destructor.
        SharedPTR() noexcept = default;

        SharedPTR(PureType* pObj, TDeleter deleter_ = {})
        {
            if (pObj == nullptr)
                return;

            m_control_block = new ControlBlock{pObj, deleter_, false, std::is_array_v<Type>};
        }

        SharedPTR(t_SharedPTR&& other) noexcept
        {
            std::swap(m_control_block, other.m_control_block);
        }

        SharedPTR(const t_SharedPTR& other)
        {
            if (other.m_control_block == nullptr) return;

            m_control_block = other.m_control_block;

            m_control_block->add_ref();
        }

        ~SharedPTR()
        {
            if (m_control_block == nullptr) return;

            release();
        }

    public: // Assignment.
        t_SharedPTR& operator=(t_SharedPTR&& other) noexcept
        {
            if (this == &other) return *this;

            std::swap(m_control_block, other.m_control_block);

            return *this;
        }

        t_SharedPTR& operator=(PureType *pObj)
        {
            reset(pObj);

            return *this;
        }

        t_SharedPTR& operator=(const t_SharedPTR& other)
        {
            if (this == &other) return *this;

            if (m_control_block == other.m_control_block) return *this;

            release();

            if (other.m_control_block != nullptr)
            {
                m_control_block = other.m_control_block;
                m_control_block->add_ref();
            }

            return *this;
        }

    public: // Observers.
    // Dereference the stored pointer.

        PureType& operator*() const requires (!std::is_array_v<Type>)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return *(m_control_block->obj());
        }

        // Return the stored pointer.
        PureType* operator->() const requires (!std::is_array_v<Type>)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return m_control_block->obj();
        }

        // Type& operator[](size_t index) const requires (std::is_array_v<Type>)
        // {
        //     check_valid();
        //     return m_obj[index];
        // }

        // Return the stored pointer.
        PureType* get() const
        {
            return m_control_block->obj();
        }

        // Return false if the stored pointer is null.
        operator bool() const { return m_control_block != nullptr; }

        friend bool operator==(const t_SharedPTR& first, const t_SharedPTR& second)
        {
            return first.m_control_block == second.m_control_block;
        }

        friend bool operator!=(const t_SharedPTR& first, const t_SharedPTR& second)
        {
            return !(first == second);
        }

        int count_refs() const noexcept
        {
            if (m_control_block == nullptr) return 0;

            return m_control_block->refs();
        }

    public: // Modifiers.
        // Release ownership of any stored pointer.
        void release()
        {
            if (m_control_block == nullptr) return;

            m_control_block->remove_ref();

            if (m_control_block->refs() == 0)
            {
                if (m_control_block->monolithic())
                    monolithic_delete();
                else
                    split_delete();
            }

            m_control_block = nullptr;
        }

        // Replace the stored pointer.
        void reset(PureType *pObj = nullptr)
        {
            // кинуть если передали тот же самый указатель который уже хранится
            if (m_control_block != nullptr && pObj == m_control_block->obj())
                throw std::runtime_error("SharedPTR already have this pointer.");

            TDeleter del = m_control_block->deleter();

            release();

            if (pObj != nullptr)
            {
                m_control_block = new ControlBlock(pObj, del, false, std::is_array_v<Type>);
            }
        }

        // Exchange the pointer with another object.
        void swap(t_SharedPTR& other)
        {
            std::swap(m_control_block, other.m_control_block);
        }

    private:
        ControlBlock* m_control_block = nullptr;

        void monolithic_delete()
        {
            // Я запретил создавать ArrayType указатели через make_shared!
            // Я знаю как это сделать, но уже некогда с этим баловаться. 

            PureType* obj = m_control_block->obj();
            if (obj != nullptr)
                obj->~PureType();

            m_control_block->~ControlBlock();

            // Так как я выделял память для char, я решил привести указатель к типу char*,
            // прежде чем освободить память. Просто на всякий случай. 
            char* char_ptr = reinterpret_cast<char*>(m_control_block);
            operator delete(char_ptr);
        }

        void split_delete()
        {
            Type* obj = m_control_block->obj();
            TDeleter del = m_control_block->deleter();
            if (obj != nullptr)
            {
                del(obj);
            }

            delete m_control_block;
        }

    public:
        template <typename T, typename... Args>
        requires (!std::is_array_v<T>)
        friend auto make_shared(Args&&... args);


    };


    template <typename Type, typename... Args>
    requires (!std::is_array_v<Type>)
    auto make_shared(Args&&... args)
    {
        using CBT = SharedPTR<Type>::ControlBlock;
        size_t total_size = sizeof(CBT) + sizeof(Type);
        void* raw_memory = nullptr;
        Type* obj = nullptr;
        CBT* control_block = nullptr;

        try
        {
            raw_memory = operator new(total_size);
            char* char_memory = static_cast<char*>(raw_memory);
            obj = new (char_memory + sizeof(CBT)) Type{std::forward<Args>(args)...};
            control_block = new (char_memory) CBT{obj, {}, true, false};
        }
        catch (...)
        {
            if (obj != nullptr)
                obj->~Type();

            if (control_block != nullptr)
                control_block->~ControlBlock();

            if (raw_memory != nullptr)
                operator delete(raw_memory);

            throw;
        }

        return SharedPTR<Type>{control_block, false};
    }


    template <typename Type, typename AllocatorType = std::allocator<Type>, typename... Args>
    requires (std::is_array_v<Type>)
    auto make_shared(const AllocatorType& alloc, Args&&... args)
    {

    }

}