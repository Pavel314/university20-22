#include <iostream>
#include <cstdint>
#include <cmath>

using number_t=std::uint_fast32_t;

int main() {
    number_t n;
    std::cin>>n;
    //(n^2+n)/2 - известная закономерность для расчёта суммы первых n чисел 
    number_t terms=static_cast<number_t>(std::floor((std::sqrt(8.0*n+1)-1)*0.5));
    std::cout<<terms<<std::endl;
    
    for (number_t i=1;i<terms;++i){
        std::cout<<i<<' ';
    }
    --terms;
    std::cout<<n-(terms*terms+terms)/2;
    return 0;
}