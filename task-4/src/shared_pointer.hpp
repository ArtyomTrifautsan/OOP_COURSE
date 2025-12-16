#pragma once

#include <utility>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <new>


namespace Pointers {

    // namespace _Impl {

    //     template <typename Type, typename TDeleter = std::default_delete<Type>>
    //     class ControlBlock
    //     {
    //     public:

    //         ControlBlock(Type* _obg, TDeleter _deleter = {}) : m_deleter{_deleter}, m_ref_counter{1}
    //         {
    //             m_obj_ptr = _obg;
    //         }

    //         ~ControlBlock()
    //         {
    //             if (m_obj_ptr != nullptr)
    //                 m_deleter(m_obj_ptr);
    //         }

    //         void add_ref() noexcept { ++m_ref_counter; }

    //         void remove_ref() noexcept
    //         {
    //             --m_ref_counter;

    //             if (m_ref_counter == 0)
    //             {
    //                 delete this;
    //             }
    //         }

    //         Type* get_obj_ptr() noexcept { return m_obj_ptr; }

    //         size_t count_refs() const noexcept { return m_ref_counter; }

    //     private:
    //         Type* m_obj_ptr = nullptr;
    //         TDeleter m_deleter;
    //         size_t m_ref_counter = 0;       // Или size_t?
    //     };

    // };


    // namespace detail
    // {

    //     static size_t align_up(size_t size, size_t alignment) {
    //         return (size + alignment - 1) & ~(alignment - 1);
    //     }


    //     template <typename Type>
    //     struct TypeDestructor
    //     {
    //         void operator(Type* ptr) { ptr->Type(); }
    //     };


    //     struct IControlBlock
    //     {
    //     public:
    //         virtual ~IControlBlock() {}

    //         void add_ref() noexcept { ++m_ref_counter; }

    //         void remove_ref() noexcept
    //         {
    //             --m_ref_counter;

    //             if (m_ref_counter == 0)
    //             {
    //                 dispose();
    //                 destroy();
    //             }
    //         }

    //         size_t refs() const noexcept { return m_ref_counter; }

    //         virtual void dispose() = 0;

    //         virtual void destroy() = 0;

    //     protected:
    //         size_t m_ref_counter = 0;
    //     };


    //     template <typename Type, typename TDeleter = std::default_delete<Type>>
    //     struct SplitCB : public IControlBlock
    //     {
    //     public:
    //         SplitCB(Type* _obj, TDeleter del) : m_obj{_obj}, m_deleter{del}, m_ref_counter{1} {}

    //         virtual ~SplitCB() {}

    //         virtual void dispose() override
    //         {
    //             if (m_obj != nullptr)
    //                 m_deleter(m_obj);
    //         }

    //         virtual void destroy() override
    //         {
    //             delete this;
    //         }

    //     private:
    //         Type* m_obj = nullptr;
    //         TDeleter m_deleter;
    //     };


    //     template <typename Type, typename TAllocator = std::allocator<Type>>
    //     struct MonolitCB : public IControlBlock
    //     {
    //     public:
    //         MonolitCB(Type* _obj, TAllocator alloc, void* raw_memory, size_t raw_memory_size)
    //         : m_obj{_obj},
    //           m_allocator{alloc},
    //           m_raw_memory{raw_memory},
    //           m_raw_memory_size{raw_memory_size},
    //           m_ref_counter{1}
    //         {}

    //         virtual ~MonolitCB() {}

    //         virtual void dispose() override
    //         {
    //             if (m_obj != nullptr)
    //                 m_obj->~Type();
    //         }

    //         virtual void destroy() override
    //         {
    //             using ByteAllocator = typename std::allocator_traits<TAllocator>::template rebind_alloc<char>;
    //             ByteAllocator byte_allocator = m_allocator;

    //             this->~MonolitCB();

    //             if (m_raw_memory)
    //                 byte_allocator.deallocate(m_raw_memory, m_raw_memory_size);
    //         }

    //     private:
    //         Type* m_obj = nullptr;
    //         TAllocator m_allocator;
    //         void* m_raw_memory = nullptr;
    //         size_t m_raw_memory_size;
    //     };

    // }

    template <typename Type, typename AllocatorType, typename... Args>
    requires (!std::is_array_v<Type>)
    auto allocate_shared(const AllocatorType& alloc, Args&&... args);

