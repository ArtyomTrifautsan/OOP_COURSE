#pragma once

#include <string>
#include <utility>
#include <array>
#include <map>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <algorithm>


/*
1) Добавить member types как в std::map (mapped_type, key_type и тд.)
2) Перенести _Impl в приватную часть Trie
3) SFINAE поменять на один концепт
*/

namespace Containers
{
    template<typename _Iter, typename _ValueType>
    concept InputIteratorConcept = std::is_base_of_v<
                                        std::input_iterator_tag,
                                        typename std::iterator_traits<_Iter>::iterator_category
                                    > &&
                                    std::is_convertible_v<
                                        typename std::iterator_traits<_Iter>::value_type,
                                        _ValueType
                                    >;

    template <typename T>
    class Trie
    {
    private:
        template <bool const_iter> class Iterator;
        class SubTrie;
        class Node;

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


        // using vertex_type = _Impl::Vertex<T>;

        Trie()
        {
            m_root = Node::create(0);
        }

        template <InputIteratorConcept<value_type> InputIterator>
        Trie(InputIterator first, InputIterator last)
        {
            m_root = Node::create(0);

            insert(first, last);
        }


        Trie(const Trie& other)
        {
            m_root = Node::create(0);

            insert(other.begin(), other.end());
        }


        Trie& operator=(const Trie& other)
        {
            if (this == &other) return *this;

            clear();

            insert(other.begin(), other.end());

            return *this;
        }


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

        bool empty() const noexcept { return size() == 0; }

        size_type size() const noexcept { return m_root->subtree_size(); }

        mapped_type& operator[](const key_type& key)
        {
            auto it = find(key);

            if (it == end())
            {
                auto new_it = insert(key, mapped_type{}).first;
                return (*new_it).second;
            }

            return (*it).second;
        }

        std::pair<iterator, bool> insert(const key_type& key, const mapped_type& value)
        {
            return m_root->insert("", key, value);
        }

        template <InputIteratorConcept<value_type> InputIterator>
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
                throw std::runtime_error("Invalid iterator: the end() iterator cannot be used as a value for position.");

            m_root->erase("", position->first);
        }

        size_type erase(const key_type& key)
        {
            return m_root->erase("", key);
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
            if (this == &other) return;
            std::swap(m_root, other.m_root);
        }

        void clear()
        {
            m_root->clear();
        }

        iterator find(const key_type& key)
        {
            // return std::find_if(begin(), end(), [&key](value_type& v) { return v.first == key; });

            return iterator(m_root->find("", key));
        }

        const_iterator find(const key_type& key) const
        {
            return const_iterator(m_root->find("", key));
        }

        SubTrie GetSubTrie(const key_type& key)
        {
            std::shared_ptr<Node> m_subtree_root = m_root->find("", key);

            if (m_subtree_root == nullptr)
                throw std::runtime_error("Invalid key error: The key is not found in the trie.");

            return SubTrie(m_subtree_root);
        }


    private:
        std::shared_ptr<Node> m_root = nullptr;

        std::shared_ptr<Node> first_node_with_value() const
        {
            if (m_root->has_value()) return m_root;

            return m_root->next_node_with_value();
        }


        class SubTrie
        {
        public:
            SubTrie(std::shared_ptr<Node> r) : m_subtree_root{r}
            {
                // m_begin = std::make_shared<const_iterator>(m_subtree_root);
                // std::shared_ptr<Node> parent_ptr = m_subtree_root->parent().lock();
                // m_end = std::make_shared<const_iterator>(parent_ptr);
            }

            SubTrie(const SubTrie&) = delete;
            SubTrie& operator=(const SubTrie&) = delete;

            SubTrie(SubTrie&&) = default;
            SubTrie& operator=(SubTrie&&) = default;

            const_iterator begin() const 
            { 
                return const_iterator{m_subtree_root}; 
            };

            const_iterator end() const 
            {
                return const_iterator{m_subtree_root->parent()};
            };

            bool empty() const noexcept { return size() == 0; }

            size_type size() const noexcept { return m_subtree_root->subtree_size(); }

        private:
            std::shared_ptr<Node> m_subtree_root = nullptr;
            // std::shared_ptr<const_iterator> m_begin = nullptr;
            // std::shared_ptr<const_iterator> m_end = nullptr;
        };


