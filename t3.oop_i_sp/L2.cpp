#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>

//!!!Необходим С++20!!!
//Вариант 6. Журнал, Книга, печатное издание, учебник
//В случае невозможности компиляции, запуск может быть осуществлен онлайн-ресурсом: https://wandbox.org/

template <typename TIterator, typename T> struct parse_result {
    bool ok;
    TIterator it;
    T obj;
};

//------------------------------
//-------UTILS SECTION----------
//------------------------------
namespace utils
{
template <typename Type>
void request_type(Type& v,
    const std::string_view welcome = "",
    const std::string_view err_str = "Error, please try again\n")
{
    while(!std::cin.eof()) {
        std::cout << welcome;
        std::cin >> v;
        bool err = false;
        if(std::cin.fail()) {
            std::cout << err_str;
            std::cin.clear();
            err = true;
        };
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(!err)
            break;
    }
}

template <unsigned Count, typename EnumT, typename T>
constexpr std::array<T, Count> create_by_inds(std::initializer_list<std::pair<EnumT, T>> itms)
{
    std::array<T, Count> res {};
    if(itms.size() != Count)
        throw "Inncorect items count";
    for(const auto& item : itms)
        res[static_cast<typename decltype(res)::size_type>(item.first)] = item.second;
    return res;
}
template <typename TEnum> constexpr auto to_underlying(TEnum e) noexcept
{
    return static_cast<std::underlying_type_t<TEnum>>(e);
}

template <typename TEnum, typename RandomIt>
constexpr std::ostream& write_enum(std::ostream& os, RandomIt first, const RandomIt last, TEnum val)
{
    auto distance = std::distance(first, last);
    const auto ind = to_underlying(val);
    if(ind >= 0 && ind < distance) {
        std::advance(first, to_underlying(val));
        return os << *first;
    }
    return os << ind;
}

template <typename TEnum, typename StorageIt, typename EnumStrIt>
constexpr parse_result<StorageIt, TEnum>
one_char_parse_enum(StorageIt sfirst, const StorageIt slast, EnumStrIt efirst, const EnumStrIt elast)
{
    constexpr TEnum err = static_cast<TEnum>(0);
    if(sfirst == slast) {
        return { false, sfirst, err };
    }

    const EnumStrIt it =
        std::find_if(efirst, elast, [&sfirst](const std::string_view& v) { return v.starts_with(*sfirst); });
    if(it == elast)
        return { false, sfirst, err };

    for(const char s : *it) {
        if(sfirst == slast || *sfirst != s) {
            return { false, sfirst, err };
        }
        ++sfirst;
    }
    return { true, sfirst, static_cast<TEnum>(std::distance(efirst, it)) };
}

template <typename T> std::istream& implement_in_operator(std::istream& is, T& v)
{
    auto pres = T::parse(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
    if(!pres.ok)
        is.setstate(std::ios::failbit);
    v = pres.obj;
    return is;
}

constexpr bool isdigit(const char c)
{
    return (c >= '0' && c <= '9');
    // return std::isdigit(static_cast<unsigned char>(ch));
}
}
//------------------------------
//------  END OF UTILS  --------
//------------------------------

//------------------------------
//------DATETIME SECTION--------
//------------------------------
struct datetime {
    // Initially, there was a type std::chrono::time_point<std::chrono::system>,
    // but it is not suitable for storing old dates, it matters if we are talking about books
    constexpr static char date_delim = '-';
    constexpr static char datetime_delim = ' ';
    constexpr static char time_delim = ':';
    constexpr static unsigned total_width = 19; // 19=year(4)+D+mon(2)+D+day(2)+D+h(2)+D+m(2)+D+s(2)

