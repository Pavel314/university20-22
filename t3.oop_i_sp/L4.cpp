#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ostream>
#include <ratio>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
//---
#include <cctype>
#include <cstddef>
#include <cstdlib> //(std::exit)
#include <iomanip>
#include <iostream>
#include <istream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
namespace std_ext
{
//------------------------------
//-ALLOCATOR EXTENSION SECTION--
//------------------------------
template <class Alloc>
constexpr void destroy(Alloc& alloc, typename std::allocator_traits<Alloc>::pointer first, const decltype(first) last)
{
    for(; first != last; ++first) {
        std::allocator_traits<Alloc>::destroy(alloc, first);
    }
}

template <class Alloc, class InputIt>
constexpr void initialized_rewrite(Alloc& alloc,
    InputIt first,
    InputIt last,
    const typename std::allocator_traits<Alloc>::pointer d_first)
{
    auto current = d_first;
    try {
        for(; first != last; ++first, (void)++current) {
            std::allocator_traits<Alloc>::destroy(alloc, current);
            std::allocator_traits<Alloc>::construct(alloc, current, *first);
        }
    } catch(...) {
        destroy(alloc, d_first, current);
        throw;
    }
}

template <class Alloc, class InputIt, class Size>
constexpr void initialized_first_destroy_and_copy(Alloc& alloc,
    InputIt first,
    InputIt last,
    Size destroy_count,
    const typename std::allocator_traits<Alloc>::pointer d_first)
{
    auto current = d_first;
    try {
        for(; first != last; ++first, (void)++current) {
            if(destroy_count > 0) {
                std::allocator_traits<Alloc>::destroy(alloc, current);
                --destroy_count;
            }
            std::allocator_traits<Alloc>::construct(alloc, current, *first);
        }
    } catch(...) {
        destroy(alloc, d_first, current);
        throw;
    }
}

template <class Alloc>
constexpr void uninitialized_destroy_move(Alloc& alloc,
    typename std::allocator_traits<Alloc>::pointer first,
    decltype(first) last,
    const decltype(first) d_first)
{
    auto current = d_first;
    try {
        for(; first != last; ++first, (void)++current) {
            std::allocator_traits<Alloc>::construct(alloc, current, std::move(*first));
            std::allocator_traits<Alloc>::destroy(alloc, first);
        }
    } catch(...) {
        destroy(alloc, d_first, current);
        throw;
    }
}

template <class Alloc, class InputIt>
constexpr void uninitialized_copy(Alloc& alloc,
    InputIt first,
    InputIt last,
    const typename std::allocator_traits<Alloc>::pointer d_first)
{
    auto current = d_first;
    try {
        for(; first != last; ++first, (void)++current) {
            std::allocator_traits<Alloc>::construct(alloc, current, *first);
        }
    } catch(...) {
        destroy(alloc, d_first, current);
        throw;
    }
}

template <class Alloc>
constexpr void uninitialized_default_construct(Alloc& alloc,
    const typename std::allocator_traits<Alloc>::pointer first,
    const decltype(first) last)
{
    auto current = first;
    try {
        for(; current != last; ++current) {
            std::allocator_traits<Alloc>::construct(alloc, current);
        }
    } catch(...) {
        destroy(alloc, first, current);
        throw;
    }
}

template <class Alloc>
constexpr void uninitialized_fill(Alloc& alloc,
    const typename std::allocator_traits<Alloc>::pointer first,
    const decltype(first) last,
    const typename std::allocator_traits<Alloc>::value_type& value)
{
    auto current = first;
    try {
        for(; current != last; ++current) {
            std::allocator_traits<Alloc>::construct(alloc, current, value);
        }
    } catch(...) {
        destroy(alloc, first, current);
        throw;
    }
}

template <class Alloc, class size_type, class LInputIt, class RInputIt, class BinaryOp>
constexpr typename std::allocator_traits<Alloc>::pointer uninitialized_transform_n(Alloc& alloc,
    const typename std::allocator_traits<Alloc>::pointer d_first,
    size_type n,
    LInputIt lfirst,
    RInputIt rfirst,
    BinaryOp binop)
{
    auto current = d_first;
    try {
        for(; n != 0; ++current, (void)--n) {
            std::allocator_traits<Alloc>::construct(alloc, current, binop(*lfirst++, *rfirst++));
        }
        return current;
    } catch(...) {
        destroy(alloc, d_first, current);
        throw;
    }
}

template <class Alloc>
constexpr void safe_deallocate(Alloc& alloc,
    typename std::allocator_traits<Alloc>::pointer buf,
    typename std::allocator_traits<Alloc>::size_type size)
{
    if(size > 0)
        std::allocator_traits<Alloc>::deallocate(alloc, buf, size);
}

template <class Alloc>
[[nodiscard]] constexpr typename std::allocator_traits<Alloc>::pointer safe_allocate(Alloc& alloc,
    typename std::allocator_traits<Alloc>::size_type size)
{
    if(size > 0)
        return std::allocator_traits<Alloc>::allocate(alloc, size);
    return nullptr;
}
//------------------------------
//-FUNCTIONAL EXTENSION SECTION-
//------------------------------
struct assign_add {
    template <typename LT, typename RT>
    constexpr auto operator()(LT& l, RT&& r) const noexcept(noexcept(l += std::forward<RT>(r)))
        -> decltype(l += std::forward<RT>(r))
    {
        return l += std::forward<RT>(r);
    }
};
struct assign_sub {
    template <typename LT, typename RT>
    constexpr auto operator()(LT& l, RT&& r) const noexcept(noexcept(l -= std::forward<RT>(r)))
        -> decltype(l -= std::forward<RT>(r))
    {
        return l -= std::forward<RT>(r);
    }
};
struct assign_mul {
    template <typename LT, typename RT>
    constexpr auto operator()(LT& l, RT&& r) const noexcept(noexcept(l *= std::forward<RT>(r)))
        -> decltype(l *= std::forward<RT>(r))
    {
        return l *= std::forward<RT>(r);
    }
};
struct assign_div {
    template <typename LT, typename RT>
    constexpr auto operator()(LT& l, RT&& r) const noexcept(noexcept(l /= std::forward<RT>(r)))
        -> decltype(l /= std::forward<RT>(r))
    {
        return l /= std::forward<RT>(r);
    }
};
//------------------------------
//-----OTHER METHOD SECTION-----
//------------------------------
template <class InputIt, class TString>
std::ostream& out_content(std::ostream& os, InputIt first, InputIt last, const TString& delim = ' ')
{
    if(first == last)
        return os;
    os << *first++;
    for(; first != last; ++first)
        os << delim << *first;
    return os;
    // using T = typename std::iterator_traits<InputIt>::value_type;
    // std::copy(first, --last, std::ostream_iterator<T>(os, delim));
}

template <class LInputIt, class RInputIt, class BinaryOp>
constexpr void binary_apply(LInputIt lfirst, LInputIt llast, RInputIt rfirst, BinaryOp binop)
{
    for(; lfirst != llast; ++lfirst, ++rfirst)
        binop(*lfirst, *rfirst);
}
}

