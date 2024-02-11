#include <cstddef>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>

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
        : container(src.container, alloc){}

    template <typename Alloc, typename = uses_alloc<Alloc>>
    max_stack(max_stack&& src, const Alloc& alloc)
        : container(std::move(src.container), alloc){}

    template <typename = typename std::enable_if<std::is_default_constructible<Elem>::value>::type>
    max_stack() : container(){}

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
        return container.back().first;
    }
    const_reference top() const
    {
        return container.back().first;
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

using num_t = std::uint_least32_t;

int main()
{
    num_t count;
    std::cin >> count;
    max_stack<num_t> stack;
    std::string str;
    while(count--) {
        std::cin >> str;
        if(str == "push") {
            num_t num; std::cin >> num;
            stack.push(num);
        } else if(str == "pop")
            stack.pop();
        else if(str == "max")
            std::cout << stack.max()<<std::endl;
    }
    return 0;
}