#pragma once

#include <string>
#include <utility>
#include <array>
#include <map>
#include <memory>



/*
a = 97
b = 98
c = 99
d = 100
e = 101
f = 102
g = 103
h = 104
i = 105
j = 106
k = 107
l = 108
m = 109
n = 110
o = 111
p = 112
q = 113
r = 114
s = 115
t = 116
u = 117
v = 118
w = 119
x = 120
y = 121
z = 122
*/


namespace ns_Trie
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

        Trie() = default;

        template <typename InputIterator>
        Trie(InputIterator first, InputIterator last)
        {

        }


        Trie(const Trie<T>& other) = default;


        Trie<T>& operator=(const Trie<T> other) = default;


        Trie(Trie<T>&& other) = default;


        Trie<T>& operator=(Trie<T>&& other) = default;


        iterator begin();
        const_iterator begin() const;

        iterator end();
        const_iterator end() const;

        bool empty() const;

        size_type size() const;

        value_type operator[](const key_type& key);

        std::pair<iterator, bool> insert(const key_type& key, const value_type& value);

        template <typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        void erase(iterator position);

        size_type erase(const key_type& key);

        void erase(iterator first, iterator last);

        void swap(Trie<T>& other);

        void clear();

        iterator find(const key_type& key);

        const_iterator find(const key_type& key) const;

        _Impl::SubTrie<T> GetSubTrie(const key_type& key);

    protected:
        // friend class _Impl::Vertex<T>;

        _Impl::Vertex<T>* get_next_vertex();

    private:
        using vertex_ptr_t = std::shared_ptr<_Impl::Vertex<T>>;
        vertex_ptr_t m_root_ptr = nullptr;
        size_type m_size = 0;
    };


    namespace _Impl {

        template <typename T>
        class SubTrie 
        {

        };

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

            Vertex(const char _s, std::shared_ptr<Vertex<T>> _parent)
            {
                m_s = _s;
                m_parent = _parent;
            }

            Vertex(const key_type& key, const T& _data, std::shared_ptr<Vertex<T>> _parent)
            {
                m_s = _s;
                m_data = std::make_shared<std::pair<const key_type, T>>(key, _data);
                m_parent = _parent;
                m_end_of_word = true;
            }

            Vertex(const Vertex&) = default;
            Vertex& operator=(const Vertex&) = default;
            Vertex(Vertex&&) = default;
            Vertex& operator=(Vertex&&) = default;

            bool write_data(const key_type& key, const T& _data) 
            {
                if (m_data == nullptr)
                {
                    m_data = std::make_shared<std::pair<const key_type, T>>({key, _data});
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

            char symbol() const noexcept { return m_s; }
            std::pair<const key_type, T>& data() const noexcept { return *m_data; }
            std::pair<const key_type, T>* data_ptr() const noexcept { return m_data.get(); }
            const std::shared_ptr<Vertex> parent() const noexcept { return m_parent; }
            const std::map<char, std::shared_ptr<Vertex>>& children() const noexcept { return m_children; }
            bool end_of_word() const noexcept { return m_end_of_word; }
            // bool& end_of_word() noexcept { return m_end_of_word; }

            std::string prefix() const
            {
                if (m_parent == nullptr)
                    return "";

                std::string _prefix{};  // т.к. это не noexcept, мы не можем сделать наш метод noexcept
                _prefix += m_s;

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

            std::shared_ptr<Vertex<T>> next_vertex()
            {
                // char current_symbol = 0;
                // std::shared_ptr<Vertex<T>> next_vertex{};
                // std::shared_ptr<Vertex<T>> checking_vertex = m_vertex;

                // ЗАДАЧА: переместить метод next_vertex() из Iterator в Vertex.
            }

        private:
            // Почему мы вообще можем тут инициализировать поля?
            char m_s{};
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

            Iterator(const Trie<T>* _trie, value_type& x) // : m_trie{_trie} 
            { 
                
            }

            Iterator(const Iterator& other) = default;
            Iterator& operator=(const Iterator& other) = default;
            
            Iterator(Iterator&& other) = default;
            Iterator& operator=(Iterator&& other) = default;

            Iterator& operator++()
            {
                while ((m_vertex != nullptr) && (!m_vertex->end_of_word()))
                    m_vertex = next_vertex();
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

            std::shared_ptr<Vertex<T>> next_vertex()
            {
                /*
                Этот метод находит и возвращает следующую по порядку вершину дерева, не зависимо
                от того, является она концом ключа или нет. Если метод дойдет до конца дерева, так
                и не найдя вершины, то он вернет nullptr
                */

                /*
                Какие могут быть случаи:
                У текущей вершины есть непройденные дочерние вершины:
                    1) Идём к первой непройденной вершине в массиве дочерних
                Нет непройденных дочерних вершин:
                    2) Есть родительская вершина - идем к ней и от неё в следующую дочернюю
                    3) Нет родительской - значит мы в корне дерева, то есть обход завершен.

                Условие для прекращение работы алгоритма:
                Мы дошли до корневой вершины, и у неё все дочерние были пройдены.

                Шаги следующие.
                1) берем первую непройденную верщину из дочерних, если таковая имеется
                2) Если таких нет, то возвращаемся к родительской и выполняем пункт 1)
                уже для родительской вершины. Если вдруг родительской нет, это значит 
                что мы дошли до конца дерева
                */

                char current_symbol = 0;
                std::shared_ptr<Vertex<T>> next_vertex{};
                std::shared_ptr<Vertex<T>> checking_vertex = m_vertex;

                bool finish = false;
                while (!finish)
                {
                    for (auto& [symbol, v] : checking_vertex->children())
                    {
                        if (symbol > current_symbol)
                        {
                            next_vertex = v;
                            break;
                        }
                    }

                    if (next_vertex != nullptr)
                    {
                        // Нашли непройденную вершину
                        finish = true;
                    }
                    else // Дочерних вершин нет
                    {
                        // Если есть родительская, то перемещаемся к ней
                        if (checking_vertex->parent() != nullptr)
                        {
                            current_symbol = checking_vertex->symbol();
                            checking_vertex = checking_vertex->parent();
                        }
                        else 
                        {
                            // Тут алгоритм заканчивает работу
                            finish = true;
                        }
                    }
                }

                return next_vertex;
            }
        };


        template <typename T>
        // class ConstIterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
        class ConstIterator
        {

        };
    
    }
}
