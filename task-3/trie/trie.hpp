#pragma once

#include <string>
#include <utility>
#include <array>
#include <map>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <type_traits>


namespace Containers
{
    namespace _Impl {

        template <typename T> class Iterator;


        template <typename T> class ConstIterator;


        template <typename T> class Vertex;


        template <typename T> class SubTrie;
    
    }


    template <typename T>
    class Trie
    {
    public:
        using iterator = _Impl::Iterator<T>;
        using const_iterator = _Impl::ConstIterator<T>;

        using value_type = T;
        using key_type = std::string;

        using size_type = size_t;

        using vertex_type = _Impl::Vertex<T>;

        Trie() = default;

        template <typename InputIterator,
                    typename = std::enable_if_t<
                        std::is_base_of_v<
                            std::input_iterator_tag,
                            typename std::iterator_traits<InputIterator>::iterator_category
                        > &&
                        std::is_convertible_v<
                            std::pair<key_type, value_type>,
                            typename std::iterator_traits<InputIterator>::value_type
                        >
                    >
                >
        Trie(InputIterator first, InputIterator last)
        {
            insert(first, last);
        }


        Trie(const Trie<T>& other) = default;


        Trie<T>& operator=(const Trie<T>& other) = default;


        Trie(Trie<T>&& other) = default;


        Trie<T>& operator=(Trie<T>&& other) = default;


        iterator begin()
        {
            std::shared_ptr<vertex_type> v = m_root;

            while (v != nullptr && !(v->end_of_word())) v = v->next();

            return iterator{v};
        }

        const_iterator begin() const
        {
            std::shared_ptr<vertex_type> v = m_root;

            while (v != nullptr && !v->end_of_word()) v = v->next();

            return const_iterator{v};
        }

        iterator end() { return iterator{nullptr}; }
        const_iterator end() const { return const_iterator{nullptr}; }

        bool empty() const noexcept { return m_size == 0; }

        size_type size() const noexcept { return m_size; }

        value_type& operator[](const key_type& key)
        {
            auto vertex = find_vertex(key);

            if (vertex == nullptr)
            {
                insert(key, value_type{});
                vertex = find_vertex(key);
            }

            return vertex->value();
        }

        std::pair<iterator, bool> insert(const key_type& key, const value_type& value)
        {
            // 1. Add prefix
            if (m_root == nullptr)
                m_root = std::make_shared<vertex_type>(static_cast<char>(0), nullptr);

            auto it = key.begin();
            std::shared_ptr<vertex_type> curr_vertex = m_root;
            
            while (it != key.end())
            {
                std::shared_ptr<vertex_type> child_vertex = curr_vertex->child_with_symbol(*it);
                if (child_vertex == nullptr)
                {
                    curr_vertex->add_child(*it, std::make_shared<vertex_type>(*it, curr_vertex));
                    child_vertex = curr_vertex->child_with_symbol(*it);
                }
                ++it;
                curr_vertex = child_vertex;
            }

            // 2. Write value
            bool new_data = curr_vertex->write_data(key, value);

            if (new_data) ++m_size;

            return std::pair<iterator, bool>{iterator{curr_vertex}, new_data};

        }

        template <typename InputIterator,
                    typename = std::enable_if_t<
                        std::is_base_of_v<
                            std::input_iterator_tag,
                            typename std::iterator_traits<InputIterator>::iterator_category
                        > &&
                        std::is_convertible_v<
                            std::pair<key_type, value_type>,
                            typename std::iterator_traits<InputIterator>::value_type
                        >
                    >
                >
        void insert(InputIterator first, InputIterator last)
        {
            while (first != last)
            {
                insert(first->first, first->second);
                ++first;
            }
        }

        void erase(iterator position)
        {
            if (position == end())
                throw std::runtime_error("Invalid iterator: cannot erase item at the end() position.");

            position.vertex()->release_data();
            --m_size;
        }