namespace darray_ns
{
// Implementation of dynamic array. Created by PavelPI 08.12.2021
template <class Allocator, class Ratio = std::ratio<3, 2>> class darray
{
    static_assert(Ratio::num > 0 && Ratio::den > 0 && Ratio::num > Ratio::den);

public:
    using allocator_type = Allocator;

private:
    using alloc_tr = std::allocator_traits<allocator_type>;
    using pointer = typename alloc_tr::pointer;

public:
    using ratio = Ratio;
    using value_type = typename alloc_tr::value_type;
    using reference = value_type&;
    using rvalue_reference = value_type&&;
    using const_reference = const value_type&;
    using iterator = typename alloc_tr::pointer;
    using const_iterator = typename alloc_tr::const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::iter_difference_t<iterator>;
    using size_type = typename alloc_tr::size_type;

private:
    static constexpr const char* at_out_err = "darray::at(pos) - Attempt to access a non-existent index";

    size_type mcap;
    size_type msize;
    Allocator malloc;
    pointer mbuf;

    [[nodiscard]] constexpr pointer hint_allocate(size_type n)
    {
        if(mcap > 0)
            return alloc_tr::allocate(malloc, n, mbuf + mcap - 1);
        else
            return alloc_tr::allocate(malloc, n);
    }

    constexpr void grow_alloc(size_type new_cap)
    {
        assert(new_cap > 0 && new_cap >= size());
        pointer new_buf = hint_allocate(new_cap); // alloc_tr::allocate(malloc, new_cap);
        constexpr bool is_copyable = std::is_copy_constructible<value_type>::value;
        if constexpr(!std::is_nothrow_move_constructible<value_type>::value && is_copyable) {
            try {
                std_ext::uninitialized_copy(malloc, mbuf, mbuf + size(), new_buf);
            } catch(...) {
                alloc_tr::deallocate(malloc, new_buf, new_cap);
                throw;
            }
            std_ext::destroy(malloc, mbuf, mbuf + size());
        } else {
            try {
                std_ext::uninitialized_destroy_move(malloc, mbuf, mbuf + size(), new_buf);
            } catch(...) {
                alloc_tr::deallocate(malloc, new_buf, new_cap);
                throw;
            }
        }
        std_ext::safe_deallocate(malloc, mbuf, capacity());
        mbuf = new_buf;
        mcap = new_cap;
    }

    template <bool OtherFill>
    constexpr void internal_resize(size_type new_size,
        std::conditional_t<OtherFill, const_reference, std::tuple<>> value)
    {
        if(size() == new_size)
            return;
        const bool need_grow = new_size > capacity();
        if(need_grow || new_size > size()) {
            if(need_grow)
                grow_alloc(new_size);
            if constexpr(OtherFill)
                std_ext::uninitialized_fill(malloc, mbuf + size(), mbuf + new_size, value);
            else
                std_ext::uninitialized_default_construct(malloc, mbuf + size(), mbuf + new_size);
        } else
            std_ext::destroy(malloc, mbuf + new_size, mbuf + msize);
        msize = new_size;
    }

    template <bool Init, class InputIt> constexpr void internal_assign(InputIt first, InputIt last)
    {
        using tag = typename std::iterator_traits<InputIt>::iterator_category;
        static_assert(!std::is_same<tag, std::output_iterator_tag>::value);
        if constexpr(std::is_same<tag, std::input_iterator_tag>::value) {
            if constexpr(!Init)
                clear_and_free();
            std::copy(first, last, std::back_inserter(*this));
        } else {
            const size_type new_cap = static_cast<size_type>(std::distance(first, last));
            if(new_cap > capacity() || Init) {
                pointer new_buf = hint_allocate(new_cap);
                try {
                    std_ext::uninitialized_copy(malloc, first, last, new_buf);
                } catch(...) {
                    alloc_tr::deallocate(malloc, new_buf, new_cap);
                    throw;
                }
                if constexpr(!Init)
                    destroy_and_dealloc();
                mcap = msize = new_cap;
                mbuf = new_buf;
            } else {
                // new_cap<=capacity && !Init
                if(new_cap <= msize) {
                    std_ext::initialized_rewrite(malloc, first, last, mbuf);
                    std_ext::destroy(malloc, mbuf + new_cap, mbuf + msize);
                } else
                    std_ext::initialized_first_destroy_and_copy(malloc, first, last, msize, mbuf);

                msize = new_cap;
            }
        }
    }

    constexpr void clear_and_free()
    {
        clear();
        std_ext::safe_deallocate(malloc, mbuf, capacity());
        mcap = 0;
    }

    constexpr void destroy_and_dealloc()
    {
        std_ext::destroy(malloc, mbuf, mbuf + size());
        std_ext::safe_deallocate(malloc, mbuf, capacity());
    }

    constexpr size_type next_cap() const
    {
        if(mcap != 0) {
            if constexpr(ratio::den != 1) {
                return static_cast<size_type>((static_cast<std::uintmax_t>(mcap) * ratio::num + ratio::den - 1) /
                    ratio::den); // We are wary of overflow and rounding up
            } else
                return capacity() * ratio::num;
        }
        return 1;
    }

    constexpr static int private_call = 0;
    constexpr darray(size_type init, const Allocator& alloc, int) noexcept
        : mcap(init)
        , msize(init)
        , malloc(alloc)
        , mbuf(std_ext::safe_allocate(malloc, init))
    {
    }

public:
    //------------------------------
    //-----CONSTRUCTORS SECTION-----
    //------------------------------
    constexpr explicit darray(const Allocator& alloc) noexcept
        : mcap(0)
        , msize(0)
        , malloc(alloc)
    //, mbuf(nullptr)
    {
    }

    constexpr darray() noexcept(noexcept(Allocator()))
        : darray(Allocator())
    {
    }

    constexpr explicit darray(size_type count, const Allocator& alloc = Allocator())
        : darray(count, alloc, private_call)
    {
        std_ext::uninitialized_default_construct(alloc, mbuf, mbuf + size());
    }

    constexpr darray(size_type count, const value_type& value, const Allocator& alloc = Allocator())
        : darray(count, alloc, private_call)
    {
        std_ext::uninitialized_fill(alloc, mbuf, mbuf + size(), value);
    }
    //
    // typename std::iterator_traits<InputIt>::value_type
    template <class InputIt, std::enable_if_t<!std::is_same<std::iter_value_t<InputIt>, void>::value, int> = 0>
    constexpr darray(InputIt first, InputIt last, const Allocator& alloc = Allocator())
        : darray(alloc)
    {
        internal_assign<true>(first, last);
    }

    constexpr darray(const darray& o, const Allocator& alloc)
        : darray(o.begin(), o.end(), alloc)
    {
    }
    constexpr darray(const darray& o)
        : darray(o, alloc_tr::select_on_container_copy_construction(o.malloc))
    {
    }

    constexpr darray(darray&& o) noexcept
        : mcap(std::exchange(o.mcap, 0))
        , msize(std::exchange(o.msize, 0))
        , malloc(std::move(o.malloc))
        , mbuf(std::move(o.mbuf)) // vs std::exchange(o.mbuf, nullptr)
    {
    }
    constexpr darray(darray&& o, const Allocator& alloc) noexcept
        : malloc(alloc)
    {
        if(o.get_allocator() != get_allocator()) {
            msize = mcap = 0;
            // vs mbuf=nullptr;
            internal_assign<true>(std::make_move_iterator(o.first), std::make_move_iterator(o.last));
            o.msize = o.mcap = 0;
            // vs o.mbbuf=nullptr
        } else {
            mcap = std::exchange(o.mcap, 0);
            msize = std::exchange(o.msize, 0);
            mbuf = std::move(o.mbuf); // vs std::exchange(o.mbuf, nullptr);
        }
    }

    constexpr darray(std::initializer_list<value_type> il, const Allocator& alloc = Allocator())
        : darray(il.begin(), il.end(), alloc)
    {
    }

    constexpr ~darray()
    {
        destroy_and_dealloc();
    }
    //------------------------------
    //-----EDN OF CONSTRUCTORS------
    //------------------------------
    //------------------------------
    //----ELEMENT ACCESS SECTION----
    //------------------------------
    constexpr reference operator[](size_type pos)
    {
        return mbuf[pos];
    }
    constexpr const_reference operator[](size_type pos) const
    {
        return mbuf[pos];
    }
    constexpr reference at(size_type pos)
    {
        if(pos < size()) {
            return operator[](pos);
        } else
            throw std::out_of_range(at_out_err);
    }
    constexpr const_reference at(size_type pos) const
    {
        // TODO Duplicate
        if(pos < size()) {
            return operator[](pos);
        } else
            throw std::out_of_range(at_out_err);
    }
    constexpr reference front()
    {
        return *begin();
    }
    constexpr const_reference front() const
    {
        return *cbegin();
    }
    constexpr reference back()
    {
        return *std::prev(end());
    }
    constexpr const_reference back() const
    {
        return *std::prev(cend());
    }
    constexpr pointer data() noexcept
    {
        return mbuf;
    }
    constexpr const pointer data() const noexcept
    {
        return mbuf;
    }
    //------------------------------
    //----END OF ELEMENT ACCESS-----
    //------------------------------

    //------------------------------
    //------ITERATORS SECTION-------
    //------------------------------
    constexpr iterator begin() noexcept
    {
        return mbuf;
    }
    constexpr const_iterator begin() const noexcept
    {
        return mbuf;
    }
    constexpr const_iterator cbegin() const noexcept
    {
        return const_cast<const darray&>(*this).begin();
    }
    constexpr iterator end() noexcept
    {
        return mbuf + size();
    }
    constexpr const_iterator end() const noexcept
    {
        return mbuf + size();
    }
    constexpr const_iterator cend() const noexcept
    {
        return const_cast<const darray&>(*this).end();
    }
    // reverse
    constexpr reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_cast<const darray&>(*this).rbegin();
    }