    unsigned year = 1, mon = 1, day = 1, hour = 0, min = 0, sec = 0;
    // should be character iterator
    template <typename InputIt>
    constexpr static parse_result<InputIt, datetime> parse(InputIt first, const InputIt last)
    {
#define TRY_PEEK_DIG(dig_min, dig_max, out_dig)                                \
    {                                                                          \
        if((first == last) || (*first < (dig_min)) || (*first > (dig_max)))    \
            return { false, first, datetime { yyyy, mon, dd, hh, min, sec } }; \
        out_dig = *first - '0';                                                \
        ++first;                                                               \
    }

#define MAYBE_RETURN(dest_char)                                               \
    {                                                                         \
        if((first == last) || (*first != (dest_char))) {                      \
            return { true, first, datetime { yyyy, mon, dd, hh, min, sec } }; \
        }                                                                     \
        ++first;                                                              \
    }

        unsigned yyyy = 1, mon = 1, dd = 1, hh = 0, min = 0, sec = 0, o = 0;
        // year
        TRY_PEEK_DIG('0', '9', o)
        yyyy = o * 1000;
        TRY_PEEK_DIG('0', '9', o)
        yyyy += o * 100;
        TRY_PEEK_DIG('0', '9', o)
        yyyy += o * 10;
        TRY_PEEK_DIG(yyyy == 0 ? '1' : '0', '9', o)
        yyyy += o;
        MAYBE_RETURN(date_delim)
        // month
        TRY_PEEK_DIG('0', '1', o)
        mon = o * 10;
        TRY_PEEK_DIG(o != 0 ? '0' : '1', o == 1 ? '2' : '9', o)
        mon += o;
        MAYBE_RETURN(date_delim)
        // day
        TRY_PEEK_DIG('0', '3', o)
        dd = o * 10;
        TRY_PEEK_DIG(o != 0 ? '0' : '1', o == 3 ? '1' : '9', o)
        dd += o;
        MAYBE_RETURN(datetime_delim)
        // hours
        TRY_PEEK_DIG('0', '2', o)
        hh = o * 10;
        TRY_PEEK_DIG('0', o == 2 ? '3' : '9', o)
        hh += o;
        MAYBE_RETURN(time_delim)
        // minutes
        TRY_PEEK_DIG('0', '5', o)
        min = o * 10;
        TRY_PEEK_DIG('0', '9', o)
        min += o;
        MAYBE_RETURN(time_delim)
        // seconds
        TRY_PEEK_DIG('0', '6', o)
        sec = o * 10;
        TRY_PEEK_DIG('0', o == 6 ? '0' : '9', o)
        sec += o;
        return { true, first, datetime { yyyy, mon, dd, hh, min, sec } };
#undef TRY_PEEK_DIG
#undef MAYBE_RETURN
    }
    constexpr static std::array<char, total_width + 1 /*null term*/> format(const datetime& dt)
    {
        std::array<char, total_width + 1> res {};
        auto it = std::begin(res);

        *(it++) = (dt.year / 1000) % 10 + '0';
        *(it++) = (dt.year / 100) % 10 + '0';
        *(it++) = (dt.year / 10) % 10 + '0';
        *(it++) = (dt.year) % 10 + '0';

        char delim = date_delim;
        std::array<unsigned, 5> flds { dt.mon, dt.day, dt.hour, dt.min, dt.sec };

        for(decltype(flds.size()) i = 0; i != flds.size(); i++) {
            *(it++) = delim;
            const int v = flds[i];
            if(v < 10) {
                *(it++) = '0';
                *(it++) = v + '0';
            } else {
                *(it++) = (v / 10) % 10 + '0';
                *(it++) = v % 10 + '0';
            }
            if(i == 1)
                delim = datetime_delim;
            else if(i > 1)
                delim = time_delim;
        }
        return res;
    }
};

std::istream& operator>>(std::istream& is, datetime& v)
{
    return utils::implement_in_operator(is, v);
}
std::ostream& operator<<(std::ostream& os, const datetime& v)
{
    const auto chars = datetime::format(v);
    return os.write(chars.data(), chars.size() - 1 /*null tem*/);
}
//------------------------------
//------ END OF DATETIME -------
//------------------------------

//------------------------------
//------ISBN13_T SECTION--------
//------------------------------
struct isbn13_t {
    static constexpr char delim = '-';
    static constexpr int fields = 5;
    static constexpr int digits = 13;
    static constexpr int total_width = fields + digits - 1;
    std::array<char, total_width + 1> isbn; // include null term

