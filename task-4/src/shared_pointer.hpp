#pragma once

#include <utility>
#include <iostream>
#include <type_traits>
#include <stdexcept>


namespace Pointers {

    namespace _Impl {

        template <typename Type, typename TDeleter = std::default_delete<Type>>
        class ControlBlock
        {
        public:

            ControlBlock(Type* _obg, TDeleter _deleter = {}) : m_deleter{_deleter}, m_ref_counter{1}
            {
                m_obj_ptr = _obg;
            }

            ~ControlBlock()
            {
                if (m_obj_ptr != nullptr)
                    m_deleter(m_obj_ptr);
            }

            void add_ref() noexcept { ++m_ref_counter; }

            void remove_ref() noexcept
            {
                --m_ref_counter;

                if (m_ref_counter == 0)
                {
                    delete this;
                }
            }

            Type* get_obj_ptr() noexcept { return m_obj_ptr; }

            size_t count_refs() const noexcept { return m_ref_counter; }

        private:
            Type* m_obj_ptr = nullptr;
            TDeleter m_deleter;
            size_t m_ref_counter = 0;       // Или size_t?
        };

    };


    // namespace detail
    // {

    //     class IControlBlock
    //     {
    //     public:
    //         virtual ~IControlBlock() = 0;
    //         virtual void add_ref() = 0;
    //         virtual void remove_ref() = 0;

    //     private:
    //         size_t m_ref_counter = 0;
    //     };

    // }

    template<typename Type, typename TDeleter = std::default_delete<Type>>
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
            m_control_block = other.m_control_block;
            other.m_control_block = nullptr;
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

            m_control_block->remove_ref();
        }

    public: // Assignment.
        t_SharedPTR& operator=(t_SharedPTR&& other) noexcept
        {
            if (this == &other) return *this;

            // Вы сказали добавить эту строчку, но мне кажется это ломает логику работы
            // if (m_control_block == other.m_control_block) return *this;

            release();

            m_control_block = other.m_control_block;

            other.m_control_block = nullptr;

            return *this;
        }

        t_SharedPTR& operator=(Type *pObj)
        {
            if (m_control_block != nullptr && pObj == m_control_block->get_obj_ptr())
                throw std::runtime_error("SharedPTR already have this pointer.");

            release();  

            if (pObj != nullptr) 
                m_control_block = new _Impl::ControlBlock(pObj, TDeleter{});

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

        Type& operator*() const
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return *(m_control_block->get_obj_ptr());
        }

        // Return the stored pointer.
        Type* operator->() const
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return m_control_block->get_obj_ptr();
        }

        // Return the stored pointer.
        Type* get() const
        {
            return m_control_block != nullptr ? m_control_block->get_obj_ptr() : nullptr;
        }

        // Return false if the stored pointer is null.
        operator bool() const { return m_control_block != nullptr; }

        friend bool operator==(const t_SharedPTR& first, const t_SharedPTR& second)
        {
            return first.m_control_block == second.m_control_block;
        }

        int count_refs() const noexcept
        {
            if (m_control_block == nullptr) return 0;

            return m_control_block->count_refs();
        }

    public: // Modifiers.
        // Release ownership of any stored pointer.
        void release()
        {
            if (m_control_block == nullptr) return;

            m_control_block->remove_ref();
            m_control_block = nullptr;
        }

        // Replace the stored pointer.
        void reset(Type *pObj = nullptr)
        {
            // кинуть если передали тот же самый указатель который уже хранится
            if (m_control_block != nullptr && pObj == m_control_block->get_obj_ptr())
                throw std::runtime_error("SharedPTR already have this pointer.");

            release();

            if (pObj == nullptr) return;

            m_control_block = new _Impl::ControlBlock(pObj, TDeleter{});
        }

        // Exchange the pointer with another object.
        void swap(t_SharedPTR& other)
        {
            std::swap(m_control_block, other.m_control_block);
        }


    private:
        _Impl::ControlBlock<Type, TDeleter>* m_control_block = nullptr;
    };


    template<typename Type, typename... Args>
    auto make_shared(Args&&... args)
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
        Type* pObj = new Type{std::forward<Args>(args)...};
        return SharedPTR<Type>(pObj);
    }



    // template<typename Type, typename... Args>
    // requires (std::is_array_v<Type> == false)
    // auto make_shared_pointer(Args&&... args) {
    //     using PureType = std::remove_extent_t<Type>;
    //     using DtorPureType = detail::Dtor<PureType>;

    //     using Cblock = typename Shared_pointer<Type, DtorPureType>::ControlBlock;

    //     void* memory = ::operator new(sizeof(PureType) + sizeof(Cblock));


    //     PureType* p_obj = nullptr;
    //     Cblock* p_block = nullptr;

    //     try {
    //         p_block = new(memory) Cblock(DtorPureType());
    //         p_obj = new(static_cast<char*>(memory) + sizeof(Cblock)) Type(std::forward<Args>(args)...);

    //         return Shared_pointer<Type, DtorPureType>(p_obj, p_block);
    //     }
    //     catch (...) {
    //         if (p_obj) p_obj->~PureType();
    //         if (p_block) p_block->~ControlBlock();
    //         ::operator delete(memory);
    //         throw;
    //     }
    // }

    // template<typename Type>
    //     requires std::is_array_v<Type>
    // auto make_shared_pointer(std::size_t N) {
    //     using PureType = std::remove_extent_t<Type>;
    //     using DtorPureType = detail::Dtor<PureType[]>;
    //     using Cblock = typename Shared_pointer<Type, DtorPureType>::ControlBlock;

    //     if (N == 0) {
    //         return Shared_pointer<Type, DtorPureType>(nullptr, nullptr);
    //     }

    //     void* memory = ::operator new(sizeof(PureType) * N + sizeof(Cblock));
    //     PureType* p_obj = nullptr;
    //     Cblock* p_block = nullptr;

    //     try {
    //         p_block = new(memory) Cblock(DtorPureType(N));
    //         p_obj = reinterpret_cast<PureType*>(static_cast<char*>(memory) + sizeof(Cblock));

    //         for (std::size_t i = 0; i < N; ++i) {
    //             new(&p_obj[i]) PureType();
    //         }

    //         return Shared_pointer<Type, DtorPureType>(p_obj, p_block);
    //     }
    //     catch (...) {
    //         if (p_obj) {
    //             for (std::size_t i = 0; i < N; ++i) {
    //                 p_obj[i].~PureType();
    //             }
    //         }
    //         if (p_block) p_block->~Cblock();
    //         ::operator delete(memory);
    //         throw;
    //     }
    // }

}