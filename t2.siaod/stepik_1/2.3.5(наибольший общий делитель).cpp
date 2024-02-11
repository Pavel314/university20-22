#include <iostream>
#include <cstdint>

template <typename TyNum>
TyNum gcd(TyNum a, TyNum b)
{
    while(true) {
        if(a > b) {
            if(b == 0)
                return a;
            a %= b;
        } else {
            if(a == 0)
                return b;
            b %= a;
        }
    }
}

int main() {
  std::uint_fast32_t a, b;
  std::cin >> a >> b;
  std::cout << gcd(a, b);
  return 0;
}