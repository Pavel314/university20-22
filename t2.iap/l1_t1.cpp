#include <cmath>
#include <iostream>
#include <string>
double calculate(double x, double y)
{
    if(x - y > 0) {
        return std::pow(x - y, 1.5) + std::tan(x);
    } else if(x - y < 0) {
        return std::pow(y - x, 2) + std::cos(x);
    } else
        return x * x + std::pow(y, 1.5) + std::sin(x);
}

template<typename NumT>
NumT request_num(const std::string& text)
{
    NumT res;
    while(true) {
        std::cout << text << std::flush;
        std::cin >> res;
        if(std::cin.good())
            return res;

        std::cin.clear();
        std::string dummy;
        std::cin >> dummy;
    }
}

int main(int argc, char** argv)
{
    auto x = request_num<double>("Input x:\n");
    auto y = request_num<double>("Input y:\n");
    std::cout<<"Result: "<<calculate(x,y)<<"\n";
    return 0;
}