    template <typename InputIt>
    constexpr static parse_result<InputIt, isbn13_t> parse(InputIt first, const InputIt last)
    {
        isbn13_t res {};
        std::uint_fast32_t value = 0;
        unsigned index = 0;
        bool prev_is_delim = true;
        int target_digs = digits;
        auto it = std::begin(res.isbn);
        while(target_digs > 0) {
            if(first == last) {
                return { false, first, res };
            }
            const char c = *first;
            const bool isdig = utils::isdigit(c);
            const bool isdelim = c == delim;
            if(isdig) {
                // isbn is 13 digits and 5 fields. The each field can not be greate 10^(13-5)-1
                if(value > 100000000 - 1)
                    return { false, first, res };

                value = value * 10 + (c - '0');
                *(it++) = c;
                prev_is_delim = false;
                --target_digs;
            } else {
                if(isdelim && !prev_is_delim) {
                    if(index == fields - 1) // Special case, when five fields were read but 13 digits were not read
                        return { false, first, res };

                    ++index;
                    *(it++) = c;
                    value = 0;
                    prev_is_delim = true;
                } else {
                    return { false, first, res };
                }
            }
            ++first;
        }

        if(index != fields - 1)
            return { false, first, res };
        return { true, first, res };
    }
};

std::istream& operator>>(std::istream& is, isbn13_t& v)
{
    return utils::implement_in_operator(is, v);
}

std::ostream& operator<<(std::ostream& os, const isbn13_t& v)
{
    return os.write(v.isbn.data(), v.isbn.size() - 1 /*null tem*/);
};
//------------------------------
//------   END OF ISBN   -------
//------------------------------

//------------------------------
//-------ISSN8_T SECTION--------
//------------------------------
struct issn8_t {
    static constexpr char delim = '-';
    static constexpr char ten_char = 'X';
    static constexpr char ten_char2 = 'x';
    static constexpr int total_width = 9; // including check digit
    static constexpr int delim_pos = 4;
    std::array<char, total_width + 1> issn; // +1 nullterm
    template <typename InputIt> constexpr static parse_result<InputIt, issn8_t> parse(InputIt first, const InputIt last)
    {
        issn8_t res {};
        auto it = std::begin(res.issn);
        for(int i = 0; i < total_width; ++i, ++first) {
            if(first == last)
                return { false, first, res };

            const char c = *first;
            if(i == delim_pos) {
                if(c != delim)
                    return { false, first, res };
            } else {
                if(!utils::isdigit(c) && (i != total_width - 1 || (c != ten_char && c != ten_char2)))
                    return { false, first, res };
            }
            *(it++) = c;
        }
        if(*(--it) == ten_char2)
            *it = ten_char;
        return { true, first, res };
    }
};
std::istream& operator>>(std::istream& is, issn8_t& v)
{
    return utils::implement_in_operator(is, v);
}

std::ostream& operator<<(std::ostream& os, issn8_t v)
{
    return os.write(v.issn.data(), v.issn.size() - 1 /*null tem*/);
};
//------------------------------
//-------- END OF ISSN ---------
//------------------------------

//------------------------------
//-----PERIODICITY SECTION------
//------------------------------
enum class periodicity { week = 0, quarter, year };
constexpr auto periodicity_str = utils::create_by_inds<3, periodicity, std::string_view>({ //
    { periodicity::week, "week" },
    //
    { periodicity::quarter, "quarter" },
    //
    { periodicity::year, "year" }

});

std::istream& operator>>(std::istream& is, periodicity& v)
{
    auto pres = utils::one_char_parse_enum<periodicity>(std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>(), periodicity_str.cbegin(), periodicity_str.cend());
    if(!pres.ok)
        is.setstate(std::ios::failbit);
    v = pres.obj;
    return is;

    // return utils::one_char_read_enum(is, v, periodicity_str.cbegin(), periodicity_str.cend());
}
std::ostream& operator<<(std::ostream& os, periodicity v)
{
    return utils::write_enum(os, periodicity_str.cbegin(), periodicity_str.cend(), v);
}
//------------------------------
//------END OF PERIODICITY------
//------------------------------

//------------------------------
//---PRINTED_PRODUCT SECTION----
//------------------------------
class printed_product
{
public:
    static std::deque<std::unique_ptr<printed_product>> memory;
    static void print_memory()
    {
        if (memory.cbegin()==memory.cend()) return;
        auto it=memory.cbegin();
        it->get()->interactive_print();
        
        for(const auto& item : memory) {
            std::cout<<"\n\n";
            item->interactive_print();
        }
        std::cout<<"\n";
    }
    template <typename T> static void memorise(const T& v)
    {
        memory.push_back(std::make_unique<T>(v));
    }
    template <typename T> static void memorise(std::unique_ptr<T> ptr)
    {
        memory.push_back(std::move(ptr));
    }

protected:
    template <typename TFld, bool First = false>
    static inline std::ostream& concat_fields(std::ostream& os, const std::string_view name, const TFld& val)
    {
        /*if constexpr(!First)
            os << ", ";
        return os << name << ": " << val;*/
        if constexpr(!First)
            os << "\n";
        return os << name << ": " << val;
    }