        size_type erase(const key_type& key)
        {
            auto position = find(key);

            if (position == end())
            {
                // throw std::runtime_error("Invalid key: key is not found in trie.");
                return 0;
            }

            erase(position);

            return 1;
        }

        void erase(iterator first, iterator last)
        {
            while (first != last)
            {
                erase(first);
                ++first;
            }
        }

        void swap(Trie<T>& other)
        {
            std::swap(m_root, other.m_root);
            std::swap(m_size, other.m_size);
        }

        void clear()
        {
            m_root = nullptr;
            m_size = 0;
        }

        iterator find(const key_type& key) { return iterator{find_vertex(key)}; }

        const_iterator find(const key_type& key) const { return const_iterator{find_vertex(key)}; }

        _Impl::SubTrie<T> GetSubTrie(const key_type& key);


    private:
        std::shared_ptr<vertex_type> m_root{};
        size_type m_size = 0;

        std::shared_ptr<vertex_type> find_vertex(const key_type& key)
        {
            if (m_root == nullptr)
                    return nullptr;

            auto it = key.begin();
            std::shared_ptr<vertex_type> curr_vertex = m_root;

            while (it != key.end())
            {
                std::shared_ptr<vertex_type> child_vertex = curr_vertex->child_with_symbol(*it);

                if (child_vertex == nullptr)
                    return nullptr;

                ++it;
                curr_vertex = child_vertex;
            }

            return curr_vertex;
        }
    };


    namespace _Impl {

        template <typename T>
        class SubTrie {};

        template <typename T> class Vertex
        {
        public:
            using key_type = std::string;

            Vertex() = delete;

            Vertex(const char _s, std::shared_ptr<Vertex<T>> _parent) : m_symbol{_s}, m_parent{_parent} {}

            Vertex(const Vertex&) = default;
            Vertex& operator=(const Vertex&) = default;
            Vertex(Vertex&&) = default;
            Vertex& operator=(Vertex&&) = default;

            bool write_data(const key_type& key, const T& _data) 
            {
                if (m_data == nullptr)
                {
                    m_data = std::make_shared<std::pair<const key_type, T>>(key, _data);
                    m_end_of_word = true;
                    return true;
                }
                else
                {
                    m_data->second = _data;
                    m_end_of_word = true;
                    return false;
                }
            }

            void release_data()
            {
                m_data = nullptr;
                m_end_of_word = false;
            }

            char symbol() const noexcept { return m_symbol; }

            std::pair<const key_type, T>& data() const noexcept { return *m_data; }

            std::pair<const key_type, T>* data_ptr() const noexcept { return m_data.get(); }

            T& value() { return m_data->second; }

            const std::shared_ptr<Vertex> parent() const noexcept { return m_parent; }

            const std::map<char, std::shared_ptr<Vertex>>& children() const noexcept { return m_children; }

            bool end_of_word() const noexcept { return m_end_of_word; }

            std::string prefix() const
            {
                if (m_parent == nullptr)
                    return "";

                std::string _prefix{};  // т.к. это не noexcept, мы не можем сделать наш метод noexcept
                _prefix += m_symbol;

                std::shared_ptr<Vertex> current_vertex = m_parent;
                while (current_vertex != nullptr)
                {
                    _prefix += current_vertex->symbol();
                    current_vertex = current_vertex->parent();
                }

                return _prefix;
            }

            friend bool operator==(const Vertex& first, const Vertex& second)
            {
                return first.prefix() == second.prefix();
            }

            friend bool operator!=(const Vertex& first, const Vertex& second)
            {
                return !(first == second);
            }

            std::shared_ptr<Vertex<T>> next()
            {
                /*
                Этот метод находит и возвращает следующую по порядку вершину дерева, не зависимо
                от того, является она концом ключа или нет. Если метод дойдет до конца дерева, так
                и не найдя вершины, то он вернет nullptr
                */

                if (!m_children.empty())
                    return m_children.begin()->second;

                const Vertex<T>* curr_vertex = this;
                char curr_symbol{};
                while (curr_vertex->m_parent != nullptr)
                {
                    curr_symbol = curr_vertex->m_symbol;
                    curr_vertex = curr_vertex->m_parent.get();
                    for (auto& [aux_symbol, aux_vertex] : curr_vertex->m_children)
                    {
                        if (aux_symbol > curr_symbol)
                            return aux_vertex;
                    }
                }
                
                return nullptr;
            }

            std::shared_ptr<Vertex<T>> child_with_symbol(char _s)
            {
                auto it = m_children.find(_s);
                return it != m_children.end() ? it->second : nullptr;
            }

            void add_child(char _s, std::shared_ptr<Vertex<T>> _child)
            {
                m_children[_s] = _child;
            }
            

        private:
            // Почему мы вообще можем тут инициализировать поля?
            char m_symbol{};
            std::shared_ptr<std::pair<const key_type, T>> m_data{};
            std::weak_ptr<Vertex<T>> m_parent{};
            std::map<char, std::shared_ptr<Vertex<T>>> m_children{};
            bool m_end_of_word = false;
        };


