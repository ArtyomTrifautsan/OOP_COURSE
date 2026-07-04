#pragma once

#include <utility>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <new>


namespace Pointers {

    template<typename Type, typename TDeleter = std::default_delete<
            std::conditional_t<
                std::is_array_v<Type>,
                std::remove_extent_t<Type>[],
                Type
            >
        >
    >
    class SharedPTR {
    public:
        using t_SharedPTR = SharedPTR<Type, TDeleter>;
        using PureType = std::conditional_t<std::is_array_v<Type>, std::remove_extent_t<Type>, Type>;
        bool static constexpr IsTypeArray = std::is_array_v<Type>;

    public:
        class ControlBlock
        {
        public:

            ControlBlock(PureType* _obj, TDeleter del, bool _monolithic, size_t _array_size)
                : m_obj{_obj}, m_deleter{del}, m_monolithic{_monolithic}, m_array_size{_array_size}, m_ref_counter{1} {}
            ~ControlBlock() = default;

            void add_ref() noexcept { ++m_ref_counter; }
            void remove_ref() noexcept { --m_ref_counter; }

            PureType* obj() noexcept { return m_obj; }
            TDeleter deleter() noexcept { return m_deleter; }
            size_t refs() const noexcept { return m_ref_counter; }  
            bool monolithic() const noexcept { return m_monolithic; }
            size_t array_size() const noexcept { return m_array_size; }

            void* get_deleter_ptr() {
                if (std::is_array_v<Type>) {
                    return static_cast<void*>(m_obj);
                }
                return m_obj;
            }

        private:
            PureType* m_obj = nullptr;
            TDeleter m_deleter;
            size_t m_ref_counter = 0;
            bool m_monolithic = false;
            size_t m_array_size = 0;
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

            m_control_block = new ControlBlock{pObj, deleter_, false, 0};
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

        PureType& operator*() const requires (!IsTypeArray)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return *(m_control_block->obj());
        }

        // Return the stored pointer.
        PureType* operator->() const requires (!IsTypeArray)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return m_control_block->obj();
        }

        PureType& operator[](size_t index) requires (IsTypeArray)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");
            
            if (index >= m_control_block->array_size())
                throw std::runtime_error("Access denied: index is out of range.");

            return m_control_block->obj()[index];
        }

        // Return the stored pointer.
        PureType* get() const
        {
            if (m_control_block == nullptr) return nullptr;

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
        void reset(PureType *pObj = nullptr, size_t s = 0)
        {
            // кинуть если передали тот же самый указатель который уже хранится
            if (m_control_block != nullptr && pObj == m_control_block->obj())
                throw std::runtime_error("SharedPTR already have this pointer.");

            TDeleter del = m_control_block->deleter();

            release();

            if (pObj != nullptr)
            {
                m_control_block = new ControlBlock(pObj, del, false, s);
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
            PureType* obj = m_control_block->obj();
            size_t s = m_control_block->array_size();

            if (s == 0)
            {
                if (obj != nullptr)
                    obj->~PureType();
            }
            else
            {
                // for (size_t created = s; created >= 0; created--)
                // {
                //     obj[sizeof(CBT) + sizeof(Type) * created].~PureType();
                // }
                for (size_t i = s; i > 0; --i)
                    obj[i - 1].~PureType();
            }

            m_control_block->~ControlBlock();

            // Так как я выделял память для char, я решил привести указатель к типу char*,
            // прежде чем освободить память. Просто на всякий случай. 
            char* char_ptr = reinterpret_cast<char*>(m_control_block);
            operator delete(char_ptr);
        }

        void split_delete()
        {
            PureType* obj = m_control_block->obj();
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

        template <typename T>
        requires (std::is_array_v<T>)
        friend auto make_shared(size_t size);
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
            control_block = new (char_memory) CBT{obj, {}, true, 0};
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


    template <typename Type>
    requires (std::is_array_v<Type>)
    auto make_shared(size_t size)
    {
        using CBT = SharedPTR<Type>::ControlBlock;
        using PureType = std::remove_extent_t<Type>;

        size_t total_size = sizeof(CBT) + sizeof(PureType) * size;
        void* raw_memory = nullptr;
        PureType* obj = nullptr;
        CBT* control_block = nullptr;
        size_t created = 0;

        try
        {
            raw_memory = operator new(total_size);
            char* char_memory = static_cast<char*>(raw_memory);
            if (size > 0)
            {
                obj = reinterpret_cast<PureType*>(char_memory + sizeof(CBT));
                while(created < size)
                {
                    // new (char_memory + sizeof(CBT) + sizeof(PureType) * created) PureType{};
                    new (obj + created) PureType{};
                    ++created;
                }
            }
            control_block = new (char_memory) CBT{obj, {}, true, size};
        }
        catch (...)
        {
            // if (obj != nullptr)
            //     obj->~Type();
            if (obj != nullptr)
            {
                if (created > 0)
                {
                    // for (; created >= 0; created--)
                    // {
                    //     obj[sizeof(CBT) + sizeof(Type) * created].~PureType();
                    // }
                    for (size_t i = created; i > 0; --i)
                        obj[i - 1].~PureType();
                }
            }

            if (control_block != nullptr)
                control_block->~ControlBlock();

            if (raw_memory != nullptr)
                operator delete(raw_memory);

            throw;
        }

        return SharedPTR<Type>{control_block, true};
    }

}