        class Node : public std::enable_shared_from_this<Node>
        {
        public:
            static std::shared_ptr<Node> create(size_type _pos, std::weak_ptr<Node> parent = {})
            {
                auto node = std::make_shared<Node>(_pos, parent);
                return node;
            }

            Node(size_type _pos, std::weak_ptr<Node> parent) : m_node_position{_pos}, m_parent{parent} {}

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&&) = delete;
            Node& operator=(Node&&) = delete;

            std::pair<iterator, bool> insert(const key_type& prefix, const key_type& key, const mapped_type& value)
            {
                check_prefix_is_correct(prefix, key);

                if (key == prefix)
                {
                    m_data = std::make_shared<value_type>(key, value);

                    bool new_word_flag = !m_has_value;
                    m_has_value = true;

                    m_subtree_size += static_cast<int>(new_word_flag);

                    return std::pair<iterator, bool>{iterator{this->shared_from_this()}, new_word_flag};
                }

                char next_symbol = key[prefix.length()];
                size_type next_node_index = static_cast<size_type>(static_cast<unsigned char>(next_symbol));

                if (m_children[next_node_index] == nullptr)
                    m_children[next_node_index] = Node::create(next_node_index, std::weak_ptr<Node>(this->shared_from_this()));

                auto result = m_children[next_node_index]->insert(prefix + next_symbol, key, value);

                if (result.second)
                    ++m_subtree_size;

                return result;
            }

            size_type erase(const key_type& prefix, const key_type& key)
            {
                check_prefix_is_correct(prefix, key);

                if (key == prefix)
                {
                    m_data = nullptr;

                    size_type erased = static_cast<size_type>(m_has_value);
                    m_has_value = false;

                    m_subtree_size -= erased;

                    return erased;
                }

                char next_symbol = key[prefix.length()];
                size_type next_node_index = static_cast<size_type>(static_cast<unsigned char>(next_symbol));

                if (m_children[next_node_index] == nullptr)
                {
                    // throw std::runtime_error("Key error. This key is not in the trie: " + key);
                    return 0;
                }

                size_type erased = m_children[next_node_index]->erase(prefix + next_symbol, key);

                m_subtree_size -= erased;

                if (m_children[next_node_index]->m_subtree_size == 0)
                    m_children[next_node_index] = nullptr;

                return erased;
            }

            std::shared_ptr<Node> find(const key_type& prefix, const key_type& key)
            {
                check_prefix_is_correct(prefix, key);

                if (key == prefix)
                {
                    if (m_has_value)
                        return this->shared_from_this();

                    return nullptr;
                }

                char next_symbol = key[prefix.length()];
                size_type next_node_index = static_cast<size_type>(static_cast<unsigned char>(next_symbol));

                if (m_children[next_node_index] == nullptr)
                    return nullptr;

                return m_children[next_node_index]->find(prefix + next_symbol, key);
            }

            std::shared_ptr<Node> next_node_with_value() const
            {
                std::shared_ptr<Node> n = next();

                while (n != nullptr && !(n->has_value())) 
                    n = n->next();

                return n;
            }

            reference data() { return *m_data; }
            pointer pdata() { return m_data.get(); }

            bool has_value() const noexcept { return m_has_value; }

            size_type subtree_size() const noexcept { return m_subtree_size; }

            void clear()
            {
                for (size_type i = 0; i < m_children.size(); i++)
                {
                    if (m_children[i] != nullptr)
                        m_children[i] = nullptr;
                }
                m_subtree_size = 0;
            }

            std::shared_ptr<Node> parent() { return m_parent.lock(); }

        private:
            std::shared_ptr<value_type> m_data{};
            std::weak_ptr<Node> m_parent{};
            std::array<std::shared_ptr<Node>, 256> m_children{};
            bool m_has_value = false;

            // m_subtree_size - сколько элементов хранится в поддереве, корнем
            // которого является текущая вершина
            size_type m_subtree_size = 0;

            // Позиция текущей вершины в массиве потомков у родителя
            size_type m_node_position{};

            // char symbol() const { return m_node_symbol; }

            std::shared_ptr<Node> next() const
            {
                return recoursive_next(0);
            }

            std::shared_ptr<Node> recoursive_next(size_type index = 0) const
            {
                // index - индекс потомка, с которого начинаем поиск следующей вершины
                for (size_type i = index; i < m_children.size(); i++)
                {
                    if (m_children[i] != nullptr) return m_children[i];
                }

                auto sp2 = m_parent.lock();

                if (sp2 == nullptr) return nullptr;

                return sp2->recoursive_next(static_cast<size_type>(m_node_position + 1));
            }

