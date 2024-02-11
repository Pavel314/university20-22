#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename NumT, typename RestrictFun> NumT request_num(std::string text, const RestrictFun& fun)
{
    NumT res;
    while(true) {
        std::cout << text << std::flush;
        std::cin >> res;
        if(!std::cin.good()) {
            std::cin.clear();
            std::string dummy;
            std::cin >> dummy;
        } else if(fun(res))
            return res;
    }
}
template <typename NumT> NumT request_num(std::string text)
{
    return request_num<NumT>(text, [](NumT n) { return true; });
}

int main()
{
    std::size_t n = request_num<std::size_t>("Enter the size of the array\n");
    std::vector<int> data(n);
    using ind_t = decltype(data)::size_type;

    ind_t series_count = 0;

    for(ind_t i = 0; i < data.size(); ++i) {
        data[i] = request_num<int>("Input value " + std::to_string(i) + ": ");
        if(i == 0 || data[i] != data[i - 1])
            ++series_count;
    }
    std::cout << "\nSeries count=" << series_count << std::endl;

    ind_t fst_start = request_num<ind_t>(
        "Enter the first index of the series(The index should not be more than the number of series)\n",
        [series_count](ind_t v) { return v < series_count; });
    ind_t snd_start = request_num<ind_t>(
        "Enter the second index of the series(The index should not be more than the number of series)\n",
        [series_count](ind_t v) { return v < series_count; });

    if(fst_start != snd_start) {
        if(fst_start > snd_start)
            std::swap(fst_start, snd_start);

        ind_t si = 0;
        ind_t fst_ind = 0;
        ind_t snd_ind = 0;
        for(ind_t i = 1; i < data.size(); ++i) {
            if(data[i] != data[i - 1]) {
                ++si;
                if(si == fst_start) {
                    fst_ind = i;
                } else if(si == snd_start) {
                    snd_ind = i;
                    break;
                }
            }
        }
        ind_t fprev = data[fst_ind];
        ind_t sprev = data[snd_ind];
        for(ind_t i = fst_ind; i < data.size(); ++i) {
            std::swap(data[fst_ind++], data[snd_ind++]);
            if(data[fst_ind] != fprev || data[snd_ind] != sprev)
                break;
            fprev = fst_ind;
            sprev = snd_ind;
        }
    }

    for(auto num : data) {
        std::cout << num << ' ';
    }
}
/*13

1
2
3
4
4
5
5
6
6
6
6
7
8

0
1*/