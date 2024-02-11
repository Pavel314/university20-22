#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <vector>

namespace local{
template <typename InputIt, typename ElTy> InputIt binary_search(InputIt first, InputIt last, const ElTy& target)
{
    typename std::iterator_traits<InputIt>::difference_type count = std::distance(first, last);
    InputIt temp_it;
    while(count > 0) {
        temp_it = first;
        const bool odd = count & 1;
        count /= 2;
        std::advance(temp_it, count - !odd);
        if(target > *temp_it) {
            first = ++temp_it;
        } else if(target == *temp_it) {
            return temp_it;
        }
    }
    return last;
}}

using number_t = std::int_fast32_t;
int main()
{
    std::size_t count;
    std::cin >> count;
    std::vector<number_t> numbers(count);
    std::for_each(numbers.begin(), numbers.end(), [](number_t& num) { std::cin >> num; });

    std::size_t search_count;
    std::cin >> search_count;

    while(search_count--) {
        number_t val;
        std::cin >> val;
        auto it = local::binary_search(numbers.cbegin(), numbers.cend(), val);
        if(it != numbers.cend())
            std::cout << std::distance(numbers.cbegin(),it)+1 << ' ';
        else
            std::cout << -1 << ' ';
    }
    
}