    constexpr reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_cast<const darray&>(*this).rend();
    }
    //------------------------------
    //------END OF ITERATORS--------
    //------------------------------
    //------------------------------
    //------CAPACITY SECTION--------
    //------------------------------
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    }
    constexpr size_type size() const noexcept
    {
        return msize;
    }
    constexpr size_type max_size() const noexcept
    {
        return alloc_tr::max_size(malloc);
    }
    constexpr void reserve(size_type new_cap)
    {
        if(new_cap > mcap) {
            grow_alloc(new_cap);
        }
    }
    constexpr size_type capacity() const noexcept
    {
        return mcap;
    }

    constexpr void shrink_to_fit()
    {
        if(size() == capacity())
            return;
        if(size() != 0)
            grow_alloc(size());
        else
            std_ext::safe_deallocate(malloc, mbuf, capacity());
    }
    //------------------------------
    //------END OF CAPACITY --------
    //------------------------------
    //------------------------------
    //-----MODIFIERS SECTION--------
    //------------------------------
    constexpr void clear() noexcept
    {
        std_ext::destroy(malloc, mbuf, mbuf + msize); // TODO reverse order is better?
        msize = 0;
    }

    template <class... Args> constexpr reference emplace_back(Args&&... args)
    {
        if(msize >= mcap) {
            // std::cout << "\n" << "GROW" << next_cap() << "\n";
            grow_alloc(next_cap());
        }
        alloc_tr::construct(malloc, mbuf + msize, std::forward<Args>(args)...);
        ++msize;
        return back();
    }

    constexpr void push_back(const value_type& val)
    {
        emplace_back(val);
    }

    constexpr void push_back(value_type&& val)
    {
        emplace_back(std::move(val));
    }

    constexpr void pop_back() noexcept
    {
        --msize;
        alloc_tr::destroy(malloc, mbuf + msize);
    }

    constexpr void resize(size_type new_size)
    {
        internal_resize<false>(new_size, std::tuple<>());
    }
    constexpr void resize(size_type new_size, const_reference value)
    {
        internal_resize<true>(new_size, value);
    }

    constexpr void swap(darray& o) noexcept(
        alloc_tr::propagate_on_container_swap::value || alloc_tr::is_always_equal::value)
    {
        using std::swap;
        if constexpr(alloc_tr::propagate_on_container_swap::value)
            swap(malloc, o.malloc);
        else {
            if constexpr(!alloc_tr::is_always_equal::value)
                assert(malloc == o.malloc);
        }
        swap(mcap, o.mcap);
        swap(msize, o.msize);
        swap(mbuf, o.mbuf);
    }
    //------------------------------
    //------END OF MODIFIERS--------
    //------------------------------

    //------------------------------
    //---MEMBER FUNCTIONS SECTION---
    //------------------------------
    template <class InputIt> constexpr void assign(InputIt first, InputIt last)
    {
        internal_assign<false>(first, last);
    }
    constexpr void assign(std::initializer_list<value_type> il)
    {
        assign(il.begin(), il.end());
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return malloc;
    }

    darray& operator=(const darray& o)
    {
        if(this != &o) {
            if constexpr(alloc_tr::propagate_on_container_copy_assignment::value) {
                if(malloc != o.malloc) {
                    clear_and_free();
                    malloc = o.malloc;
                }
            }
            assign(o.begin(), o.end());
        }
        return *this;
    }

    constexpr darray& operator=(darray&& o) noexcept(
        alloc_tr::propagate_on_container_move_assignment::value || alloc_tr::is_always_equal::value)
    {
        if(this != &o) {
            const auto updater = [&]() {
                destroy_and_dealloc();
                msize = std::exchange(o.msize, 0);
                mcap = std::exchange(o.mcap, 0);
                mbuf = std::move(o.mbuf); // TODO vs std::exchange(o.mbuf,nullptr)
            };
            if constexpr(alloc_tr::propagate_on_container_move_assignment::value) {
                updater();
                malloc = std::move(o.malloc); // TODO with move vs without move
            } else {
                if(malloc != o.malloc)
                    assign(std::make_move_iterator(o.begin()), std::make_move_iterator(o.end()));
                else
                    updater();
            }
        }
        return *this;
    }

    constexpr darray& operator=(std::initializer_list<value_type> il)
    {
        assign(il.begin(), il.end());
    }
    //------------------------------
    //---END OF MEMBER FUNCTIONS----
    //------------------------------

    //------------------------------
    //------OPERATORS SECTION-------
    //------------------------------
    // for operator +=,-=, etc...
    template <class ForwardIt, class BinaryOp>
    constexpr darray& assign_transform(ForwardIt first, ForwardIt last, BinaryOp binop)
    {
        std_ext::binary_apply(
            begin(), begin() + std::min(size(), static_cast<size_type>(std::distance(first, last))), first, binop);
        return *this;
    }

    template <class LForwardIt, class RForwardIt, class BinaryOp>
    constexpr static darray
    transform(LForwardIt lfirst, LForwardIt llast, RForwardIt rfirst, RForwardIt rlast, BinaryOp binary_op)
    {
        darray res;
        const size_type min_dist = std::min(std::distance(lfirst, llast), std::distance(rfirst, rlast));
        if(min_dist == 0)
            return res;
        res.mbuf = alloc_tr::allocate(res.malloc, min_dist);
        res.mcap = res.msize = min_dist;
        try {
            std_ext::uninitialized_transform_n(res.malloc, res.mbuf, min_dist, lfirst, rfirst, binary_op);
        } catch(...) {
            alloc_tr::deallocate(res.malloc, res.mbuf, res.mcap);
            res.mcap = res.msize = 0;
            throw;
        }
        return res;
    }
    friend constexpr bool operator==(const darray& l, const darray& r)
    {
        if(l.size() != r.size())
            return false;
        return std::equal(l.begin(), l.end(), r.begin());
    }

    friend constexpr auto operator<=>(const darray& l, const darray& r)
    {
        return std::lexicographical_compare_three_way(l.begin(), l.end(), r.begin(), r.end());
    }
    //+,-,*,/
    friend constexpr darray operator+(const darray& l, const darray& r)
    {
        return transform(l.begin(), l.end(), r.begin(), r.end(), std::plus<> {});
    }
    friend constexpr darray operator-(const darray& l, const darray& r)
    {
        return transform(l.begin(), l.end(), r.begin(), r.end(), std::minus<> {});
    }
    friend constexpr darray operator*(const darray& l, const darray& r)
    {
        return transform(l.begin(), l.end(), r.begin(), r.end(), std::multiplies<> {});
    }
    friend constexpr darray operator/(const darray& l, const darray& r)
    {
        return transform(l.begin(), l.end(), r.begin(), r.end(), std::divides<> {});
    }
    //+=, -=, *=, /=
    constexpr darray& operator+=(const darray& r)
    {
        return assign_transform(r.begin(), r.end(), std_ext::assign_add {});
    }
    constexpr darray& operator-=(const darray& r)
    {
        return assign_transform(r.begin(), r.end(), std_ext::assign_sub {});
    }
    constexpr darray& operator*=(const darray& r)
    {
        return assign_transform(r.begin(), r.end(), std_ext::assign_mul {});
    }
    constexpr darray& operator/=(const darray& r)
    {
        return assign_transform(r.begin(), r.end(), std_ext::assign_div {});
    }
    //------------------------------
    //------END OF OPERATORS--------
    //------------------------------
};
template <class Alloc, class Ratio>
void swap(darray<Alloc, Ratio>& l, darray<Alloc, Ratio>& r) noexcept(noexcept(l.swap(r)))
{
    l.swap(r);
}
}
template <class T, class Ratio = std::ratio<3, 2>> using darray = darray_ns::darray<std::allocator<T>, Ratio>;
/****************************************
 ***************************************
 ***************************************
 ***************************************
 ***************************************
 ****************************************/