    virtual std::string_view class_name() const
    {
        return "Printed prodct";
    }

    virtual std::ostream& print_fields(std::ostream& os) const
    {
        concat_fields<decltype(title), true>(os, "Title", title);
        concat_fields(os, "Authers", authers);
        concat_fields(os, "Pages", pages);
        return concat_fields(os, "Print date", print_date);
        // return os << "Title: " << title << ", Authers: " << authers << ", Pages: " << pages
        //         << ", Print Date: " << print_date;
    }

public:
    std::string title;
    std::string authers;
    std::uintmax_t pages;
    datetime print_date;

    printed_product() = default;

    printed_product(const std::string& title_, const std::string& authers_, std::uintmax_t pages_, datetime print_date_)
        : title(title_)
        , authers(authers_)
        , pages(pages_)
        , print_date(print_date_)
    {
    }
    // copy construcotr is implicit defined by complier
    // destructor is implicit defined by complier
    // assigment operator is implicit defined by complier

    std::ostream& interactive_print() const
    {
        return print_fields(std::cout << class_name() << "=[") << "]";
    }

    virtual void interactive_read()
    {
        utils::request_type(title, "Title: ");
        utils::request_type(authers, "Authers: ");
        utils::request_type(pages, "Pages: ");
        utils::request_type(print_date, "Print Date: ");
    }

    virtual ~printed_product()
    {
    }
};

std::deque<std::unique_ptr<printed_product>> printed_product::memory;
//------------------------------
//----END OF PRINTED_PRODUCT----
//------------------------------

//------------------------------
//---------BOOK SECTION---------
//------------------------------
class book : public printed_product
{
protected:
    using base_t = printed_product;
    std::string_view class_name() const override
    {
        return "Book";
    }
    std::ostream& print_fields(std::ostream& os) const override
    {
        base_t::print_fields(os);
        concat_fields(os, "ISBN", isbn13);
        return concat_fields(os, "Circulation", circulation);
        // return base_t::print_fields(os) << ", ISBN: " << isbn13 << ", Circulation: " << circulation;
    }

public:
    isbn13_t isbn13;
    std::uintmax_t circulation;

    book() = default;
    book(const std::string& title_,
        const std::string& authers_,
        std::uintmax_t pages_,
        datetime print_date_,
        //
        const isbn13_t& isbn13_,
        std::uintmax_t circulation_)
        : printed_product(title_, authers_, pages_, print_date_)
        , isbn13(isbn13_)
        , circulation(circulation_)
    {
    }

    void interactive_read() override
    {
        base_t::interactive_read();
        utils::request_type(isbn13, "ISBN: ");
        utils::request_type(circulation, "Circulation: ");
    }
};
//------------------------------
//---------END OF BOOK----------
//------------------------------

//------------------------------
//------TEXT_BOOK SECTION-------
//------------------------------
class textbook : public book
{
protected:
    using base_t = book;
    std::string_view class_name() const override
    {
        return "Textbook";
    }

    std::ostream& print_fields(std::ostream& os) const override
    {
        base_t::print_fields(os);
        concat_fields(os, "Subject", subject);
        concat_fields(os, "Teacher required", teacher_requires);
        return os;
        // return base_t::print_fields(os) << ", Subject: " << subject << ", Teacher required: " << teacher_requires;
    }

public:
    std::string subject;
    bool teacher_requires;

