#include <iostream>
#include <cstdint>
#include <vector>

using m_luint=std::uint_fast64_t;
using m_uint=std::uint_fast32_t;

std::vector<m_uint> get_pisano(const m_uint n)
{
    m_uint cur = 0, next = 1;
    if (n <= 1) return std::vector<m_uint>{ cur };
    std::vector<m_uint> res;
    do {
        res.push_back(cur);
        const auto prev = next;
        next = cur + next;
        if (next >= n) next -= n;
        cur = prev;
    } while (cur != 0 || next != 1);
    return res;
}

m_uint get_fib_mod(const m_luint n, const m_uint m){
    const std::vector<m_uint> seq(get_pisano(m));
    return seq[n%seq.size()];
}

int main()
{
    m_luint m;
    m_uint n;
    std::cin>>m>>n;
    std::cout<<get_fib_mod(m,n);
}