#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <algorithm>
#include <utility>

template <typename InputIt, typename Callback>
void build_heap(const InputIt first, const InputIt last, const Callback callback)
{
    InputIt it = first + (std::distance(first, last) + 1) / 2 - 1;
    for(; it + 1 != first; --it) {
        InputIt ind = it;
        InputIt child = ind + std::distance(first, ind) + 1;
        while(child < last) {
            if((child + 1) < last && *(child + 1) < *child)
                ++child;
            if(*child < *ind){}else break;
             callback(ind, child);
            std::swap(*ind,*child);
            ind = child;
            child = ind + std::distance(first, ind) + 1;
        }
    }
}    

using num_t=std::uint_least32_t;

int main() {
    std::size_t count;
    std::cin>>count;
    std::vector<num_t> data(count);
    std::for_each(data.begin(),data.end(),[](num_t& num){std::cin>>num;});
    using it_t=decltype(data)::iterator;
    std::vector<std::pair<std::iterator_traits<it_t>::value_type,std::iterator_traits<it_t>::value_type>> swaps;
    const it_t beg=data.begin();
    build_heap(beg,data.end(),[&swaps,&beg](it_t a, it_t b){
        swaps.push_back(std::make_pair(std::distance(beg,a),std::distance(beg,b)));
    });
    std::cout<<swaps.size()<<std::endl;
    for(const auto& item: swaps){
        std::cout<<item.first<<' '<<item.second<<std::endl;
    }
    return 0;
}