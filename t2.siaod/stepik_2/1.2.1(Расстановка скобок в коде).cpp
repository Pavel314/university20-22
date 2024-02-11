#include <iostream>
#include <iterator>
#include <stack>
#include <string>

int get_bracket_type(const char symbol)
{
    switch(symbol) {
    case '(':
        return 1;
    case '[':
        return 2;
    case '{':
        return 3;
    case ')':
        return -1;
    case ']':
        return -2;
    case '}':
        return -3;
    default:
        return 0;
    }
}

template <typename InputIt> InputIt check_brackets(InputIt first, InputIt last)
{
    std::stack<int> stack;
    InputIt first_open;
    for(auto it = first; it != last; ++it) {
        const int bk = get_bracket_type(*it);
        if(bk > 0) {
            if(stack.empty())
                first_open = it;
            stack.push(bk);
        } else if(bk < 0) {
            if(!stack.empty() && stack.top() == -bk) {
                stack.pop();
            } else
                return it;
        }
    }
    if(!stack.empty())
        return first_open;

    return last;
}

int main()
{
    const std::string input(std::istreambuf_iterator<char>(std::cin),std::istreambuf_iterator<char>());
    auto end=input.cend();
    auto it = check_brackets(input.cbegin(), end);
    if(it != end)
        std::cout << std::distance(input.cbegin(), it) + 1;
    else
        std::cout << "Success";
    return 0;
}