//------------------------------
//--------DEBUG SECTION---------
//------------------------------
namespace debug
{
#ifndef NDEBUG
class out_t : public std::stringstream
{
};
out_t out;
/*using out_t = decltype(std::cout);
out_t& out = std::cout;*/

template <class T> std::ostream& operator<<(out_t& os, T&& v)
{
    return static_cast<std::ostream&>(os) << (std::forward<T>(v));
}

void show()
{
    std::cout << out.str();
}
void clear()
{
    out.str("");
}

template <class Alloc, class Ratio>
std::ostream& operator<<(std::ostream& os, const ::darray_ns::darray<Alloc, Ratio>& d)
{
    return std_ext::out_content(os, d.cbegin(), d.cend(), ", ") << '\n';
}

template <class Alloc, class Ratio> void state(const darray_ns::darray<Alloc, Ratio>& d)
{
    out << "\nContent: ";
    std_ext::out_content(debug::out, d.cbegin(), d.cend(), "");
    out << "\nState: "
        << "Size=" << d.size() << " Capacity=" << d.capacity() << '\n';
}

void lb(const char* msg)
{
    out << "\n\n" << msg << '\n';
}

struct monitor {
    int a;
    bool del;

private:
    monitor(int a_, bool del_) noexcept
        : a(a_)
        , del(del_)
    {
    }
    static void write(int l, int r, const char* msg)
    {
        debug::out << '[' << l << ' ' << msg << ' ' << r << ']';
    }
    static void write(int v, const char* msg)
    {
        debug::out << '[' << v << ' ' << msg << ']';
    }
    static void write(const char* msg)
    {
        debug::out << '[' << msg << ']';
    }

public:
    monitor()
        : monitor(-100, false)
    {
        write("default");
    }
    monitor(const monitor& t)
        : monitor(t.a, t.del)
    {
        write(a, "copy&");
    }
    monitor(monitor&& t) noexcept
        : monitor(t.a, t.del)
    {
        write(a, "copy&&");
    }
    monitor(int a_)
        : monitor(a_, false)
    {
        write(a, "cr");
    }
    ~monitor()
    {
        //!!! Endless lopp for debug
        while(del)
            ;
        write(a, "des");
        del = true;
    }
    monitor& operator=(const monitor& t)
    {
        write(a, t.a, "=&");
        a = t.a;
        del = t.del;
        return *this;
    }
    monitor& operator=(monitor&& t)
    {
        write(a, t.a, "=&&");
        std::swap(a, t.a);
        std::swap(del, t.del);
        return *this;
    }
    monitor& operator+=(const monitor& t)
    {
        write(a, t.a, "+=");
        a += t.a;
        return *this;
    }
    friend monitor operator+(const monitor& l, const monitor& r)
    {
        write(l.a, r.a, "+");
        return monitor(l.a + r.a);
    }
    friend std::ostream& operator<<(std::ostream& os, const monitor& v)
    {
        os << "monitor[" << v.a << "]";
        return os;
    }
};

void check_passed(const char* correct, int n)
{
    if(out.str() != correct) {
        std::cout << "test NOT passed " << n << '\n';
        show();
    } else
        std::cout << "test passed " << n << '\n';
}
template <class T> struct mallocator {
    using value_type = T;
    using size_type = std::size_t;

    constexpr size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    value_type* allocate(const size_type n) const
    {
        if(n == 0) {
            return nullptr;
        }
        if(n > max_size()) {
            throw std::bad_array_new_length();
        }
        void* const ptr = malloc(n * sizeof(value_type));
        if(!ptr) {
            throw std::bad_alloc();
        }
        //  std::cout << ptr << "All<-\n";
        debug::out << "\nAllocator: allocate " << n << '\n';
        return static_cast<value_type*>(ptr);
    }
    void deallocate(value_type* const ptr, size_t n) const noexcept
    {
        // std::cout << ptr << "Del<-\n";
        // debug::out_content(std::cout, ptr, ptr + n, " ");
        debug::out << "\nAllocator: deallocate " << n << '\n';
        free(ptr);
    }
};

//------------------------------
//----DEBUG::TESTS SECTION------
//------------------------------
using dbg_darray = darray_ns::darray<mallocator<monitor>>;
namespace tests
{
    void test1()
    {
        debug::clear();
        dbg_darray d { 1, 2, 3, 4 };

        state(d);
        lb("resize with default construct");
        d.resize(d.size() * 2);
        state(d);
        lb("resize shrink");
        d.resize(d.size() / 2);
        state(d);
        lb("resize fill");
        d.resize(d.size() * 4, -41);
        state(d);

        lb("assign");
        d.assign({ 12, 13, 14, 15 });
        state(d);
        lb("shrink");
        d.shrink_to_fit();
        state(d);

        lb("assign grow");
        d.assign({ 21, 22, 23, 24, 25 });
        state(d);

        lb("assign shrink");
        d.assign({ 31, 32, 33, 34 });
        state(d);
        lb("assign shrink less");
        d.assign({ 41, 42 });
        state(d);

        lb("assign shrink greate");
        d.assign({ 51, 52, 53, 54 });
        state(d);
        constexpr const char* correct = R"([1 cr][2 cr][3 cr][4 cr]
Allocator: allocate 4
[1 copy&][2 copy&][3 copy&][4 copy&][4 des][3 des][2 des][1 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]
State: Size=4 Capacity=4


resize with default construct

Allocator: allocate 8
[1 copy&&][1 des][2 copy&&][2 des][3 copy&&][3 des][4 copy&&][4 des]
Allocator: deallocate 4
[default][default][default][default]
Content: monitor[1]monitor[2]monitor[3]monitor[4]monitor[-100]monitor[-100]monitor[-100]monitor[-100]
State: Size=8 Capacity=8


resize shrink
[-100 des][-100 des][-100 des][-100 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]
State: Size=4 Capacity=8


resize fill
[-41 cr]
Allocator: allocate 16
[1 copy&&][1 des][2 copy&&][2 des][3 copy&&][3 des][4 copy&&][4 des]
Allocator: deallocate 8
[-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 copy&][-41 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]monitor[-41]
State: Size=16 Capacity=16


assign
[12 cr][13 cr][14 cr][15 cr][1 des][12 copy&][2 des][13 copy&][3 des][14 copy&][4 des][15 copy&][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][-41 des][15 des][14 des][13 des][12 des]
Content: monitor[12]monitor[13]monitor[14]monitor[15]
State: Size=4 Capacity=16


shrink

Allocator: allocate 4
[12 copy&&][12 des][13 copy&&][13 des][14 copy&&][14 des][15 copy&&][15 des]
Allocator: deallocate 16

Content: monitor[12]monitor[13]monitor[14]monitor[15]
State: Size=4 Capacity=4


assign grow
[21 cr][22 cr][23 cr][24 cr][25 cr]
Allocator: allocate 5
[21 copy&][22 copy&][23 copy&][24 copy&][25 copy&][12 des][13 des][14 des][15 des]
Allocator: deallocate 4
[25 des][24 des][23 des][22 des][21 des]
Content: monitor[21]monitor[22]monitor[23]monitor[24]monitor[25]
State: Size=5 Capacity=5


assign shrink
[31 cr][32 cr][33 cr][34 cr][21 des][31 copy&][22 des][32 copy&][23 des][33 copy&][24 des][34 copy&][25 des][34 des][33 des][32 des][31 des]
Content: monitor[31]monitor[32]monitor[33]monitor[34]
State: Size=4 Capacity=5


assign shrink less
[41 cr][42 cr][31 des][41 copy&][32 des][42 copy&][33 des][34 des][42 des][41 des]
Content: monitor[41]monitor[42]
State: Size=2 Capacity=5


assign shrink greate
[51 cr][52 cr][53 cr][54 cr][41 des][51 copy&][42 des][52 copy&][53 copy&][54 copy&][54 des][53 des][52 des][51 des]
Content: monitor[51]monitor[52]monitor[53]monitor[54]
State: Size=4 Capacity=5
)";
        // std::cout<<""<<out.str().size()<<"!!!"<<std::string(correct).size();
        // std::cout<<out.str();