    namespace detail
    {

        // template <typename Type, typename TDeleter = std::default_delete<Type>, typename TAllocator = std::allocator<Type>>
        template <typename Type, typename TDeleter, typename TAllocator>
        class ControlBlock
        {
        public:

            ControlBlock(Type* _obj, TDeleter del, TAllocator alloc, size_t raw_mem_size = 0)
                : m_deleter{del}, m_allocator{alloc}, m_ref_counter{1}
            {
                m_raw_memory_size = raw_mem_size == 0 ? sizeof(*this) : raw_mem_size;
            }

            ~ControlBlock() = default;

            void add_ref() noexcept { ++m_ref_counter; }

            void remove_ref() noexcept
            {
                --m_ref_counter;

                // if (m_ref_counter == 0)
                // {
                //     // Что делать?
                // }
            }

            // Type* obj() noexcept { return m_obj; }

            TDeleter deleter() const { return m_deleter; }

            TAllocator allocator() const { return m_allocator; }

            size_t refs() const noexcept { return m_ref_counter; }

            size_t raw_memory_size() const noexcept { return m_raw_memory_size; }

        private:
            // Type* m_obj = nullptr;
            TDeleter m_deleter;
            TAllocator m_allocator;
            size_t m_ref_counter = 0;
            size_t m_raw_memory_size = 0;
        };


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

    template<typename Type, typename TDeleter = std::default_delete<Type>, typename TAllocator = std::allocator<Type>>
    class SharedPTR {
    public:
        // Я решил сделать функцию allocate_shared friend, чтобы она могла вызвать приватный
        // конструктор SharedPTR(Type *pObj, detail::ControlBlock<Type, TDeleter, TAllocator> cb).
        // Я сделал его приватным, потому что не хочу, чтобы его видели извне
        // template <typename Type, typename AllocatorType = std::allocator<Type>, typename... Args>
        // requires (!std::is_array_v<Type>)
        // friend auto allocate_shared(const AllocatorType& alloc, Args&&... args);
        // template <typename... Args>
        // requires (!std::is_array_v<Type>)
        // friend auto allocate_shared<Type, AllocatorType, Args...>(const AllocatorType& alloc, Args&&... args);

        // template <typename TAlloc_, typename... Args_>
        // friend auto allocate_shared(const TAlloc_& alloc, Args_&&... args);

        using t_SharedPTR = SharedPTR<Type, TDeleter, TAllocator>;

        using PureType = std::conditional_t<std::is_array_v<Type>, std::remove_extent_t<Type>, Type>;

        using TControlBlock = detail::ControlBlock<PureType, TDeleter, TAllocator>;
    
    // private:
    public:
        // template <typename TAllocator = std::allocator<Type>>
        SharedPTR(Type *pObj, detail::ControlBlock<Type, TDeleter, TAllocator>* cb)
        {
            /*
            По идее, тут не может быть nullptr
            */
            // if (pObj == nullptr)
            //     return;

            m_obj = pObj;

            m_control_block = cb;
        }

    public: // Constructors and destructor.
        SharedPTR() noexcept = default;

        SharedPTR(PureType* pObj, TDeleter deleter_ = {})
        {
            if (pObj == nullptr)
                return;

            m_obj = pObj;

            m_control_block = new detail::ControlBlock(pObj, deleter_, std::allocator<Type>{});
        }

        SharedPTR(t_SharedPTR&& other) noexcept
        {
            std::swap(m_obj, other.m_obj);
            std::swap(m_control_block, other.m_control_block);
        }

        SharedPTR(const t_SharedPTR& other)
        {
            if (other.m_control_block == nullptr) return;

            m_control_block = other.m_control_block;

            m_obj = other.m_obj;

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

            std::swap(m_obj, other.m_obj);
            std::swap(m_control_block, other.m_control_block);

            return *this;
        }

        t_SharedPTR& operator=(Type *pObj)
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
                m_obj = other.m_obj;
                m_control_block = other.m_control_block;
                m_control_block->add_ref();
            }

            return *this;
        }

    public: // Observers.
    // Dereference the stored pointer.

        Type& operator*() const requires (!std::is_array_v<Type>)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            if (m_obj == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return *m_obj;
        }

