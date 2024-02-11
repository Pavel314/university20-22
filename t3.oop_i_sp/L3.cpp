#include <array>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <ios> /*skeepws*/
#include <iostream>
#include <iterator>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility> /*std::move*/
#include <vector>

//!!!Необходим С++20!!!
//Вариант 21. АТД - двухмерный массив (матрица) вещественных чисел. Перегрузить операции: (), m-m, m-s,==
//В случае невозможности компиляции, запуск может быть осуществлен онлайн-ресурсом: https://wandbox.org/
namespace _impl_details
{
template <class Iter, bool IsConstIt> class istep_iterator
{
private:
    using base_iter = Iter;

public:
    using iterator_type = Iter;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::iter_difference_t<base_iter>;
    using value_type = std::iter_value_t<base_iter>;
    using pointer = typename std::iterator_traits<base_iter>::pointer;
    using reference = std::iter_reference_t<base_iter>;
    using step_type = std::size_t;

protected:
    using mbc_reference = typename std::conditional<IsConstIt, const reference, reference>::type;
    using mbc_pointer = typename std::conditional<IsConstIt, const pointer, pointer>::type;
    base_iter it;
    base_iter eit; // This is iterator to last element in visiting group.
    step_type _step;

public:
    //---CONSTRUCTORS----
    constexpr istep_iterator()
        : it {}
        , eit {}
        , _step {} // init _step as zero(for safe).
    {
    }
    constexpr istep_iterator(base_iter it_, base_iter eit_, step_type step_)
        : it(it_)
        , eit(eit_)
        , _step(step_)
    {
    }
    constexpr istep_iterator(const istep_iterator<Iter, true>& it_)
        : istep_iterator { it_.it, it_.eit, it_._step }
    {
    }
    constexpr istep_iterator(const istep_iterator<Iter, false>& it_)
        : istep_iterator { it_.it, it_.eit, it_._step }
    {
    }
    constexpr istep_iterator& operator=(const istep_iterator& other)
    {
        it = other.it;
        eit = other.eit;
        _step = other._step;
        return *this;
    }

    //----ADDICTION METHODS----
    constexpr step_type step() const
    {
        return _step;
    }
    constexpr base_iter base() const
    {
        return it;
    }
    constexpr base_iter last_elemnt() const
    {
        return eit;
    }
    constexpr istep_iterator create_end() const
    {
        return istep_iterator { eit + 1, eit, _step };
    }
    constexpr bool can_move() const
    {
        return it != eit + 1;
    }

    //---OPERATORS---
    //"==" and "--"
    friend constexpr bool operator==(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO. Should other fields participate fields in comparison?
        return l.it == r.it;
    }
    friend constexpr bool operator!=(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO
        return l.it != r.it;
    }
    //---OPERATORS---
    //"*" and "->"
    constexpr mbc_reference operator*() const
    {
        return *it;
    }
    constexpr mbc_pointer operator->() const
    {
        return it.operator->();
    }
    //---------------OPERATORS----------------
    //"++" and "++(int)" and "--" and "--(int)"
    constexpr istep_iterator& operator++()
    {
        if(it == eit)
            ++it;
        else
            it += _step;
        return *this;
    }

    constexpr istep_iterator operator++(int)
    {
        istep_iterator tmp { *this };
        operator++();
        return tmp;
    }

    constexpr istep_iterator& operator--()
    {
        if(it - 1 == eit)
            --it;
        else
            it -= _step;
        return *this;
    }

    constexpr istep_iterator operator--(int)
    {
        istep_iterator tmp { *this };
        operator--();
        return tmp;
    }
    //--------------------OPERATORS----------------------
    //"it+n" and "it-n" and "it-it" and "n+it" and "n-it"
    constexpr istep_iterator operator+(difference_type n) const
    {
        return istep_iterator(it + n * _step, eit, _step);
    }
    constexpr istep_iterator operator-(difference_type n) const
    {
        return istep_iterator(it - n * _step, eit, _step);
    }
    constexpr istep_iterator operator-(const istep_iterator& l) const
    {
        // TODO
        return istep_iterator { it - l.it, eit, _step };
    }

    friend constexpr istep_iterator operator+(difference_type n, const istep_iterator& it)
    {
        return istep_iterator(n * it.step + it, it.eit, it.step);
    }

    friend constexpr istep_iterator operator-(difference_type n, const istep_iterator& it)
    {
        return istep_iterator(n * it.step - it, it.eit, it.step);
    }
    //--------OPERATORS----------
    //">" and "<" and ">=" and "<"
    friend constexpr bool operator>(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO
        return l.it > r.it;
    }
    friend constexpr bool operator<(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO
        return l.it < r.it;
    }
    friend constexpr bool operator>=(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO
        return l.it >= r.it;
    }

    friend constexpr bool operator<=(const istep_iterator& l, const istep_iterator& r)
    {
        // TODO
        return l.it <= r.it;
    }
    //-----OPERATORS--------
    //"+=" and "-=" and "[]"
    constexpr istep_iterator& operator+=(difference_type n)
    {
        it += _step * n;
        return *this;
    }
    constexpr istep_iterator& operator-=(difference_type n)
    {
        it -= _step * n;
        return *this;
    }
    constexpr reference operator[](difference_type n) const
    {
        return *it[n * _step];
    }
};
}
template <class T> using step_iterator = _impl_details::istep_iterator<T, false>;
template <class T> using const_step_iterator = _impl_details::istep_iterator<T, true>;

