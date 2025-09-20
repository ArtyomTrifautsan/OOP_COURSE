#pragma once

#include <algorithm>


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
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    MyVector(const MyVector& other)
    {
        if (other.m_size == 0) return;

        reserve(other.m_capacity);
        m_size = other.m_size;
        std::copy(other.m_data, other.m_data + other.m_size, m_data);
    }

    MyVector(MyVector&& other)
    {

        m_data = other.m_data;
        other.m_data = nullptr;

        m_capacity = other.m_capacity;
        other.m_capacity = 0;

        m_size = other.m_size;
        other.m_size = 0;
    }

    MyVector& operator=(const MyVector& other)
    {
        if (this == &other) return *this;

        if (m_data) delete[] m_data;

        reserve(other.m_capacity);
        m_size = other.m_size;
        std::copy(other.m_data, other.m_data + other.m_size, m_data);

        return *this;
    }

    MyVector& operator=(MyVector&& other)
    {
        if (this == &other) return *this;

        m_data = other.m_data;
        other.m_data = nullptr;

        m_capacity = other.m_capacity;
        other.m_capacity = 0;

        m_size = other.m_size;
        other.m_size = 0;

        return *this;
    }

    ~MyVector()
    {
        if (m_data)
        {
            // std::destroy(m_data, m_data + m_size);
            delete[] m_data;
        }
    }

    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data + m_size; }
    const_iterator cbegin() const { return m_data; }
    const_iterator cend() const { return m_data + m_size; }

    size_type size() const { return m_size; }
    size_type capacity() const { return m_capacity; }

    void clear()
    {
        std::destroy(m_data, m_data + m_size);
        m_size = 0;
    }

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

    void reserve(size_type new_capacity)
    {
        if (new_capacity > m_capacity)
        {
            T* old_data = m_data;
            m_data = new T[new_capacity];
            std::copy(old_data, old_data + m_size, m_data);     // Прочесть как это работает
            m_capacity = new_capacity;
            delete[] old_data;
        }
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

    void release()
    {
        delete[] m_data;
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    // void swap(MyVector& other)
    // {
    //     std::swap()
    // }

private:
    T* m_data = nullptr;
    size_type m_size{};
    size_type m_capacity{};
};