    textbook() = default;
    textbook(const std::string& title_,
        const std::string& authers_,
        std::uintmax_t pages_,
        datetime print_date_,
        //
        const isbn13_t& isbn13_,
        std::uintmax_t circulation_,
        //
        const std::string& subject_,
        bool teacehr_requires_)
        : book(title_, authers_, pages_, print_date_, isbn13_, circulation_)
        , subject(subject_)
        , teacher_requires(teacehr_requires_)
    {
    }
    void interactive_read() override
    {
        base_t::interactive_read();
        utils::request_type(subject, "Subject: ");
        utils::request_type(teacher_requires, "Teacher requires: ");
    }
};
//------------------------------
//------END OF TEXT_BOOK--------
//------------------------------

//------------------------------
//------MAGAZINE SECTION--------
//------------------------------
class magazine : public printed_product
{
protected:
    using base_t = printed_product;
    std::string_view class_name() const override
    {
        return "Magazine";
    }
    std::ostream& print_fields(std::ostream& os) const override
    {
        base_t::print_fields(os);
        concat_fields(os, "ISSN", issn8);
        return concat_fields(os, "Periodicity", period);
        // return base_t::print_fields(os) << ", ISSN: " << issn8 << ", Periodicity: " << period;
    }

public:
    issn8_t issn8;
    periodicity period;

    magazine() = default;
    magazine(const std::string& title_,
        const std::string& authers_,
        std::uintmax_t pages_,
        datetime print_date_,
        //
        issn8_t issn8_,
        periodicity period_)
        : printed_product(title_, authers_, pages_, print_date_)
        , issn8(issn8_)
        , period(period_)
    {
    }

