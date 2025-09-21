#pragma once

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <new>

/*
MyVector - is my custom data structure. I create it for tests.
*/


template <typename T>
class MyVector
{
public:

    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;

    MyVector()
    {
        set_default();
    }

    MyVector(const MyVector& other)
    {
        if (this == &other) return;

        clone(other);
    }

    MyVector(MyVector&& other)
    {
        if (this == &other) return;

        swap(other);
    }

    MyVector& operator=(const MyVector& other)
    {
        if (this == &other) return *this;

        clone(other);

        return *this;
    }

    MyVector& operator=(MyVector&& other)
    {
        if (this == &other) return *this;

        swap(other);

        return *this;
    }

    ~MyVector()
    {
        release();
    }

    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data + m_size; }
    const_iterator cbegin() const { return m_data; }
    const_iterator cend() const { return m_data + m_size; }

    size_type size() const { return m_size; }
    size_type capacity() const { return m_capacity; }

    void push_back(const T& value)
    {
        if (m_capacity == 0) reserve(1);
        else if (m_size == m_capacity) reserve(m_capacity * 2);

        new (m_data + m_size) T(value); // Прочесть что это такое
        ++m_size;
    }

    void push_back(T&& value)
    {
        if (m_capacity == 0) reserve(1);
        else if (m_size == m_capacity) reserve(m_capacity * 2);

        new (m_data + m_size) T(std::move(value));
        ++m_size;
    }

    T& operator[](size_type index) { return *(m_data + index); }
    const T& operator[](size_type index) const { return *(m_data + index); }

    friend bool operator==(const MyVector& vector1, const MyVector& vector2) noexcept
    {
        if (vector1.m_size != vector2.m_size) return false;
        return std::equal(vector1.begin(), vector1.end(), vector2.begin());
    }

    friend bool operator!=(const MyVector& vector1, const MyVector& vector2) noexcept
    {
        return !(vector1 == vector2);
    }

    void reserve(const size_type& new_capacity)
    {
        if (new_capacity > m_capacity)
        {
            T* new_data = static_cast<T*>(operator new(sizeof(T) * new_capacity));
            for (size_t i = 0; i < m_size; i++)
                new (new_data + i) T(m_data[i]);
            if (m_size > 0)
                std::destroy(m_data, m_data + m_size);
            if (m_data != nullptr)
                operator delete(m_data);
            m_data = new_data;
            m_capacity = new_capacity;
        }
    }

    void clear()
    {
        if (m_size > 0)
            std::destroy(m_data, m_data + m_size);
        m_size = 0;
    }

    void release()
    {
        clear();
        if (m_data != nullptr)
            operator delete(m_data);
        set_default();
    }

    void swap(MyVector& other)
    {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    void clone(const MyVector& other)
    {
        release();

        if (other.m_size == 0) return;

        reserve(other.m_size);
        m_size = other.m_size;
        for (size_type i = 0; i < other.m_size; i++)
            new(m_data + i) T(other.m_data[i]);
    }

    void set_default()
    {
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

private:
    T* m_data = nullptr;
    size_type m_size{};
    size_type m_capacity{};
};