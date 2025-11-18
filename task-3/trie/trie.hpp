#pragma once

#include <string>
#include <utility>
#include <array>
#include <map>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <type_traits>


/*
1) Добавить member types как в std::map (mapped_type, key_type и тд.)
2) Перенести _Impl в приватную часть Trie
3) SFINAE поменять на один концепт
*/

namespace Containers
{

    template <typename T>
    class Trie
    {
    public:
        using key_type = std::string;
        using mapped_type = T;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;


        template<typename _Iter>
        concept InputIteratorConcept = std::is_base_of_v<
                            std::input_iterator_tag,
                            typename std::iterator_traits<_Iter>::iterator_category
                        > &&
                        std::is_convertible_v<
                            std::pair<const key_type, value_type>,
                            typename std::iterator_traits<_Iter>::value_type
                        >;


        // using vertex_type = _Impl::Vertex<T>;

        Trie() = default;

        template <InputIteratorConcept InputIterator>
        Trie(InputIterator first, InputIterator last)
        {
            insert(first, last);
        }


        Trie(const Trie& other) = default;


        Trie& operator=(const Trie& other) = default;


        Trie(Trie&& other) = default;


        Trie& operator=(Trie&& other) = default;


        iterator begin()
        {
            return iterator{first_node_with_value()};
        }

        const_iterator begin() const
        {
            return const_iterator{first_node_with_value()};
        }

        iterator end() { return iterator{nullptr}; }
        const_iterator end() const { return const_iterator{nullptr}; }

        bool empty() const noexcept { return m_size == 0; }

        size_type size() const noexcept { return m_size; }

        mapped_type& operator[](const key_type& key)
        {
            auto it = find(key);

            if (it == end())
            {
                auto new_it = insert(key, value_type{}).first;
                return (*new_it).second;
            }

            return (*it).second;;
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

        template <InputIteratorConcept InputIterator>
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

        iterator find(const key_type& key) { return iterator{find_vertex(key)};  } // использовать тут stdd::find_if

        const_iterator find(const key_type& key) const { return const_iterator{find_vertex(key)}; }

        _Impl::SubTrie<T> GetSubTrie(const key_type& key);


    private:
        std::shared_ptr<Node> m_root{};
        size_type m_size = 0;

        std::shared_ptr<Node> first_node_with_value()
        {
            if (m_root == nullptr) return nullptr;

            if (m_root->has_value()) return m_root;

            return m_root->next_node_with_value();
        }

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


        class SubTrie {};


        class Node : public std::enable_shared_from_this<Node>
        {
        public:
            static std::shared_ptr<Node> create(const key_type& key, std::weak_ptr<Node> parent = {})
            {
                auto node = std::make_shared<Node>(key, parent);
                return node;
            }

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&&) = delete;
            Node& operator=(Node&&) = delete;

            bool insert(const key_type& key, const mapped_type& value)
            {
                if (key == m_data.first)
                {
                    m_data.second = value;

                    bool new_word = !m_has_value;
                    m_has_value = true;
                    return new_word;
                }

                char symbol = key[m_data.first.length()];
                size_type index = static_cast<size_type>(static_cast<unsigned char>(symbol));

                if (m_children[index] == nullptr)
                    m_children[index] = create(m_data.first + symbol, std::weak_ptr<Node>(shared_from_this()));

                ++values;
                return m_children[index]->insert(key, value);
            }

            std::shared_ptr<Node> next_node_with_value()
            {
                std::shared_ptr<Node> n = next();

                while (n != nullptr && !(n->has_value())) 
                    n = n->next();
                
                return n;
            }

            reference data() { return m_data; }
            pointer pdata() { return &m_data; }

            bool has_value() { return m_has_value; }

        private:
            value_type m_data{};
            std::weak_ptr<Node> m_parent{};
            std::array<std::shared_ptr<Node>, 256> m_children{};
            bool m_has_value = false;

            // values - сколько элементов хранят потомки. Если values == 0,
            // то удаляем всех потомков
            int values = 0;

            Node(const key_type& key, std::weak_ptr<Node> parent)
            {
                // m_data = std::pair<const key_type, T>{key, T()};
                m_data = {key, mapped_type()};
                m_parent = parent;
            }

            char symbol() { return m_data.first[m_data.first.length() - 1]; }

            std::shared_ptr<Node> next()
            {
                return recoursive_next(0);
            }

            std::shared_ptr<Node> recoursive_next(size_type index = 0)
            {
                for (size_type i = index; i < m_children.size(); i++)
                {
                    if (m_children[i] != nullptr) return m_children[i];
                }

                if (m_parent == nullptr) return nullptr;

                return m_parent->recoursive_next(static_cast<size_type>(static_cast<unsigned char>(symbol())) + 1);
            }
        };


        template <bool const_iter>
        class Iterator
        {
        public:
            Iterator(std::shared_ptr<Node> node) : m_node{node} {}

            Iterator& operator++()
            {
                if (m_node != nullptr) m_node = m_node->next_node_with_value();

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
                return m_node == other.m_node;
            }

            bool operator!=(const Iterator& other) const 
            {
                return !(*this == other);
            }

            std::conditional<const_iter, const reference, reference>::type  operator*()
            {
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the nullptr.");

                return m_node->data();
            }

            std::conditional<const_iter, const pointer, pointer>::type operator->()
            {
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the nullptr.");

                return m_node->pdata();
            }

        private:
            std::shared_ptr<Node> m_node = nullptr;
        };

    };


    namespace _Impl {
        

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


        // std::conditional - добавить
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
