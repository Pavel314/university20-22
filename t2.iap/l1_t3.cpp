#include <cmath>
#include <iostream>
#include <string>

double calculate(double x, double eps)
{
    double denom = 1.0;
    double numer = x;
    double sum = 0.0;
    int sign = 1;
    while(true) {
        double rat = numer / denom;
        if(std::abs(rat) <= eps)
            break;
        sum += rat * sign;
        sign *= (-1);
        numer *= x * x;
        denom += 2;
    }
    return sum;
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
    auto x =  request_num<double>("Input x(x should be in -1..1):\n", [](double x) { return x > -1 && x < 1; });
    auto eps =  request_num < double>(
                         "Input epsilon(eps should be greate or eql 0)\n", [](double x) { return x >= 0; });
    std::cout << "Model value=" << std::atan(x) << "\nCalculated value=" << calculate(x, eps) << "\n";
    return 0;
}
