#include <array>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>
//!!!Необходим С++20!!!
//Вариант 6. Структура "Животное"
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
template <typename T> std::istream& implement_in_operator(std::istream& is, T& v)
{
    auto pres = T::parse(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
    if(!pres.ok)
        is.setstate(std::ios::failbit);
    v = pres.obj;
    return is;
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
template <typename TEnum, typename InputIt>
std::istream& implement_enum_in_operator(std::istream& is, TEnum& v, InputIt first, InputIt last)
{
    auto pres =
        one_char_parse_enum<TEnum>(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), first, last);
    if(!pres.ok)
        is.setstate(std::ios::failbit);
    v = pres.obj;
    return is;
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
//--REPRODUCTION_KIND SECTION---
//------------------------------
enum class reprod_kind { male = 0, feemale, asexual };
constexpr auto reprod_kind_str = utils::create_by_inds<3, reprod_kind, std::string_view>({ //
    { reprod_kind::male, "male" },
    //
    { reprod_kind::feemale, "feemale" },
    //
    { reprod_kind::asexual, "asexual" } });

std::istream& operator>>(std::istream& is, reprod_kind& v)
{
    return utils::implement_enum_in_operator(is, v, reprod_kind_str.cbegin(), reprod_kind_str.cend());
}
std::ostream& operator<<(std::ostream& os, reprod_kind v)
{
    return utils::write_enum(os, reprod_kind_str.cbegin(), reprod_kind_str.cend(), v);
}
//------------------------------
//-- END OF REPRODUCTION_KIND --
//------------------------------

//------------------------------
//-----MASS UNITS SECTION-------
//------------------------------
enum class mass_units2 { gr = 0, kg, cn, tn };
constexpr auto mass_units2_str = utils::create_by_inds<4, mass_units2, std::string_view>({
    //
    { mass_units2::gr, "gr" },
    //
    { mass_units2::kg, "kg" },
    //
    { mass_units2::cn, "cn" },
    //
    { mass_units2::tn, "tn" },
});

std::istream& operator>>(std::istream& is, mass_units2& v)
{
    return utils::implement_enum_in_operator(is, v, mass_units2_str.cbegin(), mass_units2_str.cend());
}
std::ostream& operator<<(std::ostream& os, mass_units2 v)
{
    return utils::write_enum(os, mass_units2_str.cbegin(), mass_units2_str.cend(), v);
}
//------------------------------
//-----END OF MASS UNITS--------
//------------------------------

struct mass_t {
    std::uintmax_t count;
    mass_units2 units;
};

std::istream& operator>>(std::istream& is, mass_t& v)
{
    return is >> v.count >> v.units;
}

std::ostream& operator<<(std::ostream& os, mass_t v)
{
    return os << v.count << v.units;
}

constexpr auto operator"" _gr(unsigned long long v)
{
    return mass_t { v, mass_units2::gr };
}
constexpr auto operator"" _kg(unsigned long long v)
{
    return mass_t { v, mass_units2::kg };
}
constexpr auto operator"" _cn(unsigned long long v)
{
    return mass_t { v, mass_units2::cn };
}
constexpr auto operator"" _tn(unsigned long long v)
{
    return mass_t { v, mass_units2::tn };
}

struct animal {
    std::string species;
    reprod_kind reprod;
    mass_t mass;
    int health_mark;
    std::string health_comment;
    datetime birth_date;

    animal() = default;

    animal(const std::string& species_,
        reprod_kind reprod_,
        mass_t mass_,
        int health_mark_,
        const std::string& health_comment_,
        datetime birth_date_)
        : species(species_)
        , reprod(reprod_)
        , mass(mass_)
        , health_mark(health_mark_)
        , health_comment(health_comment_)
        , birth_date(birth_date_)

    {
    }
    animal(const animal&) = default;

    std::ostream& interactive_print() const
    {
        std::cout << "Species: " << species << '\n'
                  << "Reproduction: " << reprod << '\n'
                  << "Mass: " << mass << '\n'
                  << "Health mark: " << health_mark << '\n'
                  << "Health comment: " << health_comment << '\n'
                  << "Birh date: " << birth_date;
        return std::cout;
    }

    void interactive_read()
    {
        utils::request_type(species, "Species: ");
        utils::request_type(reprod, "Reproduction: ");
        utils::request_type(mass, "Mass: ");
        utils::request_type(health_mark, "Health mark: ");
        utils::request_type(health_comment, "Health comment: ");
        utils::request_type(birth_date, "Birth date: ");
    }

    virtual ~animal() = default;
};

namespace menu_option_ns
{
enum menu_option { add_animal = 1, view_animals, help, close };
constexpr int first_option = add_animal;
constexpr int last_option = close;
}

int main(int argc, char** argv)
{
    using namespace menu_option_ns;
    std::vector<std::unique_ptr<animal>> animals;
    // animals.push_back(animal { "Lian", reprod_kind::male, 10_kg, 10, "GOOD", datetime { 1999 } });
    const auto push = [&animals](const animal& an) { animals.push_back(std::make_unique<animal>(an)); };
    //#ifndef NDEBUG
    push(animal { "Lian", reprod_kind::male, 115_kg, 8, "Very good", datetime { 2016 } });
    push(animal { "Polar bear", reprod_kind::feemale, 610_kg, 7, "Good", datetime { 2003,9 } });
    //#endif
    std::cout << "Welcome to Zoo 1.0!\n"
              //
              << add_animal << " - Add animal\n"
              << view_animals << " - View animals\n"
              << help << " - Get help about of using program\n"
              << close << " - Close the program\n\n";

    while(!std::cin.eof()) {
        int choose;
        utils::request_type(choose, "What would you like to do?\n", "Error, can not recognizable integer number\n");
        switch(choose) {
        case add_animal:
            animals.push_back(std::make_unique<animal>());
            animals.back()->interactive_read();
            break;
        case view_animals:
            for(const auto& an : animals)
                an->interactive_print() << "\n\n";
            break;
        case help:
            std::cout << "=====List of format description=====\n"
                         "The field named 'Reproduction' is an enumeration,\n"
                         "and serves to store one of the values: male, feemale, asexual\n"
                         //
                         "The 'mass' type is intended for recording the mass of an animal, in the form: numUnits.\n"
                         "'num' - is any natural number and 'Units' should be [gr, kh, cn, tn]. Example: 10kg\n"
                         //
                         "The date and time is expected in the format yyyy-mm-dd hh:mm:ss\n"
                         "Components of lesser importance may be missing.\n"
                         "Examples of correct dates: '2002-04-03 15:17', '1986-02', '1986'\n"
                         "====================================\n"
                         "Auther: PavelPI. 2021.11.18\n";
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
    /*std::cout << "Task option 6. Structure of animals. Usivich P.I. 20-ITZs\n";
    std::cout << "List of actions:\nInput 1 for add animal\nInput 2 for view animals\nInput 3 for exit\n\n";
    int choose;
    while(!std::cin.eof()) {
        std::cin >> choose; // TODO correct num only
        switch(choose) {
        case 1:
            animals.push_back(animal::read_by_user());
            break;
        case 2:
            std::for_each(animals.cbegin(), animals.cend(), [](const animal& a) { std::cout << a << std::endl; });
            break;
        case 3:
            std::exit(0);
            break;
        default:
            std::cout << "Inncorrect choose, please try again\n";
        }
        std::cout << "OK\n\n";
    }
    return 0;*/
}