            // size_type next_node_in_key(const key_type& key)
            // {
            //     // Метод возвращает индекс потомка в массиве потомков, который 
            //     // соответствует следующей вершине по ключу
            //     return static_cast<size_type>(static_cast<unsigned char>(key[m_data->first.length()]));
            // }

            void check_prefix_is_correct(const key_type& prefix, const key_type& key) const
            {
                if (!key.starts_with(prefix))
                    throw std::runtime_error("Internal error. The algorithm chose the wrong path to the node with this key: " + key + ". The prefix: " + prefix + ".");
            }
        };


        template <bool const_iter>
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Containers::Trie<T>::value_type;
            using difference_type = std::ptrdiff_t;
            using pointer = std::conditional_t<const_iter, const value_type*, value_type*>;
            using reference = std::conditional_t<const_iter, const value_type&, value_type&>;


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
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                return m_node->data();
            }

            std::conditional<const_iter, const pointer, pointer>::type operator->()
            {
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                return m_node->pdata();
            }

        private:
            std::shared_ptr<Node> m_node = nullptr;
        };

    };


    // namespace _Impl {
        

    //     template <typename T> class Vertex
    //     {
    //     public:
    //         using key_type = std::string;

    //         Vertex() = delete;

    //         Vertex(const char _s, std::shared_ptr<Vertex<T>> _parent) : m_symbol{_s}, m_parent{_parent} {}

    //         Vertex(const Vertex&) = default;
    //         Vertex& operator=(const Vertex&) = default;
    //         Vertex(Vertex&&) = default;
    //         Vertex& operator=(Vertex&&) = default;

    //         bool write_data(const key_type& key, const T& _data) 
    //         {
    //             if (m_data == nullptr)
    //             {
    //                 m_data = std::make_shared<std::pair<const key_type, T>>(key, _data);
    //                 m_end_of_word = true;
    //                 return true;
    //             }
    //             else
    //             {
    //                 m_data->second = _data;
    //                 m_end_of_word = true;
    //                 return false;
    //             }
    //         }

    //         void release_data()
    //         {
    //             m_data = nullptr;
    //             m_end_of_word = false;
    //         }

    //         char symbol() const noexcept { return m_symbol; }

    //         std::pair<const key_type, T>& data() const noexcept { return *m_data; }

    //         std::pair<const key_type, T>* data_ptr() const noexcept { return m_data.get(); }

    //         T& value() { return m_data->second; }

    //         const std::shared_ptr<Vertex> parent() const noexcept { return m_parent; }

    //         const std::map<char, std::shared_ptr<Vertex>>& children() const noexcept { return m_children; }

    //         bool end_of_word() const noexcept { return m_end_of_word; }

    //         std::string prefix() const
    //         {
    //             if (m_parent == nullptr)
    //                 return "";

    //             std::string _prefix{};  // т.к. это не noexcept, мы не можем сделать наш метод noexcept
    //             _prefix += m_symbol;

    //             std::shared_ptr<Vertex> current_vertex = m_parent;
    //             while (current_vertex != nullptr)
    //             {
    //                 _prefix += current_vertex->symbol();
    //                 current_vertex = current_vertex->parent();
    //             }

    //             return _prefix;
    //         }

    //         friend bool operator==(const Vertex& first, const Vertex& second)
    //         {
    //             return first.prefix() == second.prefix();
    //         }

    //         friend bool operator!=(const Vertex& first, const Vertex& second)
    //         {
    //             return !(first == second);
    //         }

    //         std::shared_ptr<Vertex<T>> next()
    //         {
    //             /*
    //             Этот метод находит и возвращает следующую по порядку вершину дерева, не зависимо
    //             от того, является она концом ключа или нет. Если метод дойдет до конца дерева, так
    //             и не найдя вершины, то он вернет nullptr
    //             */

    //             if (!m_children.empty())
    //                 return m_children.begin()->second;

    //             const Vertex<T>* curr_vertex = this;
    //             char curr_symbol{};
    //             while (curr_vertex->m_parent != nullptr)
    //             {
    //                 curr_symbol = curr_vertex->m_symbol;
    //                 curr_vertex = curr_vertex->m_parent.get();
    //                 for (auto& [aux_symbol, aux_vertex] : curr_vertex->m_children)
    //                 {
    //                     if (aux_symbol > curr_symbol)
    //                         return aux_vertex;
    //                 }
    //             }
                
    //             return nullptr;
    //         }

    //         std::shared_ptr<Vertex<T>> child_with_symbol(char _s)
    //         {
    //             auto it = m_children.find(_s);
    //             return it != m_children.end() ? it->second : nullptr;
    //         }

    //         void add_child(char _s, std::shared_ptr<Vertex<T>> _child)
    //         {
    //             m_children[_s] = _child;
    //         }
            

    //     private:
    //         // Почему мы вообще можем тут инициализировать поля?
    //         char m_symbol{};
    //         std::shared_ptr<std::pair<const key_type, T>> m_data{};
    //         std::weak_ptr<Vertex<T>> m_parent{};
    //         std::map<char, std::shared_ptr<Vertex<T>>> m_children{};
    //         bool m_end_of_word = false;
    //     };


    //     template <typename T>
    //     // class Iterator : public std::iterator<std::forward_iterator_tag, std::pair<const std::string, T&>>
    //     class Iterator
    //     {
    //     public:
    //         friend class Trie<T>;

    //         using value_type = std::pair<const std::string, T>;
    //         using pointer = value_type*;

    //         Iterator() = default;

    //         Iterator(std::shared_ptr<Vertex<T>> _vertex) : m_vertex{_vertex} {}

    //         Iterator(const Iterator& other) = default;
    //         Iterator& operator=(const Iterator& other) = default;
            
    //         Iterator(Iterator&& other) = default;
    //         Iterator& operator=(Iterator&& other) = default;

    //         Iterator& operator++()
    //         {
    //             if (m_vertex != nullptr) m_vertex = m_vertex->next();

    //             while ((m_vertex != nullptr) && (!m_vertex->end_of_word()))
    //                 m_vertex = m_vertex->next();
    //             return *this;
    //         }

    //         Iterator operator++(int)
    //         {
    //             auto tmp = *this;
    //             ++*this;
    //             return tmp;
    //         }

    //         bool operator==(const Iterator& other) const
    //         {
    //             if (m_vertex == nullptr && other.m_vertex == nullptr) return true;

    //             else if (m_vertex == nullptr || other.m_vertex == nullptr) return false;

    //             else return (*m_vertex == *(other.m_vertex));
    //         }

    //         bool operator!=(const Iterator& other) const 
    //         {
    //             return !(*this == other);
    //         }

    //         value_type& operator*()
    //         {
    //             return m_vertex->data();
    //         }

    //         pointer operator->()
    //         {
    //             return m_vertex->data_ptr();
    //         }

    //     private:
    //         std::shared_ptr<Vertex<T>> m_vertex{};

    //         std::shared_ptr<Vertex<T>> vertex() { return m_vertex; }
    //     };


    //     // std::conditional - добавить
    //     template <typename T>
    //     // class ConstIterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
    //     class ConstIterator
    //     {
    //     public:
    //         using value_type = const std::pair<const std::string, T>;
    //         using pointer = const value_type*;

    //         ConstIterator(std::shared_ptr<Vertex<T>> _vertex)
    //         {
    //             m_iterator = Iterator(_vertex);
    //         }

    //         ConstIterator(const ConstIterator& other) = default;
    //         ConstIterator& operator=(const ConstIterator& other) = default;
            
    //         ConstIterator(ConstIterator&& other) = default;
    //         ConstIterator& operator=(ConstIterator&& other) = default;

    //         ConstIterator& operator++()
    //         {
    //             ++m_iterator;
    //             return *this;
    //         }

    //         ConstIterator operator++(int)
    //         {
    //             auto tmp = *this;
    //             ++*this;
    //             return tmp;
    //         }

    //         bool operator==(const ConstIterator& other) const
    //         {
    //             return (m_iterator == other.m_iterator);
    //         }

    //         bool operator!=(const ConstIterator& other) const
    //         {
    //             return !(*this == other);
    //         }

    //         value_type& operator*()
    //         {
    //             return *m_iterator;
    //         }

    //         pointer operator->()
    //         {
    //             return &(*m_iterator);
    //         }


    //     private:
    //         Iterator<T> m_iterator{};
    //     };
    // }
}
