#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

namespace _disjoint_set_implementation_details
{
template <typename Ind, typename Item> struct node {
    Ind parent;
    Ind rank;
    Item item;
    node(Ind parent_, Ind rank_, Item item_)
        : parent(parent_)
        , rank(rank_)
        , item(item_)
    {
    }
};
}

template <typename Element,
    typename Ind = std::size_t,
    class MapContainer = std::unordered_map<Element, Ind>,
    class TreeContainer = std::vector<_disjoint_set_implementation_details::node<Ind, const Element*>>>
class disjoint_set
{
public:
    using map_container = MapContainer;
    using tree_container = TreeContainer;
    using value_type = Element;
    using reference = Element&;
    using const_reference = const Element&;
    using ind_type = Ind;

protected:
    template <bool IsConstIt> class internal_iterator
    {
        friend disjoint_set;

    private:
        using base_iterator = typename map_container::iterator;
        base_iterator it;
        static inline typename std::iterator_traits<base_iterator>::value_type::first_type& reflect_func(
            base_iterator it)
        {
            return it->first;
        }

    public:
        using reference = decltype(reflect_func(it));
        using pointer = std::remove_reference<reference>*;
        using value_type = std::remove_reference<reference>;
        using iterator_category = std::forward_iterator_tag;

    private:
        using mbc_reference = typename std::conditional<IsConstIt, const reference, reference>::type;
        using mbc_pointer = typename std::conditional<IsConstIt, const pointer, pointer>::type;

        internal_iterator(const base_iterator it_)
            : it(it_)
        {
        }

    public:
        internal_iterator()
        {
        }

        internal_iterator(const internal_iterator<false>& other)
            : it(other.it)
        {
        }

        internal_iterator& operator=(const internal_iterator& other)
        {
            it = other.it;
            return *this;
        }

        internal_iterator& operator++()
        {
            ++it;
            return *this;
        }

        internal_iterator operator++(int)
        {
            iterator tmp(*this);
            operator++();
            return tmp;
        }

        const reference operator*() const
        {
            return reflect_func(it);
        }

        mbc_reference operator*()
        {
            return reflect_func(it);
        }

        const pointer operator->() const
        {
            return &reflect_func(it);
        }
        mbc_pointer operator->()
        {
            return &reflect_func(it);
        }

        bool operator!=(const internal_iterator& other) const
        {
            return it != other.it;
        }

        bool operator==(const internal_iterator& other) const
        {
            return it == other.it;
        }
    };

    mutable tree_container tree;
    mutable map_container map;
    ind_type _sets_count;
    using map_iterator = typename map_container::iterator;
    ind_type collapse(ind_type ind) const
    {
        ind_type root = ind;
        while(tree[root].parent != root) {
            root = tree[root].parent;
        }
        while(tree[ind].parent != ind) {
            ind_type oind = ind;
            ind = tree[ind].parent;
            tree[oind].parent = root;
        }
        return root;
    }

public:
    using iterator = internal_iterator<false>;
    using const_iterator = internal_iterator<true>;

protected:
    inline iterator create_it(ind_type ind)
    {
        return iterator(map.find(*tree[ind].item));
    }

    template <bool find_mode, typename VT> iterator _make_set(VT&& item)
    {
        static_assert(std::is_convertible<VT, value_type>::value, "VT should be convertible to value_type");

        const ind_type size = tree.size();
        auto emplace_result = map.emplace(std::forward<VT>(item), size);
        if(!emplace_result.second) {
            if(find_mode)
                return find_set(emplace_result.first);
            return end();
        }
        tree.emplace_back(size, 0, &(emplace_result.first->first));
        ++_sets_count;
        return emplace_result.first;
    }

public:
    disjoint_set()
        : tree()
        , map()
        , _sets_count(0)
    {
    }
    disjoint_set(const disjoint_set& other)
        : tree(other.tree)
        , map(other.map)
        , _sets_count(other.sets_count())
    {
        for(auto it = map.begin(); it != map.end(); ++it) {
            tree[it->second].item = &it->first;
        }
    }

    disjoint_set& operator=(disjoint_set other)
    {
        swap(*this, other);
        return *this;
    }

