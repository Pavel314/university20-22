#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <stack>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <iterator>
#include <vector>

template <typename Elem, typename Container = std::stack<std::pair<Elem, const Elem*>>> class max_stack
{
public:
    using size_type = typename Container::size_type;
    using container_type = Container;
    using value_type = Elem;
    using reference = Elem&;
    using const_reference = const Elem&;

protected:
    Container container;
    template <typename Alloc>
    using uses_alloc = typename std::enable_if<std::uses_allocator<Container, Alloc>::value>::type;

public:
    template <typename Alloc, typename = uses_alloc<Alloc>>
    max_stack(const max_stack& src, const Alloc& alloc)
        : container(src.container, alloc)
    {
    }

    template <typename Alloc, typename = uses_alloc<Alloc>>
    max_stack(max_stack&& src, const Alloc& alloc)
        : container(std::move(src.container), alloc)
    {
    }

    template <typename = typename std::enable_if<std::is_default_constructible<Elem>::value>::type>
    max_stack()
        : container()
    {
    }

    bool empty() const
    {
        return container.empty();
    }

    size_type size() const
    {
        return container.size();
    }

    reference top()
    {
        return container.top().first;
    }
    const_reference top() const
    {
        return container.top().first;
    }

    const_reference max() const
    {
        return *container.top().second;
    }

    void push(const value_type& item)
    {
        if(!empty()) {
            const Elem* cur = &max();
            container.push(std::make_pair(item, nullptr));
            if(*cur < item)
                cur = &container.top().first;
            container.top().second = cur;
        } else {
            container.push(std::make_pair(item, nullptr));
            container.top().second = &container.top().first;
        }
    }

    void push(value_type&& item)
    {
        if(!empty()) {
            const Elem* cur = &max();
            container.push(std::make_pair(std::move(item), nullptr));
            const Elem& pushed = container.top().first;
            if(*cur < pushed)
                cur = &pushed;
            container.top().second = cur;
        } else {
            container.push(std::make_pair(std::move(item), nullptr));
            container.top().second = &container.top().first;
        }
    }

    void pop()
    {
        container.pop();
    }
};

template <typename Elem, typename PushContainer = std::deque<Elem>, typename PopContainer = max_stack<Elem>>
class max_queue
{
public:
    static_assert(std::is_same<typename PushContainer::size_type, typename PopContainer::size_type>::value,
        "size_type of both containers should be the same");
    using size_type = typename PushContainer::size_type;
    using push_container_type = PushContainer;
    using pop_container_type = PopContainer;
    using value_type = Elem;
    using reference = Elem&;
    using const_reference = const Elem&;

protected:
    PushContainer push_container;
    PopContainer pop_container;
    value_type* push_max = nullptr;

public:
    template <typename = typename std::enable_if<std::is_default_constructible<Elem>::value>::type>
    max_queue()
        : push_container()
        , pop_container()
    {
    }

    bool empty() const
    {
        return push_container.empty() && pop_container.empty();
    }

    size_type size() const
    {
        return push_container.size() + pop_container.size();
    }

    reference front()
    {
        return !pop_container.empty() ? pop_container.top() : push_container.front();
    }
    const_reference front() const
    {
        return !pop_container.empty() ? pop_container.top() : push_container.front();
    }

    const_reference max() const
    {
        const bool pop_container_empty = pop_container.empty();
        if(!push_container.empty() && !pop_container_empty)
            return std::max(*push_max, pop_container.max());
        return !pop_container_empty ? pop_container.max() : *push_max;
    }

    void push_back(const value_type& item)
    {
        const bool upd_max = push_container.empty() || *push_max < item;
        push_container.push_back(item);
        if(upd_max)
            push_max = &push_container.back();
    }

    void push_back(value_type&& item)
    {
        const bool upd_max = push_container.empty() || *push_max < item;
        push_container.push_back(std::move(item));
        if(upd_max)
            push_max = &push_container.back();
    }

    void pop_front()
    {
        if(pop_container.empty()) {
            while(!push_container.empty()) {
                pop_container.push(push_container.back());
                push_container.pop_back();
            }
        }
        pop_container.pop();
    }
};

template <typename InputIt, typename Size, typename Callback, typename MaxQueue=max_queue<typename std::iterator_traits<InputIt>::value_type>>
void maximum_in_sliding_window(InputIt first, Size size, typename MaxQueue::size_type win_size, const Callback callback)
{
    MaxQueue window;
    for(size -= win_size - 1; size != 0; ++first) {
        window.push_back(*first);
        if(window.size() == win_size) {
            callback(window.max());
            window.pop_front();
            --size;
        }
    }
}

using num_t = std::uint_least32_t;

int main()
{
    std::size_t count;
    std::cin>>count;
    std::vector<num_t> data(count);
    std::for_each(data.begin(),data.end(), [](num_t& num){std::cin>>num;});
    std::size_t win_size;
    std::cin>>win_size;
    maximum_in_sliding_window(data.begin(), data.size(), win_size, [](const num_t max) { std::cout << max<<' '; });
    return 0;
}