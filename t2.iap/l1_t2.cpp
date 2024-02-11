#include <cmath>
#include <iostream>
#include <string>

double ctg(double x)
{
    return std::cos(x) / std::sin(x);
};

double calculate(double x, int ind)
{
    switch(ind) {
    case 1:
        return std::sqrt(std::abs(ctg(x) * std::cos(x)) / 5.0);
    case 2:
        return (x * x - x) / std::log10(x * x);
    case 3:
        return (x - 5) / x * x * std::pow(2.71828182845904523536, x);
    case 4:
        return std::sqrt(std::abs(1 + 3 * std::pow(x, 2 / 3.0)));
    default:
        throw;
    }
}

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

int main(int argc, char** argv)
{
    auto x = request_num<double>("Input x:\n", [](double v) { return true; });
    auto ind = request_num<int>("Input function index(1-4):\n", [](int v) { return (v >= 1 && v <= 4); });
    std::cout << "Result: " << calculate(x, ind) << "\n";
    return 0;
}