    friend void swap(disjoint_set& lhs, disjoint_set& rhs)
    {
        lhs.tree.swap(rhs.tree);
        lhs.map.swap(rhs.map);
        std::swap(lhs._sets_count, rhs._sets_count);
    }

    inline void reserve(ind_type size)
    {
        tree.reserve(size);
        map.reserve(size);
    }

    inline iterator begin()
    {
        return map.begin();
    }
    inline iterator end()
    {
        return map.end();
    }
    inline const_iterator cbegin() const
    {
        return map.begin();
    }
    inline const_iterator cend() const
    {
        return map.end();
    }

    template <typename VT> inline iterator make_set(VT&& item)
    {
        return _make_set<false>(std::forward<VT>(item));
    }

    template <typename VT> inline iterator find_or_make_set(VT&& item)
    {
        return _make_set<true>(std::forward<VT>(item));
    }

    inline iterator find_set(const_iterator item)
    {
        return create_it(collapse(tree[item.it->second].parent));
    }
    inline const_iterator find_set(const_iterator item) const
    {
        return map.find(*tree[collapse(tree[item.it->second].parent)].item);
    }

    template <typename VT, typename std::enable_if<std::is_convertible<VT, value_type>::value, bool>::type = true>
    iterator find_set(VT&& item)
    {
        const iterator it(map.find(std::forward<VT>(item)));
        if(it != end())
            return find_set(it);
        return it;
    }
    template <typename VT, typename std::enable_if<std::is_convertible<VT, value_type>::value, bool>::type = true>
    const_iterator find_set(VT&& item) const
    {
        const const_iterator it(map.find(std::forward<VT>(item)));
        if(it != cend())
            return find_set(it);
        return it;
    }

    inline void swap_values(const_iterator lhs, const_iterator rhs)
    {
        std::swap(tree[lhs.it->second].item, tree[rhs.it->second].item);
        std::swap(lhs.it->second, rhs.it->second);
    }

    enum class agent_kind { same_set, left_set, right_set };
    std::pair<agent_kind, iterator> link(const_iterator lhs, const_iterator rhs)
    {
        const ind_type lind = collapse(tree[lhs.it->second].parent);
        const ind_type rind = collapse(tree[rhs.it->second].parent);
        if(lind == rind)
            return std::make_pair(agent_kind::same_set, create_it(lind));
        --_sets_count;
        if(tree[lind].rank > tree[rind].rank) {
            tree[rind].parent = lind;
            return std::make_pair(agent_kind::left_set, create_it(lind));
        } else {
            tree[lind].parent = rind;
            if(tree[lind].rank == tree[rind].rank)
                ++tree[rind].rank;
            return std::make_pair(agent_kind::right_set, create_it(rind));
        }
    }

    inline void left_link(const_iterator agent, const_iterator item)
    {
        auto pair = link(agent, item);
        if(const_iterator(pair.second) != agent) {
            swap_values(agent, pair.second);
        }
    }

    inline void right_link(const_iterator item, const_iterator agent)
    {
        left_link(agent, item);
    }

    inline ind_type sets_count() const
    {
        return _sets_count;
    }
    inline ind_type size() const
    {
        return (ind_type)tree.size();
    }
    inline void clear()
    {
        map.clear();
        tree.clear();
        _sets_count = 0;
    }
};

int main()
{
    using num_t = std::uint_fast32_t;

    num_t table_size, query_count;
    std::cin >> table_size >> query_count;

    std::vector<num_t> tables(table_size);
    disjoint_set<num_t> disjoint;
    disjoint.reserve(table_size);

    num_t max_sz=0;
    for(num_t& sz:tables){
        std::cin>>sz;
        if (sz>max_sz) max_sz=sz;
    }
    
    while(query_count--){
        num_t dest,src;
        std::cin>>dest>>src;
        --dest;--src;
        auto dit=disjoint.find_or_make_set(dest);
        auto sit=disjoint.find_or_make_set(src);
        if (dit!=sit){
            tables[*dit]+=tables[*sit];
            disjoint.left_link(dit,sit);
            if (tables[*dit]>max_sz) max_sz=tables[*dit];
        }
        std::cout<<max_sz<<std::endl;
    }
}