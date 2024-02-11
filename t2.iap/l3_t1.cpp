#include <cstdint>
#include <iostream>
#include <limits>

enum class conv_result { Ok, UnkownSymbol, Overflow };

template <int base, typename NumT, typename LenT> conv_result conv_t10(const char* s, LenT len, NumT& result)
{
    static_assert(base >= 1 && base <= 16, "Base should be in 1..16");
    constexpr static int dif = 'a' - 'A';
    result = 0;
    NumT pow = 1;
    --len; // null terminator
    while(len--) {
        char d = s[len];
        int dig = 0;
        if(d >= '0' && d <= '9') {
            dig = d - '0';
        } else if(d >= 'a' && d <= 'f') {
            dig = d - 'a' + 10;
        } else if(d >= 'A' && d <= 'F') {
            dig = d - 'A' + 10;
        } else if(len == 0 && d == '-') {
            result *= -1;
            return conv_result::Ok;
        } else if(len == 0 && d == '+') {
            return conv_result::Ok;
        } else
            return conv_result::UnkownSymbol;

        auto dmp = result;
        result += pow * dig;
        if(result < dmp)
            return conv_result::Overflow;

        pow *= base;
    }
    return conv_result::Ok;
}

template <int base, typename NumT> void conv_f10(NumT num, char* result)
{
    static_assert(base >= 1 && base <= 16, "Base should be in 1..16");
    constexpr static char alphabet[] { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    if(num < 0) {
        *result++ = '-';
        num *= -1;
    }
    char* beg_add = result;
    char* end_add = beg_add;
    int iters=0;
    do {
        *end_add++ = alphabet[num % base];
        num /= base;
        ++iters;
    } while(num != 0);
    *end_add-- = '\0';
    
    iters/=2;
    while(iters--) {
        std::swap(*beg_add++, *end_add--);
    }
}

int main()
{
    using num_t = std::int_least64_t;
    // constexpr std::size_t len = sizeof(num_t) * 8 / 3;
    char buf[64]; // len + 1
    while(1) {
        std::cout << "Enter an integer in hexadecimal notation\n";
        std::cin.get(buf, sizeof(buf));
        if(std::cin.peek() != '\n') {
            if(std::cin.peek() != std::char_traits<char>::eof())
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            else {
                std::cin.clear();
                std::cin.get();
            }
        } else {
            std::cin.get();
            std::size_t alen = 0;
            while(buf[alen++] != '\0') {
            }
            num_t res;
            conv_result cres = conv_t10<16>(buf, alen, res);
            switch(cres) {
            case conv_result::Ok: {
                std::cout<<"Base10="<<res<<'\n';
                conv_f10<8>(res, buf);
                std::cout <<"Base8="<<buf << "\n\n";
            } break;
            case conv_result::Overflow:
                std::cout << "Overflow\n\n";
                break;
            case conv_result::UnkownSymbol:
                std::cout << "Unkown symbol\n\n";
                break;
            }
        }
    }
}