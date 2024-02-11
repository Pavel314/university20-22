#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

template <typename NumT> NumT request_num(const std::string& str)
{
    NumT res;
    while(true) {
        std::cout << str << std::flush;
        std::cin >> res;
        if(!std::cin.good()) {
            std::cin.clear();
            std::string dummy;
            std::cin >> dummy;
        } else
            return res;
    }
}

int main()
{
    std::size_t n = request_num<std::size_t>("Enter the size of the array\n");
    std::vector<double> data(n);
    double min = 0.0;
    bool min_inited = false;

    for(decltype(data)::size_type i = 0; i < data.size(); ++i) {
        double num = request_num<double>("Input value " + std::to_string(i + 1) + ": ");
        data[i] = num;
        if(num > 0) {
            if(min_inited) {
                if(num < min)
                    min = num;
            } else {
                min = num;
                min_inited = true;
            }
        }
    }
    if(min_inited) {
        for(auto& num : data) {
            num = num < 0 ? min : num;
            std::cout << num << ' ';
        }
    } else
        std::cout << "Could not find minimum positive number";
}