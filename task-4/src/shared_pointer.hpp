#pragma once


namespace SharedPointer {

    template<class Type, class TDeleter>
    class Pointer {
        using t_Pointer = Pointer<Type, TDeleter>;

    public: // Constructors and destructor.
        Pointer() noexcept = default;

        Pointer(Type *pObj, TDeleter deleter_ = {}) noexcept
        {
            // А что будет если передать сюда стековый указатель? У нас же не сработает delete expression
            if (pObj == nullptr)
            {
                return;
            }
            
            {
                m_obj_ptr = pObj;
                *m_ref_counter = 1;
            }

            m_deleter = deleter_;
        }

        Pointer(Pointer&& uniquePTR) noexcept // Move constructor.
        {
    

            m_obj_ptr = uniquePTR.m_obj_ptr;
            m_ref_counter = uniquePTR.m_ref_counter;
            m_deleter = uniquePTR.m_deleter;

            ++(*m_ref_counter);

            uniquePTR.m_ref_counter = nullptr;
        }

        Pointer(const t_Pointer& other)
        {
            // Мы же не можем вызвать конструктор у уже существующего объекта?


            m_obj_ptr = other.m_obj_ptr;
            m_ref_counter = other.m_ref_counter;
            m_deleter = other.m_deleter;

            ++(*m_ref_counter);
        }

        ~Pointer()
        {
            if (*m_ref_counter == 0){}

            else if (*m_ref_counter == 1)
            {
                if (m_obj_ptr != nullptr)
                {
                    m_deleter(m_obj_ptr);
                    m_obj_ptr = nullptr;
                }
                *m_ref_counter = 0;
            }

            else
            {
                --(*m_ref_counter);
            }
        }

    public: // Assignment.
        t_Pointer& operator=(t_Pointer &&Pointer) noexcept
        {
            if (other == *this) return *this;

            // m_obj_ptr = uniquePTR.m_obj_ptr;
            std::swap(m_obj_ptr, uniquePTR.m_obj_ptr);
            // m_ref_counter = uniquePTR.m_ref_counter;
            std::swap(m_ref_counter, uniquePTR.m_ref_counter);  // делаю чтобы гарантировать noexcept
            // m_deleter = uniquePTR.m_deleter;
            std::swap(m_deleter, uniquePTR.m_deleter);

            ++(*m_ref_counter);

            return *this
        }

        t_Pointer& operator=(Type *pObject)
        {
            release();

            if (pObj == nullptr)
            {
                m_obj_ptr = nullptr;
                *m_ref_counter = 0;
            }
            else 
            {
                m_obj_ptr = pObj;
                *m_ref_counter = 1;
            }

            m_deleter{};
        }

        t_Pointer& operator=(const t_Pointer&)
        {
            if (other == *this) return *this;

            release();

            m_obj_ptr = other.m_obj_ptr;
            m_ref_counter = other.m_ref_counter;
            m_deleter = other.m_deleter;

            ++(*m_ref_counter);

            return *this;
        }

    public: // Observers.
    // Dereference the stored pointer.
        Type& operator*() const { return *m_obj_ptr; }

        // Return the stored pointer.
        Type* operator->() const { return m_obj_ptr; }

        // Return the stored pointer.
        Type* get() const { return m_obj_ptr; }

        // Return a reference to the stored deleter.
        TDeleter& get_deleter() { return m_deleter; }

        // Return false if the stored pointer is null.
        operator bool() const { return m_obj_ptr != nullptr; }

    public: // Modifiers.
        // Release ownership of any stored pointer.
        void release() 
        {
            if (m_obj_ptr == nullptr) return;

            if (*m_ref_counter > 1)
            {
                m_obj_ptr = nullptr;
                --(*m_ref_counter);
            }
            else
            {
                m_deleter(m_obj_ptr);
                m_obj_ptr = nullptr;
                *m_ref_counter = 0;
            }
        }

        // Replace the stored pointer.
        void reset(Type *pObject = nullptr)
        {
            /*
            m_obj_ptr и pObject -- nullptr
            ничего не происходит, по сути просто сбрасываем и так пустой указатель

            m_obj_ptr -- nullptr, pObject - нет
            по сути создаем первый указатель на pObject, т.е. по аналогии с конструктором

            m_obj_ptr -- нет, pObject - nullptr
            сбрасываем существующий укзатель, делая его пустым

            m_obj_ptr и pObject - не nullptr
            по сути как оператор присваивания
            */

            // if (m_obj_ptr == nullptr)
            // {
            //     m_obj_ptr = pObj;
            //     *m_ref_counter = 1;
            //     m_deleter{};
            // }
            // else
            // {
            //     if (*m_ref_counter > 1)
            //     {
            //         --(*m_ref_counter);
            //         m_obj_ptr = pObject;
            //     }
            //     else
            //     {
            //         m_deleter(m_obj_ptr);
            //         m_obj_ptr = nullptr;
            //         *m_ref_counter = 0;
            //     }
            // }
        }

        // Exchange the pointer with another object.
        void swap(t_Pointer &Pointer); 

    private:
        Type* m_obj_ptr = nullptr;
        int* m_ref_counter = 0;
        TDeleter m_deleter{};
    };

}