template <typename T> struct iterator_pair {
    T begin;
    T end;
    constexpr bool can_move() const
    {
        return begin != end;
    }
    constexpr std::iter_reference_t<T> operator*() const
    {
        return begin.operator*();
    }
    constexpr auto operator->() const
    {
        return begin.operator->();
    }
};

namespace utils
{
template <class T> struct is_carray : std::false_type {
};
template <class T, std::size_t N> struct is_carray<T[N]> : std::true_type {
};

template <typename T> struct is_bouned : std::false_type {
};
template <class T, std::size_t N> struct is_bouned<T[N]> : std::true_type {
};
template <class T, std::size_t N> struct is_bouned<std::array<T, N>> : std::true_type {
};
template <typename T> struct get_characters {
    using size_type = typename T::size_type;
    using value_type = typename T::value_type;
    using reference = typename T::reference;
    using const_reference = typename T::const_reference;
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;
    using reverse_iterator = typename T::reverse_iterator;
    using const_reverse_iterator = typename T::const_reverse_iterator;
};
template <class T, std::size_t N> struct get_characters<T[N]> {
    using size_type = std::size_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;
};
}
// note: Arrays T[n][m]... not supported (only T[n])
template <class Container> class matrix
{
private:
    using h = utils::get_characters<Container>;

public:
    using container = Container;
    using size_type = typename h::size_type;
    using value_type = typename h::value_type;
    using reference = typename h::reference;
    using const_reference = typename h::const_reference;
    using iterator = typename h::iterator;
    using const_iterator = typename h::const_iterator;
    using reverse_iterator = typename h::reverse_iterator;
    using const_reverse_iterator = typename h::const_reverse_iterator;
    constexpr static bool iscarray = utils::is_carray<Container>::value;
    constexpr static bool isbound = utils::is_bouned<Container>::value;

private:
    size_type _width;
    Container _cont;
    constexpr static bool check_rectangular(std::initializer_list<std::initializer_list<value_type>> l)
    {
        for(auto it = l.begin() + 1; it != l.end(); ++it) {
            if(it->size() != (it - 1)->size())
                return false;
        }
        return true;
    }

    constexpr inline size_type conv(size_type x, size_type y) const noexcept
    {
        return y * _width + x;
    }
    template <typename RndIt> constexpr void internal_insert_column(size_type ind, RndIt first, RndIt last)
    {
        static_assert(!isbound);
        static_assert(std::is_same<typename std::iterator_traits<RndIt>::iterator_category,
            std::random_access_iterator_tag>::value);
        assert(width() > 0); // avoid empty matrix
        assert(ind <= width());
        assert(std::distance(first, last) % height() == 0);
        const size_type dist = static_cast<size_type>(std::distance(first, last));
        const size_type hh = height();
        _cont.insert(_cont.end(), dist, value_type {});
        iterator data_end = end() - dist;
        iterator carriage = end();
        size_type trp = dist; // tanspose ind
        bool sc = ind != 0;   // special case for ind==0
        while(carriage != data_end) {
            if(((data_end - begin() - ind) % width() == 0) && sc) {
                size_type dt = --trp;
                while(true) {
                    *--carriage = std::move(first[dt]);
                    if(dt < hh)
                        break;
                    dt -= hh;
                };
            }
            sc = true;
            *--carriage = std::move(*--data_end);
        }
        _width += dist / hh;
    }
    template <bool make_transpose> static constexpr matrix internal_mul(const matrix& l, matrix&& r)
    {
        matrix res(l.height(), l.height());
        if constexpr(make_transpose) {
            r.transpose();
        }
        res.visit([&l, &r](size_type x, size_type y, reference v) {
            v = value_type(0);
            for(size_type i = 0; i != l.width(); ++i)
                v += l(i, y) * r(i, x);
        });
        return res;
    }

public:
    //------------------------------
    //-----CONSTRUCTORS SECTION-----
    //------------------------------
    template <bool b = isbound, std::enable_if_t<b, int> = 0>
    constexpr matrix(size_type width_, size_type height_)
        : _width(width_)
    {
        assert(width_ > 0);
        assert(width_ * height_ == std::size(_cont));
    }
    template <bool b = isbound, std::enable_if_t<b, int> = 0>
    constexpr matrix(size_type width_, size_type height_, const value_type& value)
        : _width(width_)
    {
        assert(width_ > 0);
        assert(width_ * height_ == std::size(_cont));
        std::fill(std::begin(_cont), std::end(_cont), value);
    }

    template <bool b = isbound, std::enable_if_t<!b, int> = 0>
    constexpr matrix(size_type width_, size_type height_, const value_type& value = value_type())
        : _width(width_)
        , _cont(width_ * height_, value)
    {
        assert(width_ > 0 && height_ > 0);
    }
    constexpr matrix(std::initializer_list<std::initializer_list<value_type>> l)
    {
        assert(l.size() > 0);
        assert(l.begin()->size() > 0);
        assert(check_rectangular(l));

        _width = l.begin()->size();
        if constexpr(isbound) {
            assert(size() == l.begin()->size() * l.size());
            visit([&](size_type i, size_type j, reference v) { v = (*((l.begin() + j)->begin() + i)); });
        } else {
            _cont.reserve(l.begin()->size() * l.size());
            for(const auto& row : l)
                _cont.insert(_cont.end(), row.begin(), row.end());
        }
    }
    // copy construcotr is implicit defined by complier
    // assigment operator is implicit defined by complier
    //------------------------------
    //-----EDN OF CONSTRUCTORS------
    //------------------------------
    //------------------------------
    //----ELEMENT ACCESS SECTION----
    //------------------------------
    constexpr reference at(size_type x, size_type y)
    {
        return _cont.at(conv(x, y));
    }
    constexpr const_reference at(size_type x, size_type y) const
    {
        return _cont.at(conv(x, y));
    }
    constexpr reference operator()(size_type x, size_type y)
    {
        return _cont[conv(x, y)];
    }
    constexpr const_reference operator()(size_type x, size_type y) const
    {
        return _cont[conv(x, y)];
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
        return *(end() - 1);
    }
    constexpr const_reference back() const
    {
        return *(cend() - 1);
    }
    constexpr auto data() noexcept
    {
        return std::data(_cont);
    }
    constexpr auto data() const noexcept
    {
        return std::data(_cont);
    }
    //------------------------------
    //----END OF ELEMENT ACCESS-----
    //------------------------------

    //------------------------------
    //------ITERATORS SECTION-------
    //------------------------------
    constexpr iterator begin() noexcept
    {
        return std::begin(_cont);
    }
    constexpr const_iterator begin() const noexcept
    {
        return std::begin(_cont);
    }
    constexpr const_iterator cbegin() const noexcept
    {
        return std::cbegin(_cont);
    }
    constexpr iterator end() noexcept
    {
        return std::end(_cont);
    }
    constexpr const_iterator end() const noexcept
    {
        return std::end(_cont);
    }
    constexpr const_iterator cend() const noexcept
    {
        return std::cend(_cont);
    }
    // reverse
    constexpr reverse_iterator rbegin() noexcept
    {
        return std::rbegin(_cont);
    }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return std::rbegin(_cont);
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return std::crbegin(_cont);
    }
    constexpr reverse_iterator rend() noexcept
    {
        return std::rend(_cont);
    }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return std::rend(_cont);
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return std::crend(_cont);
    }
    // custom
    constexpr iterator_pair<iterator> get_row(size_type ind)
    {
        assert(ind < height());
        auto it = begin() + ind * width();
        return { it, it + width() };
    }
    constexpr iterator_pair<const_iterator> get_row(size_type ind) const
    {
        assert(ind < height());
        auto it = cbegin() + ind * width();
        return { it, it + width() };
    }

    constexpr step_iterator<iterator> get_column(size_type ind)
    {
        assert(ind < width());
        return step_iterator<iterator> { begin() + ind, end() - width() + ind, width() };
    }
    constexpr const_step_iterator<const_iterator> get_column(size_type ind) const
    {
        assert(ind < width());
        return const_step_iterator<const_iterator> { cbegin() + ind, cend() - width() + ind, width() };
    }
    //------------------------------
    //------END OF ITERATORS--------
    //------------------------------

    //------------------------------
    //--------UTILS SECTION---------
    //------------------------------
    container& parent()
    {
        // TODO Unsafe, but we really need it?
        return _cont;
    }

    constexpr size_type width() const noexcept
    {
        return _width;
    }
    constexpr size_type height() const
    {
        assert(_width > 0);
        return size() / _width;
    }

    constexpr size_type size() const
    {
        return std::size(_cont);
    }

    // GROUP: map and visiting
    template <typename BinaryFunc> constexpr void pair_map(const matrix& rhs, BinaryFunc func)
    {
        assert(width() == rhs.width() && height() == rhs.height());
        const_iterator ri = rhs.cbegin();
        for(iterator li = begin(); li != end(); ++li, ++ri)
            func(*li, *ri);
    }
    // mapi=map with iteratpr
    template <typename UnaryFunc> constexpr void mapi(UnaryFunc func)
    {
        for(iterator it = begin(); it != end(); ++it)
            func(it);
    }
    template <typename UnaryFunc> constexpr void mapi(UnaryFunc func) const
    {
        for(const_iterator it = cbegin(); it != cend(); ++it)
            func(it);
    }
    template <typename UnaryFunc> constexpr void map(UnaryFunc func)
    {
        std::for_each(begin(), end(), func);
    }
    template <typename UnaryFunc> constexpr void map(UnaryFunc func) const
    {
        std::for_each(cbegin(), cend(), func);
    }

    /*
     TODO More visiting methods
     constexpr void control_visit(size_type sx, size_type sy, size_type ex, size_type ey, Func func)
     func should return bool for continue or break loop
    */

    template <typename Func> constexpr void visit(Func func)
    {
        size_type x = 0, y = 0;
        iterator it = begin();
        while(y < height()) {
            func(x++, y, *it++);
            if(x == width()) {
                x = 0;
                ++y;
            }
        }
    }

    template <typename Func> constexpr void visit(Func func) const
    {
        size_type x = 0, y = 0;
        const_iterator it = begin();
        while(y < height()) {
            func(x++, y, *it++);
            if(x == width()) {
                x = 0;
                ++y;
            }
        }
    }
    //------------------------------
    //--------END OF UTILS----------
    //------------------------------
    // GROUP: Not mathematical operation
    constexpr matrix& operator+=(const_reference s)
    {
        map([&s](reference v) { v += s; });
        return *this;
    }
    constexpr matrix& operator-=(const_reference s)
    {
        map([&s](reference v) { v -= s; });
        return *this;
    }
    friend constexpr matrix operator+(const matrix& lhs, const_reference rhs)
    {
        return matrix(lhs) += rhs;
    }
    friend constexpr matrix operator-(const matrix& lhs, const_reference rhs)
    {
        return matrix(lhs) -= rhs;
    }

    //------------------------------
    //---MATH OPERATIONS SECTION----
    //------------------------------
    // GROUP: matrix (+) matrix
    constexpr matrix& operator+=(const matrix& rhs)
    {
        pair_map(rhs, [](reference l, const_reference r) { l += r; });
        return *this;
    }
    constexpr matrix& operator-=(const matrix& rhs)
    {
        pair_map(rhs, [](reference l, const_reference r) { l -= r; });
        return *this;
    }
    /*constexpr matrix& operator*=(const matrix& r)
    {
        assert(width() == r.height() && height() == r.width());
        if(width() == height()) {
            for(size_type i = 0; i < width(); ++i) {
                auto r = get_row(i);
                auto c=get_column(i);
                while(c.can_move()){

                }
            }
        }
        for (size_type i=0;i<height();++i){
            auto q=get_row(i)
            get_row(i)*get_column(i);
        }
        return *this;
    }*/
    /* constexpr matrix& operator*=(const matrix& r)
     {
         assert(width() == r.height() && height() == r.width());
         assert((isbound && l.width() == height()) || (!isbound));
         transpose();
         matrix l(height(), height()); //, value_type(0)
         // r.transpose();
         this->visit([&l, &r](size_type x, size_type y, reference v) {
             //v = value_type(0);
             //y fst
             //
             for(size_type i = 0; i != l.width(); ++i)
                 v += l(i, y) * r(x, i); // l(i, y) * r(i,x);
         });
         // pair_map(rhs, [](reference l, const_reference r) { l -= r; });
         // return *this;
     }*/
    friend constexpr matrix operator+(const matrix& lhs, const matrix& rhs)
    {
        return matrix { lhs } += rhs;
    }
    friend constexpr matrix operator-(const matrix& lhs, const matrix& rhs)
    {
        return matrix { lhs } -= rhs;
    }

    friend constexpr matrix operator*(const matrix& l, matrix&& r)
    {
        assert(l.width() == r.height() && l.height() == r.width());
        assert((l.isbound && l.width() == l.height()) || (!l.isbound));
        return internal_mul<true>(l, std::move(r));
    }
    friend constexpr matrix operator*(const matrix& l, const matrix& r)
    {
        assert(l.width() == r.height() && l.height() == r.width());
        assert((l.isbound && l.width() == l.height()) || (!l.isbound));
        return internal_mul<false>(l, r.copy_transpose());
    }
    /*static matrix uncash_mul(const matrix& l, const matrix& r)
    {
        assert(l.width() == r.height() && l.height() == r.width());
        assert((l.isbound && l.width() == l.height()) || (!l.isbound));
        matrix res(l.height(), l.height());
        res.visit([&l, &r](size_type x, size_type y, reference v) {
            v = value_type(0);
            for(size_type i = 0; i != l.width(); ++i)
                v += l(i, y) * r(x, i);
        });
        return res;
    }*/

    // GROUP: matrix (*) scalar
    constexpr matrix& operator*=(const_reference s)
    {
        map([&s](reference v) { v *= s; });
        return *this;
    }
    constexpr matrix& operator/=(const_reference s)
    {
        map([&s](reference v) { v /= s; });
        return *this;
    }
    friend constexpr matrix operator*(const matrix& lhs, const_reference rhs)
    {
        return matrix(lhs) *= rhs;
    }
    friend constexpr matrix operator/(const matrix& lhs, const_reference rhs)
    {
        return matrix(lhs) /= rhs;
    }
    // note: We cannot using spaceship operator because it incorrect working for carrays
    // note2: Also, the comparison for more/less will not work correctly from the "_width" field
    // auto operator<=>(const matrix&) const = default;
    friend constexpr bool operator!=(const matrix& lhs, const matrix& rhs)
    {
        if(lhs.width() != rhs.width() || lhs.height() != rhs.height())
            return true;
        for(const_iterator l = lhs.cbegin(), r = rhs.cbegin(); l != lhs.cend(); ++l, ++r)
            if(*l != *r)
                return true;
        return false;
    }
    friend constexpr bool operator==(const matrix& lhs, const matrix& rhs)
    {
        if(lhs.width() != rhs.width() || lhs.height() != rhs.height())
            return false;
        for(const_iterator l = lhs.cbegin(), r = rhs.cbegin(); l != lhs.cend(); ++l, ++r)
            if(!(*l == *r))
                return false;
        return true;
    }

    // GROUP: other
    constexpr void set_identity(const_reference zero = value_type(0), const_reference one = value_type(1))
    {
        visit([&zero, &one](size_type x, size_type y, reference v) { v = (x == y) ? one : zero; });
    }

    constexpr matrix copy_transpose() const
    {
        matrix<Container> res(height(), width());
        visit([&res](size_type x, size_type y, const_reference v) { res(y, x) = v; });
        return res;
    }

    constexpr void transpose()
    {
        if(width() == height()) {
            for(decltype(width()) i = 0; i != width(); ++i) {
                auto cit = get_row(i).begin + i;
                auto rit = get_column(i) + i;
                while(rit.can_move()) {
                    std::swap(*cit++, *rit++);
                }
            }
        } else {
            const size_type hh = height();
            if constexpr(isbound) {
                Container newc;
                visit([&newc, hh](size_type x, size_type y, reference v) { newc[x * hh + y] = std::move(v); });
                if constexpr(iscarray) {
                    std::move(std::begin(newc), std::end(newc), std::begin(_cont));
                } else
                    _cont = newc;
            } else {
                Container newc(width() * height());
                visit([&newc, hh](size_type x, size_type y, reference v) { newc[x * hh + y] = std::move(v); });
                _cont = newc;
            }
            _width = hh;
        }
    }
    //------------------------------
    //---END OF MATH OPERATIONS-----
    //------------------------------

    //------------------------------
    //--DYNAMIC OPERATIONS SECTION--
    //------------------------------
    // This section is available only for non-static arrays
    constexpr void resize(size_type width_, size_type height_, const value_type& value)
    {
        static_assert(!isbound);
        assert(width_ > 0 && height_ > 0);
        _width = width_;
        _cont.resize(width_ * height_, value);
    }
    constexpr void resize(size_type width_, size_type height_)
    {
        static_assert(!isbound);
        assert(width_ > 0 && height_ > 0);
        _width = width_;
        _cont.resize(width_ * height_);
    }
    template <typename InputIt> constexpr void insert_row(size_type ind, InputIt first, InputIt last)
    {
        static_assert(!isbound);
        assert(width() > 0); // avoid empty matrix
        assert(ind < height());
        assert(std::distance(first, last) % width() == 0);
        _cont.insert(_cont.begin() + ind * width(), first, last);
    }
    constexpr void erase_row(size_type ind, size_type count = 1)
    {
        static_assert(!isbound);
        assert(height() > count); // avoid empty matrix
        assert(ind + count <= height());
        auto it = _cont.begin() + width() * ind;
        _cont.erase(it, it + width() * count);
    }

    template <typename InputIt> constexpr void push_row(InputIt first, InputIt last)
    {
        insert_row(height(), first, last);
    }

    constexpr void pop_row(size_type count = 1)
    {
        erase_row(height() - count, count);
    }

    // section2

    template <typename InputIt> constexpr void insert_column(size_type ind, InputIt first, InputIt last)
    {
        if constexpr(!std::is_same<typename std::iterator_traits<InputIt>::iterator_category,
                         std::random_access_iterator_tag>::value) {
            std::vector<std::iter_value_t<InputIt>> vec { first, last };
            internal_insert_column(ind, vec.begin(), vec.end());
        } else {
            internal_insert_column(ind, first, last);
        }
    }

    constexpr void erase_column(size_type ind, size_type count = 1)
    {
        static_assert(!isbound);
        assert(width() > count); // avoid empty matrix
        assert(ind + count <= width());
        const iterator end_it = end();
        iterator first = begin() + ind;
        for(iterator i = first + count; i != end_it; ++i) {
            if((i - begin() - ind) % width() >= count)
                *first++ = std::move(*i);
        }
        _cont.erase(first, end_it);
        _width -= count;
    }
    template <typename InputIt> constexpr void push_column(InputIt first, InputIt last)
    {
        insert_column(width(), first, last);
    }

    constexpr void pop_column(size_type count = 1)
    {
        erase_column(width() - count, count);
    }
    //------------------------------
    //---END OF DYNAMC OPERATION----
    //------------------------------
};