        // Return the stored pointer.
        Type* operator->() const requires (!std::is_array_v<Type>)
        {
            if (m_control_block == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            if (m_obj == nullptr)
                throw std::runtime_error("Dereferencing nullptr: SharedPTR is nullptr.");

            return m_obj;
        }

        // Type& operator[](size_t index) const requires (std::is_array_v<Type>)
        // {
        //     check_valid();
        //     return m_obj[index];
        // }

        // Return the stored pointer.
        Type* get() const
        {
            return m_obj;
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
                release_object();
                delete_control_block();
            }

            m_obj = nullptr;
            m_control_block = nullptr;
        }

        // Replace the stored pointer.
        void reset(Type *pObj = nullptr)
        {
            // кинуть если передали тот же самый указатель который уже хранится
            if (m_control_block != nullptr && pObj == m_obj)
                throw std::runtime_error("SharedPTR already have this pointer.");

            TDeleter del = m_control_block->deleter();
            TAllocator alloc = m_control_block->allocator();

            release();

            if (pObj != nullptr)
            {
                m_obj = pObj;
                m_control_block = new detail::ControlBlock(pObj, del, alloc);
            }
        }

        // Exchange the pointer with another object.
        void swap(t_SharedPTR& other)
        {
            std::swap(m_obj, other.m_obj);
            std::swap(m_control_block, other.m_control_block);
        }

    private:
        Type* m_obj = nullptr;
        detail::ControlBlock<Type, TDeleter, TAllocator>* m_control_block = nullptr;

        void release_object()
        {
            auto deleter = m_control_block->deleter();
            deleter(m_obj);
            m_obj = nullptr;
        }

        void delete_control_block()
        {
            auto allocator = m_control_block->allocator();
            size_t cb_size = m_control_block->raw_memory_size();

            using ByteAllocator = typename std::allocator_traits<TAllocator>::template rebind_alloc<char>;
            ByteAllocator byte_allocator = allocator;

            m_control_block->~ControlBlock();

            byte_allocator.deallocate(reinterpret_cast<char*>(m_control_block), cb_size);

            m_control_block = nullptr;
        }
    };


    template <typename Type, typename AllocatorType = std::allocator<Type>, typename... Args>
    requires (!std::is_array_v<Type>)
    auto allocate_shared(const AllocatorType& alloc, Args&&... args)
    {
        // Обязательно учитываем выравнивание

        // 1. Расчет смещения для Type
        // Нам нужно, чтобы смещение было кратно alignof(Type).
        size_t offset_for_object = detail::align_up(sizeof(detail::ControlBlock<Type, detail::ObjectDestructor<Type>, AllocatorType>), alignof(Type));

        // 2. Расчет общего размера
        size_t total_size = offset_for_object + sizeof(Type);

        // using CommonAllocator = AllocatorType<Type>::rebind<char>::other;
        using ByteAllocator = typename std::allocator_traits<AllocatorType>::template rebind_alloc<char>;
        ByteAllocator allocator = alloc;

        void* raw_memory = nullptr;
        detail::ControlBlock<Type, detail::ObjectDestructor<Type>, AllocatorType>* control_block = nullptr;
        Type* obj = nullptr;

        try
        {
            raw_memory = allocator.allocate(total_size);

            char* char_memory = static_cast<char*>(raw_memory);

            obj = new (char_memory + offset_for_object) Type{std::forward<Args>(args)...};

            control_block = new (static_cast<detail::ControlBlock<Type, detail::ObjectDestructor<Type>, AllocatorType>*>(raw_memory)) detail::ControlBlock(obj, detail::ObjectDestructor<Type>{}, alloc, total_size);
        }
        catch (...)
        {
            if (obj != nullptr)
                obj->~Type();
            
            if (control_block != nullptr)
                control_block->~ControlBlock();

            if (raw_memory != nullptr)
                allocator.deallocate(static_cast<char*>(raw_memory), total_size);

            throw;
        }

        return SharedPTR<Type, detail::ObjectDestructor<Type>, AllocatorType>{obj, control_block};
    }


    template <typename Type, typename AllocatorType = std::allocator<Type>, typename... Args>
    requires (std::is_array_v<Type>)
    auto allocate_shared(const AllocatorType& alloc, Args&&... args)
    {

    }


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