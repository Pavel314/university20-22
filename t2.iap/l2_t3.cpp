#include <cstddef>
#include <iostream>
#include <limits>
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

template <typename NumT, typename Func> void visit_matrix(NumT sx, NumT ex, NumT sy, NumT ey, const Func& func)
{
    NumT y = sy;
    NumT x = sx;
    while(1) {
        if(x == ex) {
            x = 0;
            ++y;
        }
        if(y == ey)
            break;
        func(x, y, ex * y + x);
        ++x;
    }
}

int main()
{
    using data_t = std::vector<int>;
    using ind_t = data_t::size_type;
    using num_t = int;
    
    ind_t n = request_num<std::size_t>("Enter the size of the square matrix\n");
    ind_t cx = n;
    ind_t cy = n + 1;
    data_t data(cx * cy);

    visit_matrix<ind_t>(0, cx, 0, cy, [&](ind_t x, ind_t y, ind_t i) {
        if(y > 0) {
            data[cx * x + y + cx - 1] =
                request_num<num_t>("Input value " + std::to_string(x) + ", " + std::to_string(y) + "\n");
        } else
            data[i] = 0;
    });
    std::cout << "Input matrix:\n";
    num_t ms = std::numeric_limits<num_t>::max(), cms = 0;
    ind_t mi = 0;
    visit_matrix<ind_t>(0, cx, 1, cy, [&](ind_t x, ind_t y, ind_t i) {
        std::cout << data[cx * x + y + cx - 1] << ' ';
        cms += data[i];
        if(x == cx - 1) {
            std::cout << std::endl;
            if(cms < ms) {
                ms = cms;
                mi = y;
            }
            cms = 0;
        }
    });
    std::cout << "Minimal sum=" << ms << ", column ind=" << mi - 1 << std::endl;

    visit_matrix<ind_t>(0, cx, 1, mi, [&](ind_t x, ind_t y, ind_t i) { std::swap(data[i - cx], data[i]); });
    visit_matrix<ind_t>(0, cy, 0, cx, [&](ind_t x, ind_t y, ind_t i) {
        std::cout << data[cx * x + y] << ' ';
        if(x == cy - 1) {
            std::cout << std::endl;
        }
    });
}
/*
1 1 1 6 2 8
9 6 7 7 4 7
8 9 0 8 5 5
0 4 6 6 1 0
5 8 5 3 6 1
3 0 4 2 9 6
=>
1 1 0 1 6 2 8
9 6 0 7 7 4 7
8 9 0 0 8 5 5
0 4 0 5 5 1 0
5 8 0 5 3 6 1
3 0 0 4 2 9 6
*/