template <class T> std::ostream& operator<<(std::ostream& os, const matrix<T>& m)
{
    // TODO setw
    m.mapi([&](typename matrix<T>::const_iterator i) {
        os << *i << (((i - m.begin() + 1) % m.width() == 0) ? '\n' : ' ');
    });
    return os;
}

template <class T> std::istream& operator>>(std::istream& is, matrix<T>& m)
{
    using value_t = typename matrix<T>::value_type;
    const bool skipws = is.flags() & std::ios_base::skipws;
    if(!skipws)
        is >> std::skipws;
    const auto unset = [&is, skipws]() {
        if(!skipws)
            is.unsetf(std::ios_base::skipws);
    };

    std::istream_iterator<value_t> ss(is);
    auto it = m.begin();
    while(ss != std::istream_iterator<value_t>()) {
        *it++ = *ss;
        if(it == m.end()) {
            unset();
            return is;
        }
        ++ss;
    }
    unset();
    is.setstate(std::ios::failbit);
    return is;
}
template <class Container, std::size_t N, std::size_t M> class matrix<Container[N][M]>
{
    // Not supported
};

namespace this_program
{
using dmatrix = matrix<std::vector<double>>;
using scalar = typename dmatrix::value_type;
using crmat = const dmatrix&;
using crscalar = typename dmatrix::const_reference;

template <typename Type, typename RestrictFun>
void restri_request(Type& v,
    RestrictFun fun,
    const std::string_view welcome,
    const std::string_view err_str = "Error, please try again\n")
{
    while(!std::cin.eof()) {
        std::cout << welcome;
        std::cin >> v;
        bool ok = true;
        if(std::cin.fail()) {
            std::cout << err_str;
            std::cin.clear();
            ok = false;
        } else {
            ok = fun(v);
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(ok)
            return;
    }
    // if std::cin.eof It might be better to pass the function
    std::exit(0);
}

template <typename Type>
void request_type(Type& v, const std::string_view welcome, const std::string_view err_str = "Error, please try again\n")
{
    restri_request(
        v, [](const Type&) { return true; }, welcome, err_str);
}

dmatrix user_create_matrix()
{
    // using TMat = matrix<T>;
    static_assert(!dmatrix::isbound);
    using sz_t = typename dmatrix::size_type;
    sz_t w, h;
    restri_request(
        w, [](sz_t v) { return v > 0; }, "Enter the width of the matrix: ");
    restri_request(
        h, [](sz_t v) { return v > 0; }, "Enter the height of the matrix: ");
    dmatrix matr(w, h);
    request_type(matr, "Enter the matrix values:\n");
    return matr;
}

namespace menu_option_ns
{
    enum menu_option { sum_matr = 1, sub_matr, mul_matr, transpose, plus_scalar, minus_scalar, help, close };
    constexpr int first_option = sum_matr;
    constexpr int last_option = close;
}

template <typename Unary> void operation1(Unary op, std::string_view msg) // 1-unary
{
    dmatrix m(user_create_matrix());
    std::cout << '\n' << m << msg << op(m);
}
enum class second_kind { scalar, matrix, mulmatrix };
template <second_kind kind, typename Binary> void operation2(Binary op, char c) // 2-binary
{
    dmatrix m1(user_create_matrix());
    if constexpr(kind == second_kind::matrix) {
        std::cout << "Enter the next matrix of the same size(" << m1.width() << 'x' << m1.height() << ")\n";
        dmatrix m2(m1.width(), m1.height());
        request_type(m2, "");
        std::cout << '\n' << m1 << c << '\n' << m2 << "=\n" << op(m1, m2);
    } else if constexpr(kind == second_kind::mulmatrix) {
        std::cout << "Enter the second matrix, with the swap dimensions(" << m1.height() << 'x' << m1.width() << ")\n";
        dmatrix m2(m1.height(), m1.width());
        request_type(m2, "");
        std::cout << '\n' << m1 << c << '\n' << m2 << "=\n" << op(m1, m2);
    } else if constexpr(kind == second_kind::scalar) {
        scalar v;
        request_type(v, "Enter a real value: ");
        std::cout << '\n' << m1 << c << '\n' << v << "\n=\n" << op(m1, v);
    };
}
}

namespace tests
{
constexpr bool constexpr_tests()
{
    constexpr matrix<int[25]> m551 { { -6, 6, 0, 1, -1 }, { 1, -5, -6, -4, -7 }, { 1, -4, -6, 5, -9 },
        { 2, 3, 4, 3, 7 }, { 8, -3, 2, -6, 2 } };
    constexpr matrix<int[25]> m552 { { 6, -6, 9, -4, 4 }, { 5, 0, 1, -9, -1 }, { -7, -2, 8, -3, -9 },
        { 1, 0, -3, 6, 8 }, { 7, -3, 4, 2, 3 } };
    // constexpr matrix<int[9]> m331{{6,2,6},{2,9,-2},{-6,8,5}};
    // constexpr matrix<int[9]> m332{{-2,-5,-3},{-6,9,7},{-4,3,7}};
    constexpr matrix<int[8]> m421 { { -1, -5, -3, -2 }, { -4, 9, -9, 4 } };
    constexpr matrix<int[8]> m422 { { 5, -4, -8, 6 }, { -8, -1, 6, -5 } };

    constexpr matrix<int[4]> m221 { { -9, -2 }, { 3, 1 } };
    constexpr matrix<int[4]> m222 { { 1, 2 }, { 7, -9 } };
    constexpr matrix<int[1]> m111 { { -3 } };
    constexpr matrix<int[1]> m112 { { -9 } };
    // matrix multiplication
    {
        constexpr matrix<int[25]> m55mul { { -12, 39, -55, -26, -25 }, { -30, 27, -60, 21, 10 },
            { -30, 33, -94, 62, 75 }, { 51, -41, 72, -15, 14 }, { 27, -58, 111, -43, -25 } };
        if constexpr(m551 * m552 != m55mul)
            throw;
        constexpr matrix<int[4]> m22mul { { -23, 0 }, { 10, -3 } };
        if constexpr(m221 * m222 != m22mul)
            throw;
        constexpr matrix<int[1]> m11mul { { 27 } };
        if constexpr(m111 * m112 != m11mul)
            throw;
    }
    // matrix subtraction
    {
        constexpr matrix<int[25]> m55sub { { -12, 12, -9, 5, -5 }, { -4, -5, -7, 5, -6 }, { 8, -2, -14, 8, 0 },
            { 1, 3, 7, -3, -1 }, { 1, 0, -2, -8, -1 } };
        if constexpr(m551 - m552 != m55sub)
            throw;
        constexpr matrix<int[8]> m42sub { { -6, -1, 5, -8 }, { 4, 10, -15, 9 } };
        if constexpr(m421 - m422 != m42sub)
            throw;
        constexpr matrix<int[1]> m11sub { { 6 } };
        if constexpr(m111 - m112 != m11sub)
            throw;
    }
    // matrix substraction - scalar
    {
        constexpr int scal = 8;
        constexpr matrix<int[25]> m55ssub { { -14, -2, -8, -7, -9 }, { -7, -13, -14, -12, -15 },
            { -7, -12, -14, -3, -17 }, { -6, -5, -4, -5, -1 }, { 0, -11, -6, -14, -6 } };
        if constexpr(m551 - scal != m55ssub)
            throw;
        // constexpr matrix<int[8]> m421 { { -1, -5, -3, -2 }, { -4, 9, -9, 4 } };
        constexpr matrix<int[8]> m42ssub { { -9, -13, -11, -10 }, { -12, 1, -17, -4 } };
        if constexpr(m421 - scal != m42ssub)
            throw;
        constexpr matrix<int[1]> m11ssub { { -11 } };
        if constexpr(m111 - scal != m11ssub)
            throw;
    }

    // transpose
    {
        constexpr matrix<int[25]> m551T { { -6, 1, 1, 2, 8 }, { 6, -5, -4, 3, -3 }, { 0, -6, -6, 4, 2 },
            { 1, -4, 5, 3, -6 }, { -1, -7, -9, 7, 2 } };
        if constexpr(m551.copy_transpose() != m551T)
            throw;
        constexpr matrix<int[8]> m421T { { -1, -4 }, { -5, 9 }, { -3, -9 }, { -2, 4 } };
        if constexpr(m421.copy_transpose() != m421T)
            throw;
        constexpr matrix<int[4]> m22T { { -9, 3 }, { -2, 1 } };
        if constexpr(m221.copy_transpose() != m22T)
            throw;
        if constexpr(m111.copy_transpose() != m111)
            throw;
        matrix<int[25]> m55I(5, 5);
        m55I.set_identity();
        if(m55I != m55I.copy_transpose())
            throw;
    }
    // operator (w,h) test
    {
        // constexpr matrix<int[8]> m421 { { -1, -5, -3, -2 }, { -4, 9, -9, 4 } };
        if constexpr(m421(0, 0) != -1)
            throw;
        if constexpr(m421(1, 0) != -5)
            throw;
        if constexpr(m421(2, 0) != -3)
            throw;
        if constexpr(m421(1, 1) != 9)
            throw;
        if constexpr(m421(2, 1) != -9)
            throw;
    }
    return true;
}
}

int main()
{
    using namespace this_program;
    using namespace menu_option_ns;
    constexpr bool t = tests::constexpr_tests();
    std::cout << "Welcome to Simplest Matrix Calculator 1.0!\n"
              << sum_matr << " - Addition of two matrices\n"
              << sub_matr << " - Subtracting two matrices\n"
              << mul_matr << " - Matrix multiplication\n"
              << transpose << " - Transposition\n"
              << plus_scalar << " - Increase the matrix by a scalar(Not mathematically)\n"
              << minus_scalar << " - Decrease the matrix by a scalar(Not mathematically)\n"
              << help << " - Get help about of using program\n"
              << close << " - Close the program\n\n";
    while(!std::cin.eof()) {
        int choose;
        request_type(choose, "What would you like to do?\n", "Error, can not recognizable integer number\n");
        switch(choose) {
        case sum_matr:
            operation2<second_kind::matrix>([](crmat l, crmat r) { return l + r; }, '+');
            break;
        case sub_matr:
            operation2<second_kind::matrix>([](crmat l, crmat r) { return l - r; }, '-');
            break;
        case mul_matr:
            operation2<second_kind::mulmatrix>([](crmat l, crmat r) { return l * r; }, '*');
            break;
        case plus_scalar:
            operation2<second_kind::scalar>([](crmat l, crscalar r) { return l + r; }, '+');
            break;
        case minus_scalar:
            operation2<second_kind::scalar>([](crmat l, crscalar r) { return l - r; }, '-');
            break;
        case transpose:
            operation1([](dmatrix& v) { return v.copy_transpose(); }, "After transposing:\n");
            break;
        case help:
            std::cout << "=====Matrix format description=====\n"
                         "In this program, the matrix is defined by a sequence of real numbers\n"
                         "that are separated by spaces or newline characters\n"
                         "In addition, the input matrix is determined by the columns.\n"
                         "For example, a sequence of numbers: '1 2 3 4 5 6 7 8 9' construct the following matrix:\n"
                         "1 2 3\n"
                         "4 5 6\n"
                         "7 8 9\n"
                         "====================================\n"
                         "Auther: PavelPI. 2021.11.29\n";
            break;
        case close:
            std::exit(0);
            break;
        default:
            std::cout << "Unkown choose\n";
            break;
        }
        if(choose >= first_option && choose <= last_option) {
            std::cout << "\nOK. ";
        }
    }
    return 0;
}
