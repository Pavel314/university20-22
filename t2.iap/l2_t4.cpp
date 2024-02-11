#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

template <typename NumT> NumT request_num(std::string text)
{
    NumT res;
    while(true) {
        std::cout << text << std::flush;
        std::cin >> res;
        if(!std::cin.good()) {
            std::cin.clear();
            std::string dummy;
            std::cin >> dummy;
        } else
            return res;
    }
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

template <typename NumT, typename RetFunc> void generate_spiral(NumT sx, NumT sy, const RetFunc& ret_func)
{
    const NumT size = sx * sy;
    NumT pos[2] = { 0, 0 };
    NumT borders[4] = { sx - 1, sy - 1, 0, 1 };
    NumT* cur_inc = &pos[0];
    int ind = 0;
    int sign = 1;
    for(NumT i = 0; i < size; i++) {
        ret_func(pos[0], pos[1], i);
        if((sign == 1 && (*cur_inc >= borders[ind])) || (sign == -1 && (*cur_inc <= borders[ind]))) {
            borders[ind] -= sign;
            ind = ind < 3 ? ind + 1 : 0;
            cur_inc = &pos[ind & 1];
            sign = ((ind + 2) & 2) - 1;
        }
        *cur_inc += sign;
    }
}

int main()
{
    using data_t = std::vector<int>;
    using ind_t = data_t::size_type;
    using num_t = data_t::value_type;

    const ind_t cx = request_num<std::size_t>("Enter the width of the matrix\n");
    const ind_t cy = request_num<std::size_t>("Enter the height of the matrix\n");
    std::vector<num_t> data(cx * cy);
    std::vector<num_t> spiral(data.size());
    visit_matrix<ind_t>(0, cx, 0, cy, [&data, &cx](ind_t x, ind_t y, ind_t i) {
        data[i] = request_num<num_t>("Input value " + std::to_string(x) + ", " + std::to_string(y) + "\n");
        if(x == cx - 1) {
            std::cout << std::endl;
        }
    });
    std::sort(data.begin(), data.end());
    generate_spiral(cx, cy, [&](ind_t x, ind_t y, ind_t ind) { spiral[y * cx + x] = data[ind]; });
    visit_matrix<ind_t>(0, cx, 0, cy, [&](ind_t x, ind_t y, ind_t i) {
        std::cout << std::right << std::setw(3) << spiral[i];
        if(x < cx - 1)
            std::cout << ' ';
        else
            std::cout << std::endl;
    });
    return 0;
}