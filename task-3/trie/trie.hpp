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
                        std::is_same_v<
                            typename std::iterator_traits<InputIterator>::value_type,
                            std::pair<const key_type, value_type>
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


        iterator begin() { return iterator{m_root}; }
        const_iterator begin() const;

        iterator end() { return iterator{nullptr}; }
        const_iterator end() const;

        bool empty() const noexcept { return m_size == 0; }

        size_type size() const noexcept { return m_size; }

        value_type& operator[](const key_type& key)
        {
            auto vertex = find_vertex(key);

            if (vertex == nullptr)
            {
                // throw std::runtime_error("Key not found:" + key);
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
            return std::pair<iterator, bool>{iterator{curr_vertex}, new_data};

        }

        template <typename InputIterator,
                    typename = std::enable_if_t<
                        std::is_base_of_v<
                            std::input_iterator_tag,
                            typename std::iterator_traits<InputIterator>::iterator_category
                        > &&
                        std::is_same_v<
                            typename std::iterator_traits<InputIterator>::value_type,
                            std::pair<const key_type, value_type>
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

        void erase(iterator position);

        size_type erase(const key_type& key);

        void erase(iterator first, iterator last);

        void swap(Trie<T>& other);

        void clear();

        iterator find(const key_type& key) { return iterator{find_vertex(key)}; }

        const_iterator find(const key_type& key) const;

        _Impl::SubTrie<T> GetSubTrie(const key_type& key);


    private:
        std::shared_ptr<vertex_type> m_root{};
        size_type m_size = 0;

        std::shared_ptr<vertex_type> find_vertex(const key_type& key)
        {
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

            /*
            Когда мы создаем новую вершину?
            Создаем промежуточную вершину без значения, только с символом
            Создаем вершину со значением
            */

            Vertex(const char _s, std::shared_ptr<Vertex<T>> _parent) : m_symbol{_s}, m_parent{_parent} {}

            // Vertex(const key_type& key, const T& _data, std::shared_ptr<Vertex<T>> _parent) 
            //     : m_symbol{_s}, 
            //       m_data{std::make_shared<std::pair<const key_type, T>>(key, _data)},
            //       m_parent{_parent},
            //       m_end_of_word{true}
            // {}

            Vertex(const Vertex&) = default;
            Vertex& operator=(const Vertex&) = default;
            Vertex(Vertex&&) = default;
            Vertex& operator=(Vertex&&) = default;

            bool write_data(const key_type& key, const T& _data) 
            {
                if (m_data == nullptr)
                {
                    m_data = std::make_shared<std::pair<const key_type, T>>(key, _data);
                    return true;
                }
                else
                {
                    m_data->second = _data;
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
            std::shared_ptr<Vertex<T>> m_parent{};
            std::map<char, std::shared_ptr<Vertex<T>>> m_children{};
            bool m_end_of_word = false;
        };


        template <typename T>
        // class Iterator : public std::iterator<std::forward_iterator_tag, std::pair<const std::string, T&>>
        class Iterator
        {
        public:
            using value_type = std::pair<const std::string, T&>;
            using pointer = value_type*;

            Iterator(std::shared_ptr<Vertex<T>> _vertex) : m_vertex{_vertex} {}

            Iterator(const Iterator& other) = default;
            Iterator& operator=(const Iterator& other) = default;
            
            Iterator(Iterator&& other) = default;
            Iterator& operator=(Iterator&& other) = default;

            Iterator& operator++()
            {
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

            bool operator==(const Iterator& other)
            {
                return (*m_vertex == *(other.m_vertex));
            }

            bool operator!=(const Iterator& other)
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
            // const Trie<T>* m_trie = nullptr;
            // std::pair<std::string, T&> m_obj{};
            std::shared_ptr<Vertex<T>> m_vertex{};

            // std::shared_ptr<Vertex<T>> next_vertex()
            // {
            //     /*
            //     Этот метод находит и возвращает следующую по порядку вершину дерева, не зависимо
            //     от того, является она концом ключа или нет. Если метод дойдет до конца дерева, так
            //     и не найдя вершины, то он вернет nullptr
            //     */

            //     /*
            //     Какие могут быть случаи:
            //     У текущей вершины есть непройденные дочерние вершины:
            //         1) Идём к первой непройденной вершине в массиве дочерних
            //     Нет непройденных дочерних вершин:
            //         2) Есть родительская вершина - идем к ней и от неё в следующую дочернюю
            //         3) Нет родительской - значит мы в корне дерева, то есть обход завершен.

            //     Условие для прекращение работы алгоритма:
            //     Мы дошли до корневой вершины, и у неё все дочерние были пройдены.

            //     Шаги следующие.
            //     1) берем первую непройденную верщину из дочерних, если таковая имеется
            //     2) Если таких нет, то возвращаемся к родительской и выполняем пункт 1)
            //     уже для родительской вершины. Если вдруг родительской нет, это значит 
            //     что мы дошли до конца дерева
            //     */

            //     char current_symbol = 0;
            //     std::shared_ptr<Vertex<T>> next_vertex{};
            //     std::shared_ptr<Vertex<T>> checking_vertex = m_vertex;

            //     bool finish = false;
            //     while (!finish)
            //     {
            //         for (auto& [symbol, v] : checking_vertex->children())
            //         {
            //             if (symbol > current_symbol)
            //             {
            //                 next_vertex = v;
            //                 break;
            //             }
            //         }

            //         if (next_vertex != nullptr)
            //         {
            //             // Нашли непройденную вершину
            //             finish = true;
            //         }
            //         else // Дочерних вершин нет
            //         {
            //             // Если есть родительская, то перемещаемся к ней
            //             if (checking_vertex->parent() != nullptr)
            //             {
            //                 current_symbol = checking_vertex->symbol();
            //                 checking_vertex = checking_vertex->parent();
            //             }
            //             else 
            //             {
            //                 // Тут алгоритм заканчивает работу
            //                 finish = true;
            //             }
            //         }
            //     }

            //     return next_vertex;
            // }
        };


        template <typename T>
        // class ConstIterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
        class ConstIterator
        {

        };
    
    }
}