        check_passed(correct, 1);
    }

    void test2()
    {
        debug::clear();
        dbg_darray d { 1, 2, 3, 4, 5 };
        lb("1x push and 3x pop");
        d.push_back(6);
        d.pop_back();
        d.pop_back();
        state(d);
        lb("7x push");
        for(int i = 1; i <= 7; ++i)
            d.push_back(-i);
        state(d);
        lb("1x pop");
        d.pop_back();
        state(d);
        constexpr const char* c = R"([1 cr][2 cr][3 cr][4 cr][5 cr]
Allocator: allocate 5
[1 copy&][2 copy&][3 copy&][4 copy&][5 copy&][5 des][4 des][3 des][2 des][1 des]

1x push and 3x pop
[6 cr]
Allocator: allocate 8
[1 copy&&][1 des][2 copy&&][2 des][3 copy&&][3 des][4 copy&&][4 des][5 copy&&][5 des]
Allocator: deallocate 5
[6 copy&&][6 des][6 des][5 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]
State: Size=4 Capacity=8


7x push
[-1 cr][-1 copy&&][-1 des][-2 cr][-2 copy&&][-2 des][-3 cr][-3 copy&&][-3 des][-4 cr][-4 copy&&][-4 des][-5 cr]
Allocator: allocate 12
[1 copy&&][1 des][2 copy&&][2 des][3 copy&&][3 des][4 copy&&][4 des][-1 copy&&][-1 des][-2 copy&&][-2 des][-3 copy&&][-3 des][-4 copy&&][-4 des]
Allocator: deallocate 8
[-5 copy&&][-5 des][-6 cr][-6 copy&&][-6 des][-7 cr][-7 copy&&][-7 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]monitor[-1]monitor[-2]monitor[-3]monitor[-4]monitor[-5]monitor[-6]monitor[-7]
State: Size=11 Capacity=12


1x pop
[-7 des]
Content: monitor[1]monitor[2]monitor[3]monitor[4]monitor[-1]monitor[-2]monitor[-3]monitor[-4]monitor[-5]monitor[-6]
State: Size=10 Capacity=12
)";
        check_passed(c, 2);
    }
    void test3()
    {
        debug::clear();
        dbg_darray l { 6, 2, 3, 4, 5, -999, -998 };
        dbg_darray r { -50, -40, -30, -20, -10 };
        lb("plus opr +");
        dbg_darray res1 = l + r;
        state(res1);
        lb("inv plus opr +");
        dbg_darray res2 = r + l;
        state(res2);
        lb("plus opr +=");
        res1 += dbg_darray { 11, 12 }; // TODO may be adding operation +=(init list)
        state(res1);
        lb("index test");
        out << res1[0] << res1[1] << res1.at(2) << res1[3] << res1.at(4);
        lb("end");
        constexpr const char* c = R"([6 cr][2 cr][3 cr][4 cr][5 cr][-999 cr][-998 cr]
Allocator: allocate 7
[6 copy&][2 copy&][3 copy&][4 copy&][5 copy&][-999 copy&][-998 copy&][-998 des][-999 des][5 des][4 des][3 des][2 des][6 des][-50 cr][-40 cr][-30 cr][-20 cr][-10 cr]
Allocator: allocate 5
[-50 copy&][-40 copy&][-30 copy&][-20 copy&][-10 copy&][-10 des][-20 des][-30 des][-40 des][-50 des]

plus opr +

Allocator: allocate 5
[6 + -50][-44 cr][-44 copy&&][-44 des][2 + -40][-38 cr][-38 copy&&][-38 des][3 + -30][-27 cr][-27 copy&&][-27 des][4 + -20][-16 cr][-16 copy&&][-16 des][5 + -10][-5 cr][-5 copy&&][-5 des]
Content: monitor[-44]monitor[-38]monitor[-27]monitor[-16]monitor[-5]
State: Size=5 Capacity=5


inv plus opr +

Allocator: allocate 5
[-50 + 6][-44 cr][-44 copy&&][-44 des][-40 + 2][-38 cr][-38 copy&&][-38 des][-30 + 3][-27 cr][-27 copy&&][-27 des][-20 + 4][-16 cr][-16 copy&&][-16 des][-10 + 5][-5 cr][-5 copy&&][-5 des]
Content: monitor[-44]monitor[-38]monitor[-27]monitor[-16]monitor[-5]
State: Size=5 Capacity=5


plus opr +=
[11 cr][12 cr]
Allocator: allocate 2
[11 copy&][12 copy&][-44 += 11][-38 += 12][11 des][12 des]
Allocator: deallocate 2
[12 des][11 des]
Content: monitor[-33]monitor[-26]monitor[-27]monitor[-16]monitor[-5]
State: Size=5 Capacity=5


index test
monitor[-33]monitor[-26]monitor[-27]monitor[-16]monitor[-5]

end
)";
        check_passed(c, 3);
    }
}
#endif
void testing()
{
#ifndef NDEBUG
    using namespace tests;
    test1();
    test2();
    test3();
#endif
}

}
//------------------------------
//---------END DEBUG------------
//------------------------------

