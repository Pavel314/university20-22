#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace _merge_sort_implementation_details
{
template <bool count_inversions> struct inversions {
    using count_t=std::size_t;
    static count_t count;
};
template <> struct inversions<false> {};

template<> inversions<true>::count_t inversions<true>::count=0;
    
#pragma push_macro("if_constexpr")
#ifdef if_constexpr
#undef if_constexpr
#endif
#if __cplusplus>=201703L
#define if_constexpr if constexpr
#else
#define if_constexpr if
#endif

template <bool count_inversions> struct impl {
    template <typename LeftIt, typename RightIt, typename BufferIt>
    static void merge(LeftIt left, LeftIt left_last, RightIt right, RightIt right_last, BufferIt buffer)
    {
        for(; left != left_last && right != right_last; ++buffer) {
            const typename std::iterator_traits<LeftIt>::value_type& l_val = *left;
            const typename std::iterator_traits<RightIt>::value_type& r_val = *right;
            if(r_val < l_val) {
                *buffer = r_val;
                ++right;
                if_constexpr(count_inversions)
                    inversions<true>::count += std::distance(left, left_last);
            } else {
                *buffer = l_val;
                ++left;
            }
        }
        std::copy(left, left_last, buffer);
        std::copy(right, right_last, buffer);
    }

    template <typename DataIt, typename BufIt>
    static void merge_step(DataIt data_it,
        DataIt data_end,
        BufIt buf_it,
        BufIt buf_end,
        const std::size_t length) // length should be power of two
    {
        const std::size_t half_length = length / 2;
        for(; buf_end - buf_it >= length; buf_it += length, data_it += length) {
            merge(data_it, data_it + half_length, data_it + half_length, data_it + length, buf_it);
        }
        if(buf_it != buf_end) {
            buf_it -= length;
            DataIt merging_it = data_it - length;
            merge(buf_it, buf_it + length, data_it, data_end, merging_it);
            std::copy(merging_it, data_end, buf_it);
        }
    }
    template <typename InputIt> static void merge_sort(InputIt first, InputIt last)
    {
        if_constexpr(count_inversions)
            inversions<true>::count = 0;
        std::vector<typename std::iterator_traits<InputIt>::value_type> buffer(std::distance(first, last));
        const std::size_t size = buffer.size();
        const auto buf_start = buffer.begin();
        const auto buf_end = buffer.end();
        std::size_t step = 1;

        while(step * 4 <= size) {
            merge_step(first, last, buf_start, buf_end, step * 2);
            merge_step(buf_start, buf_end, first, last, step * 4);
            step *= 4;
        }
        if(step * 2 <= size) {
            merge_step(first, last, buf_start, buf_end, step * 2);
            if(step * 2 == size)
                std::copy(buffer.cbegin(), buffer.cend(), first);
        }
    }
};
#undef if_constexpr
#pragma pop_macro("if_constexpr")
}

template <typename InputIt> void merge_sort(InputIt first, InputIt last)
{
    _merge_sort_implementation_details::impl<false>::merge_sort(first, last);
}

template <typename InputIt> _merge_sort_implementation_details::inversions<true>::count_t merge_sort_inversions(InputIt first, InputIt last)
{
    _merge_sort_implementation_details::impl<true>::merge_sort(first, last);
    return _merge_sort_implementation_details::inversions<true>::count;
}

using number_t=std::int_fast32_t;

int main() {
    std::size_t count;
    std::cin>>count;
    std::vector<number_t> data(count);
    std::for_each(data.begin(),data.end(),[](number_t& elem){std::cin>>elem;});
    std::cout<<merge_sort_inversions(data.begin(),data.end());
    return 0;
}