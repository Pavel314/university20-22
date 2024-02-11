#include <algorithm>
#include <iostream>
#include <vector>
#include <cstddef>
#include <type_traits>
#include <iterator>

template <typename InputIt, class WightFunc>
void count_sort(InputIt first, InputIt last, WightFunc weight_func, std::size_t weight_difference)
{
    if(first == last)
        return;

    std::vector<typename std::iterator_traits<InputIt>::value_type> result((std::size_t)std::distance(first, last));
    std::vector<std::size_t> reps(weight_difference + 1);

    for(auto it = first; it != last; ++it) {
        ++reps[(std::size_t)weight_func(*it)];
    }
    std::size_t prev = reps.front();
    reps.front() = 0;
    for(auto it = reps.begin() + 1; it != reps.end(); ++it) {
        const std::size_t orig = *it;
        *it = prev + *(it - 1);
        prev = orig;
    }
    for(auto it = first; it != last; ++it) {
        result[reps[(std::size_t)weight_func(*it)]++] = std::move(*it);
    }
    std::move(result.cbegin(), result.cend(), first);
}

template <typename InputIt, typename = typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value>::type>
void count_sort(InputIt first, InputIt last)
{
    //const auto [min, max] = std::minmax_element(first, last);
    using val_t=typename std::iterator_traits<InputIt>::value_type;
    const auto min_max = std::minmax_element(first, last);
    count_sort(
        first, last, [&min_max](val_t item) { return static_cast<std::size_t>(item - *min_max.first); },
        static_cast<std::size_t>(*min_max.second - *min_max.first));
}

using number_t=int;
int main() {
    std::size_t count;
    std::cin>>count;
    std::vector<number_t> data(count);
    std::for_each(data.begin(),data.end(),[](number_t& elem){std::cin>>elem;});
    count_sort(data.begin(),data.end());
    std::copy(data.begin(),data.end(), std::ostream_iterator<number_t>(std::cout, " "));
    return 0;
}