namespace univerity
{
struct complex {
    int re;
    int im;
    constexpr complex& operator+=(complex b) noexcept
    {
        re += b.re;
        im += b.im;
        return *this;
    }
    constexpr complex& operator-=(complex b) noexcept
    {
        re -= b.re;
        im -= b.im;
        return *this;
    }
    constexpr complex& operator*=(complex b) noexcept
    {
        return *this = { re * b.re - im * b.im, re * b.im + b.re * im };
    }
};

constexpr complex operator+(complex a, complex b) noexcept
{
    return a += b;
}
constexpr complex operator-(complex a, complex b) noexcept
{
    return a -= b;
}
constexpr complex operator*(complex a, complex b) noexcept
{
    return a *= b;
}

template <class Unary> bool check_char(std::istream& is, Unary f)
{
    const int peek = is.peek();
    if(peek == std::char_traits<char>::eof())
        return false;
    if(!f(static_cast<char>(peek))) {
        is.setstate(std::ios::failbit);
        return false;
    }
    return true;
}
std::istream& operator>>(std::istream& is, complex& c)
{
    is >> c.re;
    if(is.fail())
        return is;

    if(!check_char(is, [](char c) { return c == '+' || c == '-'; }))
        return is;
    is.ignore();
    if(!check_char(is, [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
        return is;
    is >> c.im;
    if(is.fail())
        return is;

    if(!check_char(is, [](char c) { return c == 'i'; }))
        return is;
    is.ignore();
    return is;
}

std::ostream& operator<<(std::ostream& os, complex c)
{
    os << c.re;
    const bool showpos = os.flags() & std::ios_base::showpos;
    if(!showpos)
        os << std::showpos;
    os << c.im << 'i';
    if(!showpos)
        os.unsetf(std::ios_base::showpos);
    return os;
}

template <typename Type>
void request_type(Type& v, const std::string_view welcome, const std::string_view err_str = "Error, please try again\n")
{
    while(!std::cin.eof()) {
        std::cout << welcome;
        std::cin >> v;
        bool ok = true;
        if(std::cin.fail()) {
            std::cout << err_str;
            std::cin.clear();
            ok = false;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(ok)
            return;
    }
    // if std::cin.eof It might be better to pass the function
    std::exit(0);
}

template <class InputIt> void out_table(InputIt lfirst, InputIt llast, InputIt rfirst)
{
    constexpr int index_wid = 3;
    constexpr int cpx_wid = 13;
    typedef complex (*binary)(complex, complex);

    const std::array<std::pair<std::string_view, binary>, 5> cpx_funcs = {
        { { " a", [](complex a, complex b) { return a; } }, { " b", [](complex a, complex b) { return b; } },
            { " +", [](complex a, complex b) { return a + b; } }, { " -", [](complex a, complex b) { return a - b; } },
            { " *", [](complex a, complex b) { return a * b; } } }
    };
    constexpr int total_wid = cpx_wid * cpx_funcs.size() + index_wid;
    auto print_break = [&] {
        std::cout.width(total_wid + cpx_funcs.size() + 1);
        std::cout.fill('-');
        std::cout << '-' << '\n';
        std::cout.fill(' ');
    };
    auto print_headers = [&] {
        std::cout << std::setw(index_wid) << " n" << '|';
        for(const auto& f : cpx_funcs)
            std::cout << std::setw(cpx_wid) << f.first << '|';
        std::cout << std::endl;
    };
    std::cout.setf(std::ios::left, std::ios::adjustfield);
    print_break();
    print_headers();
    print_break();
    std::stringstream ss;
    for(typename std::iter_difference_t<InputIt> i = 1; lfirst != llast; ++lfirst, ++rfirst, ++i) {
        std::cout << std::setw(index_wid) << i << '|';
        for(const auto& f : cpx_funcs) {
            ss << f.second(*lfirst, *rfirst);
            std::cout << std::setw(cpx_wid) << ss.str() << '|';
            ss.str("");
        }
        std::cout << std::endl;
    }
}
enum menu { input_data = 1, random_data, help, close }; // with random_data name not complile
constexpr int first_menu = input_data;
constexpr int last_menu = close;
/*namespace menu_option_ns
{
    enum menu_option { input_data = 1, cr_random_data, help, close };//with random_data name not complile
    constexpr int first_option = input_data;
    constexpr int last_option = close;
}*/

using cpx_array = darray<complex>;

void input_data_operation()
{
    const auto input_arg_array = [] {
        cpx_array res;
        using value_t = typename cpx_array::value_type;
        auto& is = std::cin;
        std::istream_iterator<value_t> ss(is);
        const std::istream_iterator<value_t> end_ss;
        while(ss != end_ss) {
            res.push_back(*ss++);
        }
        if(is.fail()) {
            is.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return res;
    };
    std::cout << "Input the first array of complex numbers: ";
    cpx_array larr(input_arg_array());
    std::cout << "Input the second array of complex numbers: ";
    cpx_array rarr(input_arg_array());
    out_table(larr.begin(), larr.begin() + std::min(larr.size(), rarr.size()), rarr.begin());
}
struct eng_wrap {
    std::mt19937 engine;
    eng_wrap()
    {
        std::random_device device;
        engine.seed(device());
    }
};
eng_wrap eng;

template <int min, int max> struct random_iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = complex;
    using reference = complex&;
    using pointer = complex*;
    using size_type = std::size_t;
    static std::uniform_int_distribution<int> distr;
    static complex next()
    {
        return complex { distr(eng.engine), distr(eng.engine) };
    }
    size_type iters = 0;
    complex current { next() };

    random_iterator& operator++()
    {
        current = next();
        --iters;
        return *this;
    }
    random_iterator operator++(int)
    {
        random_iterator tmp { *this };
        operator++();
        return tmp;
    }
    friend bool operator==(random_iterator l, random_iterator r)
    {
        return l.iters == r.iters;
    }
    complex& operator*()
    {
        return current;
    }
};
template <int min, int max> std::uniform_int_distribution<int> random_iterator<min, max>::distr { min, max };

void random_data_operation()
{
    using rnd_it = random_iterator<-200, 200>;
    static std::uniform_int_distribution<rnd_it::size_type> len_distr(10, 100);
    rnd_it it { len_distr(eng.engine) };
    out_table(it, rnd_it(), rnd_it{it.iters});
}
}

int main()
{
    debug::testing();
    using namespace univerity;
    std::cin >> std::skipws; // for input_data_operation(); method
    std::cout << "Welcome to Simplest Complex Calculator 1.0!\n"
              << menu::input_data << " - Input data\n"
              << menu::random_data << " - Random data\n"
              << menu::help << " - Get help about of using program\n"
              << menu::close << " - Close the program\n\n";
    while(!std::cin.eof()) {
        int choose;
        request_type(choose, "What would you like to do?\n", "Error, can not recognizable integer number\n");
        switch(choose) {
        case menu::input_data:
            input_data_operation();
            break;
        case menu::random_data:
            random_data_operation();
            break;
        case menu::help:
            std::cout << "=======Inpud data description=======\n"
                         "In the menu of item 1('Input data'), a sequence of complex numbers separated by\n"
                         "spaces or newline characters is expected.\n"
                         "Each complex number should have the form a+bi, where a and b are integers.\n"
                         "To complete the input, enter a dot('.') or anything that is not a complex number\n"
                         "Possible example: '9+8i 7+7i 11+21i.'\n"
                         "====================================\n"
                         "Auther: PavelPI. 2021.12.11\n";
            break;
        case menu::close:
            std::exit(0);
            break;
        default:
            std::cout << "Unkown choose\n";
            break;
        }
        if(choose >= first_menu && choose <= last_menu) {
            std::cout << "\nOK. ";
        }
    }
    return 0;
}
/* auto qq=new int(2);
 qq[0]=-1;
 qq[1]=-2;
 darray_ns::darray<std::allocator<int>> q(3,10);
 std_ext::out_content(std::cout,q.begin(),q.end(),", ");*/