    void interactive_read() override
    {
        base_t::interactive_read();
        utils::request_type(issn8, "ISSN: ");
        utils::request_type(period, "Periodicity: ");
    }
};
//------------------------------
//-------END OF MAGAZINE--------
//------------------------------

namespace menu_option_ns
{
enum menu_option { add_book = 1, add_textbook, add_magazine, add_printed, walk_library, help, close };
constexpr int first_option = add_book;
constexpr int last_option = close;
}

template <typename T> void memorise_read()
{
    // Pointers are needed to obtain polymorphism
    auto ptr = std::make_unique<T>();
    ptr->interactive_read();
    T::memorise(std::move(ptr));
};

int main()
{
// Examples
//#ifndef NDEBUG
    printed_product::memorise(book { "The Master and Margarita", "Mikhail Bulgakov", 480, datetime { 2012 },
        isbn13_t { "978-5-389-01686-6" }, 10000 });
    printed_product::memorise(book { "A brief history of time", "Stephen Hawking", 100, datetime { 2010 },
        isbn13_t { "978-5-367-00490-8" }, 14000 });
    printed_product::memorise(textbook { "Example3", "Example3.Auther", 100, datetime { 1999 },
        isbn13_t { "1234-5678-90-12-3" }, 9000, "Complex Analysis", false });
    printed_product::memorise(magazine { "Ex4. Complex numbers and quaternions", "Ex4.Auther", 60,
        datetime { 2019, 8, 28 }, issn8_t { "7654-321X" }, periodicity::quarter });
    printed_product::memorise(
        printed_product { "Brochure. Taylor Series+arctan=PI", "Auther278", 20, datetime { 2021, 2, 19, 12, 48, 3 } });
//#endif
    using namespace menu_option_ns;
    std::cout << "Welcome to Digital Library 1.0!\n"
              << add_book << " - Add book\n"
              << add_textbook << " - Add textbook\n"
              << add_magazine << " - Add magazine\n"
              << add_printed << " - Add other printed product\n"
              << walk_library << " - Walk to library\n"
              << help << " - Get help about of using program\n"
              << close << " - Close the program\n\n";
    while(!std::cin.eof()) {
        int choose;
        utils::request_type(choose, "What would you like to do?\n", "Error, can not recognizable integer number\n");
        switch(choose) {
        case add_printed:
            memorise_read<printed_product>();
            break;
        case add_book:
            memorise_read<book>();
            break;
        case add_textbook:
            memorise_read<textbook>();
            break;
        case add_magazine:
            memorise_read<magazine>();
            break;
        case walk_library:
            printed_product::print_memory();
            break;
        case help:
            std::cout
                << "=====List of format description=====\n"
                   "ISBN is a book identifier that contains 13 digits and five fields that are separated by '-'.\n"
                   "For example 978-5-389-01686-6.\n\n"
                   //
                   "ISSN is a magazine identifier that contains 8 digits and two fields that are separated by '-'.\n"
                   "The last digit of the second field is a check digit and may include the ten value it is\n"
                   "represented by the symbol 'X'(or 'x').\n"
                   "For example 0025-570X. \n\n"
                   //
                   "The field named 'Teacher requires' is relevant for textbooks and is represented by a logical "
                   "flag.\n"
                   "In this version, '1' or '0' is used for input and output.\n\n"
                   //
                   "The periodicity is available for magazines only.\n"
                   "For periodicity, the following values are allowed: 'week', 'quarter', 'year'\n\n"
                   //
                   "The date and time is expected in the format yyyy-mm-dd hh:mm:ss\n"
                   "Components of lesser importance may be missing.\n"
                   "Examples of correct dates: '2002-04-03 15:17', '1986-02', '1986'\n"
                   "====================================\n"
                   "Auther: PavelPI. 2021.11.21\n";
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
}

/*//(thirteen-digit ISBN, foramt X-X-X-X-X):
void tests()
{
    {
        constexpr std::string_view s { "yearabc" };
        constexpr auto r = utils::one_char_parse_enum<periodicity>(
            s.cbegin(), s.cend(), periodicity_str.cbegin(), periodicity_str.cend());
        if(r.ok && r.it == s.cbegin() + 4 && r.obj == periodicity::year) {
            std::cout << "Test1 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "yearquarter" };
        constexpr auto r1 = utils::one_char_parse_enum<periodicity>(
            s.cbegin(), s.cend(), periodicity_str.cbegin(), periodicity_str.cend());
        if(r1.ok && r1.it == s.cbegin() + 4 && r1.obj == periodicity::year) {
            std::cout << "Test2 Passed\n";
        }
        constexpr auto r2 =
            utils::one_char_parse_enum<periodicity>(r1.it, s.cend(), periodicity_str.cbegin(), periodicity_str.cend());
        if(r2.ok && r2.it == s.cend() && r2.obj == periodicity::quarter) {
            std::cout << "Test3 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "1990-0987-98-78-900000" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.cbegin() + 17) {
            std::cout << "Test4 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "1990-0987-98--8-900000" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.cbegin() + 17) {
            std::cout << "Test4 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "1234-5678-90-1-2-.-900000" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(!r.ok && r.it == s.cbegin() + 16) {
            std::cout << "Test5 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "978-3-16-148410-0" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.cend()) {
            std::cout << "Test6 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "1-1-1-1-1" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(!r.ok && r.it == s.cend()) {
            std::cout << "Test7 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "123-512-1234567" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(!r.ok && r.it == s.cend()) {
            std::cout << "Test8 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "123-512-1234567-0-0" };
        auto r = isbn13_t::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.begin() + 14) {
            std::cout << "Test9 Passed\n";
        }
    }
    {
        constexpr std::string_view s { "1999-01.." };
        auto r = datetime::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.begin() + 7) {
            std::cout << "Test10 Passed\n";
        }
    }
        {
        constexpr std::string_view s { "1999-011x123" };
        auto r = issn8_t::parse(s.cbegin(), s.cend());
        if(r.ok && r.it == s.begin() + 9) {
            std::cout << "Test11 Passed\n";
        }
    }
    //       auto beg = std::istreambuf_iterator<char>(std::cin);
    // auto end = std::istreambuf_iterator<char>();
}
*/
/*template <typename TEnum, typename StorageIt> std::string format_enum(StorageIt first, StorageIt last, TEnum v)
{
    auto distance = std::distance(first, last);
    const auto ind = utils::to_underlying(v);
    if(ind >= 0 && ind < distance) {
        std::advance(first, to_underlying(v));
        return *first;
    }
    return std::to_string(ind);
}*/