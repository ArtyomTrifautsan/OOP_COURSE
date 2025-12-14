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
#include <stack>


/*
1) Добавить member types как в std::map (mapped_type, key_type и тд.)
2) Перенести _Impl в приватную часть Trie
3) SFINAE поменять на один концепт
*/



/*
1) Добавить проверку итератора _Iter на соответствие стандартному концепту forward_iterator
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


        Trie()
        {
            m_root = Node::create("", 0);
        }

        template <InputIteratorConcept<value_type> InputIterator>
        Trie(InputIterator first, InputIterator last)
        {
            m_root = Node::create("", 0);

            insert(first, last);
        }


        Trie(const Trie& other)
        {
            m_root = Node::create("", 0);

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
            if (key.length() == 0)
                throw std::runtime_error("Insert by invalid key: key == \"\"");

            std::shared_ptr<Node> curr_node = m_root;
            size_type next_node_index = 0;
            key_type next_node_key = "";
            size_type deep = 0;
            bool new_value = false;
            bool suicide = false;

            while (!suicide)
            {
                next_node_index = static_cast<size_type>(static_cast<unsigned char>(key[deep]));

                next_node_key += key[deep];

                if (next_node_key == key)
                {
                    if (curr_node->m_children[next_node_index] == nullptr)
                    {
                        curr_node->m_children[next_node_index] = Node::create(next_node_key, next_node_index, std::weak_ptr<Node>(curr_node), value);

                        curr_node = curr_node->m_children[next_node_index];

                        new_value = true;

                        curr_node->m_has_value = true;

                        suicide = true;
                    }
                    else
                    {
                        curr_node = curr_node->m_children[next_node_index];

                        curr_node->m_data.second = value;
                        
                        new_value = !(curr_node->m_has_value);

                        curr_node->m_has_value = true;

                        suicide = true;
                    }
                }
                else
                {
                    // Если такой ветки еще нет
                    if (curr_node->m_children[next_node_index] == nullptr)
                    {
                        curr_node->m_children[next_node_index] = Node::create(next_node_key, next_node_index, std::weak_ptr<Node>(curr_node));
                    }

                    curr_node = curr_node->m_children[next_node_index];

                    ++deep;
                }
            }

            std::shared_ptr<Node> inserted_node = curr_node;
            
            if (new_value)
            {
                while(curr_node != nullptr)
                {

                    ++(curr_node->m_subtree_size);

                    auto par_node = curr_node->m_parent;

                    curr_node = par_node.lock();
                }
            }

            return std::pair<iterator, bool>{iterator{inserted_node}, new_value};
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

            erase(position->first);
        }

        size_type erase(const key_type& key)
        {
            std::shared_ptr<Node> curr_node = m_root;
            size_type next_node_index = 0;
            bool suicide = false;
            size_type deep = 0;
            size_type deleted_values = 0;
            
            // Этот флаг сообщает, нужно ли вернуться к прошлым вершинам и уменьшить им m_subtree_size,
            // и, возможно, удалить ненужные поддеревья
            bool go_back = false;

            while(!suicide)
            {
                if (curr_node->m_data.first == key)
                {
                    if (curr_node->m_has_value)
                    {
                        ++deleted_values;
                        
                        curr_node->m_data.second = mapped_type{};

                        curr_node->m_has_value = false;

                        // Эта операция будет выполнена в блоке go_back
                        // --(curr_node->m_subtree_size);

                        go_back = true;
                    }

                    suicide = true;
                }
                else
                {
                    next_node_index = static_cast<size_type>(static_cast<unsigned char>(key[deep]));

                    // Если такой ветки нет
                    if (curr_node->m_children[next_node_index] == nullptr)
                    {
                        return 0;
                    }

                    curr_node = curr_node->m_children[next_node_index];

                    ++deep;
                }
            }

            if (go_back)
            {
                bool delete_child = false;

                while(curr_node != nullptr)
                {
                    if (delete_child)
                    {
                        curr_node->m_children[next_node_index] = nullptr;

                        delete_child = false;
                    }

                    --(curr_node->m_subtree_size);

                    if (curr_node->m_subtree_size == 0)
                    {
                        delete_child = true;
                        next_node_index = curr_node->m_position;
                    }

                    auto par_node = curr_node->m_parent;

                    curr_node = par_node.lock();
                }
            }

            return deleted_values;
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
            return iterator(find_node(key));
        }

        const_iterator find(const key_type& key) const
        {
            return const_iterator(find_node(key));
        }

        SubTrie GetSubTrie(const key_type& key)
        {
            std::shared_ptr<Node> m_subtree_root = find_node(key);

            if (m_subtree_root == nullptr)
                throw std::runtime_error("Invalid key error: The key is not found in the trie.");

            auto par = m_subtree_root->parent();
            return SubTrie(m_subtree_root, par);
        }


    private:
        std::shared_ptr<Node> m_root = nullptr;

        std::shared_ptr<Node> first_node_with_value() const
        {
            if (m_root->has_value()) return m_root;

            return m_root->next_node_with_value();
        }


        std::shared_ptr<Node> find_node(const key_type& key) const
        {
            std::shared_ptr<Node> curr_node = m_root;
            size_type next_node_index = 0;
            bool suicide = false;
            size_type deep = 0;
            bool new_value = false;

            while(!suicide)
            {
                if (curr_node->m_data.first == key)
                {
                    if (curr_node->m_has_value)
                        return curr_node;
                    else
                        return nullptr;
                }
                else
                {
                    next_node_index = static_cast<size_type>(static_cast<unsigned char>(key[deep]));

                    // Если такой ветки еще нет
                    if (curr_node->m_children[next_node_index] == nullptr)
                    {
                        return nullptr;
                    }

                    curr_node = curr_node->m_children[next_node_index];

                    ++deep;
                }
            }

            // Просто чтобы компилятор не ругался на то что доходит до конца не void функции
            return curr_node;
        }


        class SubTrie
        {
        public:

            class ConstIterator
            {
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type        = Containers::Trie<T>::value_type;
                using difference_type   = std::ptrdiff_t;
                using pointer           = const value_type*;
                using reference         = const value_type&;

                ConstIterator() = default;

                ConstIterator(std::shared_ptr<Node> node, std::shared_ptr<Node> end_node)
                    : m_node(node), m_end_node(end_node)
                {
                    // while (n != nullptr && !(n->has_value())) 
                    //     n = n->next(m_end_node);
                }

                ConstIterator& operator++()
                {
                    if (m_node != nullptr) m_node = m_node->next_node_with_value(m_end_node);

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
                    return m_node == other.m_node;
                }

                bool operator!=(const ConstIterator& other) const 
                {
                    return !(*this == other);
                }

                reference  operator*()
                {
                    if (m_node == m_end_node) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                    return m_node->data();
                }

                pointer operator->()
                {
                    if (m_node == m_end_node) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                    return m_node->pdata();
                }

            private:
                friend class SubTrie;

                std::shared_ptr<Node> m_node = nullptr;
                std::shared_ptr<Node> m_end_node = nullptr;  // граница поддерева
            };


            // --- интерфейс SubTrie ---

            SubTrie(std::shared_ptr<Node> r, std::shared_ptr<Node> _end_node) : m_subtree_root(r), m_end_node{_end_node} {}

            SubTrie(const SubTrie&) = delete;
            SubTrie& operator=(const SubTrie&) = delete;
            SubTrie(SubTrie&&) = default;
            SubTrie& operator=(SubTrie&&) = default;

            ConstIterator begin() const 
            {
                return ConstIterator{first_node_with_value(), m_end_node};
            }

            ConstIterator end() const {
                return ConstIterator{m_end_node, m_end_node};
            }

            bool empty() const noexcept { return size() == 0; }

            size_type size() const noexcept {
                return m_subtree_root ? m_subtree_root->subtree_size() : 0;
            }

        private:
            std::shared_ptr<Node> m_subtree_root = nullptr;
            std::shared_ptr<Node> m_end_node = nullptr;

            std::shared_ptr<Node> first_node_with_value() const
            {
                if (m_subtree_root->has_value()) return m_subtree_root;

                return m_subtree_root->next_node_with_value(m_end_node);
            }

        };


        class Node : public std::enable_shared_from_this<Node>
        {
        public:
            static std::shared_ptr<Node> create(const std::string& key, size_type _pos, std::weak_ptr<Node> parent = {}, const mapped_type& value = {})
            {
                auto node = std::make_shared<Node>(key, _pos, parent, value);
                return node;
            }

            Node(const std::string& key, size_type _pos, std::weak_ptr<Node> parent, const mapped_type& value) : m_data{key, value}, m_parent{parent}, m_position{_pos} {}

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&&) = delete;
            Node& operator=(Node&&) = delete;

            std::shared_ptr<Node> next_node_with_value(std::shared_ptr<Node> _end = nullptr)
            {
                std::shared_ptr<Node> n = next(_end);

                while (n != _end && !(n->has_value())) 
                    n = n->next(_end);

                return n;
            }

            std::shared_ptr<const Node> next_node_with_value(std::shared_ptr<const Node> _end = nullptr) const
            {
                std::shared_ptr<const Node> n = next(_end);

                while (n != _end && !(n->has_value())) 
                    n = n->next(_end);

                return n;
            }

            void clear()
            {
                for (size_type i = 0; i < m_children.size(); i++)
                {
                    if (m_children[i] != nullptr)
                        m_children[i] = nullptr;
                }
                m_subtree_size = 0;
            }

            reference data() { return m_data; }
            pointer pdata() { return &m_data; }

            bool has_value() const noexcept { return m_has_value; }

            size_type subtree_size() const noexcept { return m_subtree_size; }

            key_type key() const noexcept { return m_data.first; }

            std::shared_ptr<Node> parent() { return m_parent.lock(); }

            size_type position() const noexcept { return m_position; }

            value_type m_data{};
            std::weak_ptr<Node> m_parent{};
            std::array<std::shared_ptr<Node>, 256> m_children{};
            bool m_has_value = false;

            // m_subtree_size - сколько элементов хранится в поддереве, корнем
            // которого является текущая вершина
            size_type m_subtree_size = 0;

            // Позиция текущей вершины в массиве потомков у родителя
            size_type m_position{};

            std::shared_ptr<Node> next(std::shared_ptr<Node> _end = nullptr)
            {
                // return recoursive_next(0);
                return cycle_next(_end);
            }

            std::shared_ptr<const Node> next(std::shared_ptr<const Node> _end = nullptr) const
            {
                return cycle_next(_end);
            }


            std::shared_ptr<Node> cycle_next(std::shared_ptr<Node> _end_node = nullptr)
            {
                // Вершина _end_node - это вершина "после последней". Итератор на такую
                // вершину - это end итератор.  Мне понадобилось это ввести, чтобы этот 
                // класс итераторов работал и с SubTrie. Для Trie _end_node будет равен
                // nullptr, для SubTrie значение _end_node будет m_subtree_root->parent.

                // index - индекс потомка, с которого начинаем поиск 
                // следующей вершины в массиве m_children
                size_type index = 0;
                bool finish = false;
                std::shared_ptr<Node> curr_node = this->shared_from_this();

                while(!finish)
                {
                    for (size_type i = index; i < curr_node->m_children.size(); i++)
                    {
                        if (curr_node->m_children[i] != nullptr)
                        {
                            return curr_node->m_children[i];
                            // finish = true;
                        }
                    }
                    auto sp2 = curr_node->m_parent.lock();

                    if (sp2 == nullptr) return _end_node;

                    if (sp2 == _end_node) return _end_node;

                    index = curr_node->m_position + 1;

                    curr_node = sp2;
                }

                // Код до сюда никогда не дойдет, но я написал это чтобы компилятор не 
                // ругался словом warning.
                return curr_node;
            }


            std::shared_ptr<const Node> cycle_next(std::shared_ptr<Node> _end_node = nullptr) const
            {
                return const_cast<const Node*>(this)->cycle_next(_end_node);
            }

            void check_prefix_is_correct(const key_type& prefix, const key_type& key) const
            {
                if (!key.starts_with(prefix))
                    throw std::runtime_error("Internal error. The algorithm chose the wrong path to the node with this key: " + key + ". The prefix: " + prefix + ".");
            }

            void check_key_is_correct(const key_type& key) const
            {
                if (!key.starts_with(m_data.first))
                    throw std::runtime_error("Internal error. The algorithm chose the wrong path to the node with this key: " + key + ". The prefix of node: " + m_data.first + ".");
            }
        };


        template <bool const_iter>
        class Iterator
        {
        public:
        /*
        перенести сюда std::condition
        */
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

            reference  operator*()
            {
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                return m_node->data();
            }

            pointer operator->()
            {
                if (m_node == nullptr) throw std::runtime_error("Cannot dereference the iterator with nullptr.");

                return m_node->pdata();
            }

        private:
            std::shared_ptr<Node> m_node = nullptr;
        };

    };

}