        template <typename T>
        // class Iterator : public std::iterator<std::forward_iterator_tag, std::pair<const std::string, T&>>
        class Iterator
        {
        public:
            friend class Trie<T>;

            using value_type = std::pair<const std::string, T>;
            using pointer = value_type*;

            Iterator() = default;

            Iterator(std::shared_ptr<Vertex<T>> _vertex) : m_vertex{_vertex} {}

            Iterator(const Iterator& other) = default;
            Iterator& operator=(const Iterator& other) = default;
            
            Iterator(Iterator&& other) = default;
            Iterator& operator=(Iterator&& other) = default;

            Iterator& operator++()
            {
                if (m_vertex != nullptr) m_vertex = m_vertex->next();

                while ((m_vertex != nullptr) && (!m_vertex->end_of_word()))
                    m_vertex = m_vertex->next();
                return *this;
            }

            Iterator operator++(int)
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            bool operator==(const Iterator& other) const
            {
                if (m_vertex == nullptr && other.m_vertex == nullptr) return true;

                else if (m_vertex == nullptr || other.m_vertex == nullptr) return false;

                else return (*m_vertex == *(other.m_vertex));
            }

            bool operator!=(const Iterator& other) const 
            {
                return !(*this == other);
            }

            value_type& operator*()
            {
                return m_vertex->data();
            }

            pointer operator->()
            {
                return m_vertex->data_ptr();
            }

        private:
            std::shared_ptr<Vertex<T>> m_vertex{};

            std::shared_ptr<Vertex<T>> vertex() { return m_vertex; }
        };


        template <typename T>
        // class ConstIterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
        class ConstIterator
        {
        public:
            using value_type = const std::pair<const std::string, T>;
            using pointer = const value_type*;

            ConstIterator(std::shared_ptr<Vertex<T>> _vertex)
            {
                m_iterator = Iterator(_vertex);
            }

            ConstIterator(const ConstIterator& other) = default;
            ConstIterator& operator=(const ConstIterator& other) = default;
            
            ConstIterator(ConstIterator&& other) = default;
            ConstIterator& operator=(ConstIterator&& other) = default;

            ConstIterator& operator++()
            {
                ++m_iterator;
                return *this;
            }

            ConstIterator operator++(int)
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            bool operator==(const ConstIterator& other) const
            {
                return (m_iterator == other.m_iterator);
            }

            bool operator!=(const ConstIterator& other) const
            {
                return !(*this == other);
            }

            value_type& operator*()
            {
                return *m_iterator;
            }

            pointer operator->()
            {
                return &(*m_iterator);
            }


        private:
            Iterator<T> m_iterator{};
        };
    }
}
