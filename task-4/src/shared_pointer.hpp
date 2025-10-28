#pragma once


namespace SharedPointer {

    template<class Type, class TDeleter>
    class Pointer {
        typedef SharedPTR<Type, TDeleter> t_SharedPTR;
    public: // Constructors and destructor.
        SharedPTR();
        SharedPTR(Type *pObj);
        SharedPTR(t_SharedPTR &&uniquePTR); // Move constructor.
        SharedPTR(const t_SharedPTR&);
        ~SharedPTR();
    public: // Assignment.
        t_SharedPTR& operator=(t_SharedPTR &&sharedPTR);
        t_SharedPTR& operator=(Type *pObject);
        t_SharedPTR& operator=(const t_SharedPTR&);
    public: // Observers.
        Type& operator*() const; // Dereference the stored pointer.
        Type* operator->() const; // Return the stored pointer.
        Type* get() const; // Return the stored pointer.
        TDeleter& get_deleter(); // Return a reference to the stored deleter.
        operator bool() const; // Return false if the stored pointer is null.
    public: // Modifiers.
        void release(); // Release ownership of any stored pointer.
        void reset(Type *pObject = nullptr); // Replace the stored pointer.
        void swap(t_SharedPTR &sharedPTR); // Exchange the pointer with another object.
    };

}