#pragma once

#include <utility>
#include <iostream>
#include <type_traits>


namespace Pointers {

    namespace _Impl {

        template<typename Type>
        class BaseDeleter
        {
        public:
            void operator()(Type* ptr) const noexcept { delete ptr; }
        };


        template <typename Type, typename TDeleter = BaseDeleter<Type>>
        class ControlBlock
        {
        public:

            ControlBlock(Type* _obg, TDeleter _deleter = {}) : m_deleter{_deleter}, m_shared_ref_counter{1}
            {
                m_obj_ptr = _obg;
            }

            template <typename... Args>
            ControlBlock(Args&&... args) : m_shared_ref_counter{1}
            {
                m_obj_ptr = new Type(std::forward<Args>(args)...);
            }

            ~ControlBlock()
            {
            }

            void add_shared_ref() noexcept { ++m_shared_ref_counter; }

            void remove_shared_ref() noexcept 
            {
                --m_shared_ref_counter; 

                if (m_shared_ref_counter == 0)
                    m_deleter(m_obj_ptr);

                if (m_shared_ref_counter == 0 && m_weak_ref_counter == 0)
                    delete this;
            }

            void add_weak_ref() noexcept { ++m_weak_ref_counter; }

            void remove_weak_ref() noexcept 
            {
                --m_weak_ref_counter;

                if (m_shared_ref_counter == 0 && m_weak_ref_counter == 0)
                    delete this;
            }

            Type* get_obj_ptr() noexcept { return m_obj_ptr; }

            int count_shared_refs() const noexcept { return m_shared_ref_counter; }

        private:
            Type* m_obj_ptr = nullptr;
            TDeleter m_deleter;
            int m_shared_ref_counter = 0;       // Или size_t?
            int m_weak_ref_counter = 0;         // Или size_t?
        };


        

    };

    template<typename Type, typename TDeleter = _Impl::BaseDeleter<Type>>
    class SharedPTR {
        using t_SharedPTR = SharedPTR<Type, TDeleter>;

    public: // Constructors and destructor.
        SharedPTR() noexcept = default;

        SharedPTR(Type *pObj, TDeleter deleter_ = {}) noexcept
        {
            if (pObj == nullptr)
                return;

            m_control_block = new _Impl::ControlBlock(pObj, deleter_);
        }

        SharedPTR(t_SharedPTR&& other) noexcept
        {
            if (other.m_control_block == nullptr) return;

            std::swap(m_control_block, other.m_control_block);
        }

        SharedPTR(const t_SharedPTR& other)
        {
            if (other.m_control_block == nullptr) return;

            m_control_block = other.m_control_block;
            m_control_block->add_shared_ref();
        }

        ~SharedPTR()
        {
            if (m_control_block == nullptr) return;

            m_control_block->remove_shared_ref();
        }

    public: // Assignment.
        t_SharedPTR& operator=(t_SharedPTR&& other) noexcept
        {
            if (other == *this) return *this;

            // release()    // Сомневаюсь что тут надо делать release()

            std::swap(m_control_block, other.m_control_block);

            return *this;
        }

        t_SharedPTR& operator=(Type *pObj)
        {
            if (m_control_block != nullptr && pObj == m_control_block->get_obj_ptr())
                return *this;

            release();

            if (pObj != nullptr) 
                m_control_block = new _Impl::ControlBlock(pObj);

            return *this;
        }

        t_SharedPTR& operator=(const t_SharedPTR& other)
        {
            if (other == *this) return *this;

            release();

            if (other.m_control_block != nullptr)
            {
                m_control_block = other.m_control_block;
                m_control_block->add_shared_ref();
            }

            return *this;
        }

    public: // Observers.
    // Dereference the stored pointer.
        Type& operator*() const { return *(m_control_block->get_obj_ptr()); }

        // Return the stored pointer.
        Type* operator->() const { return m_control_block->get_obj_ptr(); }

        // Return the stored pointer.
        Type* get() const
        {
            return m_control_block != nullptr ? m_control_block->get_obj_ptr() : nullptr;
        }

        // Return false if the stored pointer is null.
        operator bool() const { return m_control_block != nullptr; }

        int count_refs() const noexcept
        {
            if (m_control_block == nullptr) return 0;

            return m_control_block->count_shared_refs();
        }

    public: // Modifiers.
        // Release ownership of any stored pointer.
        void release()
        {
            if (m_control_block == nullptr) return;

            m_control_block->remove_shared_ref();
            m_control_block = nullptr;
        }

        // Replace the stored pointer.
        void reset(Type *pObj = nullptr)
        {
            release();

            if (pObj == nullptr) return;

            m_control_block = new _Impl::ControlBlock(pObj);
        }

        // Exchange the pointer with another object.
        void swap(t_SharedPTR& other)
        {
            if (*this == other) return;

            std::swap(m_control_block, other.m_control_block);
        }

        friend bool operator==(const t_SharedPTR& first, const t_SharedPTR& second)
        {
            return first.m_control_block == second.m_control_block;
        }


    private:
        _Impl::ControlBlock<Type, TDeleter>* m_control_block = nullptr;
    };


    template<typename Type>
    SharedPTR<Type, _Impl::BaseDeleter<Type>> make_shared()
    {
        return SharedPTR<Type, _Impl::BaseDeleter<Type>>();
    }


    template<typename Type, typename... Args>
    requires (!std::is_same_v<Args, SharedPTR<Type, _Impl::BaseDeleter<Type>>&> && ...)
    SharedPTR<Type, _Impl::BaseDeleter<Type>> make_shared(Args&&... args)
    // auto make_shared(Args&&... args) -> std::enable_if_t<(sizeof...(Args) > 0) && !std::is_same_v<std::decay>>
    {
        /*
        На cpp reference написано, что при использовании std::make_shared происходит только одна
        аллокация памяти, но я не придумал как правильно реализовать эту затею.

        У меня только одна идея. Сделать класс ControlBlock интерфейсом, и сделать две реализации 
        интерфейса: MonolitControlBlock и NonMonolitControlBlock. Первый класс будет использоваться
        при создании SharedPTR с помощью функций make_shared, а второй при создании SharedPTR с 
        помощью конструкторов.
        */
        Type* pObj = new Type(std::forward<Args>(args)...);
        return SharedPTR<Type, _Impl::BaseDeleter<Type>>(pObj);
    }


    template<typename Type>
    SharedPTR<Type, _Impl::BaseDeleter<Type>> make_shared(const SharedPTR<Type, _Impl::BaseDeleter<Type>>& other)
    {
        return SharedPTR(other);
    }

}