#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using pos_type = std::int_fast32_t;

struct segment {
    pos_type min, max;
};

template <typename InputIt, typename Callback> void solve_problem(InputIt first, InputIt last, Callback callback)
{
    if (first==last) return;
    segment border { *first };
    for(++first; first != last; ++first) {
        const segment& current = *first;
        if(current.min >= border.min && current.min <= border.max) {
            border.min = current.min;
            if(current.max < border.max)
                border.max = current.max;
        } else {
            callback(border.min);
            border = current;
        }
    }
    callback(border.min);
}

int main()
{
    std::size_t count;
    std::cin >> count;

    std::vector<segment> segs(count);
    std::for_each(segs.begin(),segs.end(),[](segment& seg){std::cin >> seg.min >> seg.max;});

    std::sort(segs.begin(), segs.end(), [](const segment& s1, const segment& s2) { return s1.min < s2.min; });
    
    std::vector<pos_type> result;
    solve_problem(segs.cbegin(), segs.cend(), [&result](pos_type pos) { result.push_back(pos); });
    
    std::cout << result.size() << std::endl;
    std::copy(result.begin(), result.end(), std::ostream_iterator<pos_type>(std::cout, " "));
    return 0;
}