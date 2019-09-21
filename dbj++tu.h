#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#ifndef DBJ_TU_INCLUDED
#define DBJ_TU_INCLUDED

#include "dbj++nanolib.h"

/*
testing nano-lib

Usage is with this single macro bellow.
Testing unit is a lambda with no arguments.

TU_REGISTER(
    []{ / * testing code goes here * / }
);

void main() {
    // execute all registered tests
    dbj::tu::catalog.execute();
}

*/

#define TU_REGISTER inline auto _DBJ_CONCATENATE(dumsy_, __COUNTER__) = ::dbj::tu::catalog

namespace dbj::tu
{

using namespace std;

struct timer final
{
    using buffer = array<char, 24>;
    using timepoint = typename std::chrono::system_clock::time_point;
    const timepoint start_ = std::chrono::system_clock::now();

    double nano() const
    {
        timepoint end_ = std::chrono::system_clock::now();
        return static_cast<double>((end_ - start_).count());
    }

    double micro() const { return nano() / 1000.0; }

    double seconds() const { return micro() / 1000.0; }

    double decimal3(double arg) { return (std::round(arg * 1000)) / 1000; }

    enum class kind
    {
        nano,
        micro,
        second
    };

    friend buffer as_buffer(timer const &timer_, kind which_ = kind::micro)
    {
        buffer retval{char{0}};
        double arg{};
        char const *unit_{};
        switch (which_)
        {
        case kind::nano:
            arg = timer_.nano();
            unit_ = " nano seconds ";
            break;
        case kind::micro:
            arg = timer_.micro();
            unit_ = " micro seconds ";
            break;
        default: //seconds
            arg = timer_.seconds();
            unit_ = " seconds ";
        }
        std::snprintf(retval.data(), retval.size(), "%.3f%s", arg, unit_);
        return retval;
    }
}; // timer

constexpr inline static auto MSCVER = _MSC_VER;          // 1921
constexpr inline static auto MSCFULLVER = _MSC_FULL_VER; //192127702
constexpr inline static auto MSCBUILD = _MSC_BUILD;      // 2

using tu_function = void (*)();
using units_ = vector<tu_function>;

inline void line() noexcept { DBJ_PRINT("\n----------------------------------------------------------------------"); }

struct testing_system final
{
    inline static units_ units{};

    auto operator()(tu_function fun_) const
    {
        units.push_back(fun_);
        return fun_;
    }

    void start() const
    {
        DBJ_PRINT(DBJ_FG_CYAN);
        line();
        DBJ_PRINT("\n " _DBJ_STRINGIZE(MSCVER) " :\t\t %d", MSCVER);
        DBJ_PRINT("\n " _DBJ_STRINGIZE(MSCFULLVER) " :\t\t %d", MSCFULLVER);
        DBJ_PRINT("\n " _DBJ_STRINGIZE(MSCBUILD) " :\t\t %d", MSCBUILD);
        DBJ_PRINT("\n\n Catalogue has %zd test units", units.size());
        line();
        DBJ_PRINT(DBJ_RESET);
    }

    void end() const
    {
        DBJ_PRINT(DBJ_FG_CYAN "\n\n All tests done.\n\n" DBJ_RESET);
    }

    void execute() const
    {
        unsigned counter_{};
        start();
        for (auto &tu_ : units)
        {
            DBJ_PRINT(DBJ_FG_CYAN);
            DBJ_PRINT("\n\nTest Unit: %d ", counter_++);
            timer timer_{};
            DBJ_PRINT(DBJ_RESET);

            tu_();
            // line();
            DBJ_PRINT(DBJ_FG_CYAN);
            DBJ_PRINT("\nDone in: %s", as_buffer(timer_).data());
            line();
            DBJ_PRINT(DBJ_RESET);
        }
        end();
    }
}; // testing system

constexpr inline testing_system catalog;

#pragma region test macros
/*
TX stands for Test eXpression
I know my core principle is not to use iostreams, but I am not a zealot
I am an pragmatist. For simple and usefull test displays one can use iostreams,
like in this macro bellow.

Usage: 

DBJ_TX( 4 + 2 );
*/
#define DBJ_TX(x)                                                                                                                              \
    do                                                                                                                                         \
    {                                                                                                                                          \
        std::cout << std::boolalpha << "\n\nExpression: '" << #x << "'\n\tResult: " << (x) << "\n\tIt's type: " << typeid(x).name() << "\n\n"; \
    } while (0)

#pragma endregion

} // namespace dbj::tu

#ifdef DBJ_TX

#include <iostream>

/*
primary runtime buffer is vector of a char_type
primary compile time buffer is array of a char_type

thus I will put required operators in here
*/

inline std::ostream &operator<<(std::ostream &os_, std::vector<char> buff_)
{
    return os_ << buff_.data();
}

inline std::ostream &operator<<(std::ostream &os_, std::vector<wchar_t> buff_)
{
    return os_ << buff_.data();
}

template <size_t N>
inline std::ostream &operator<<(std::ostream &os_, std::array<char, N> buff_)
{
    return os_ << buff_.data();
}

template <size_t N>
inline std::ostream &operator<<(std::ostream &os_, std::array<wchar_t, N> buff_)
{
    return os_ << buff_.data();
}
/*
I use very often std::pair
*/
template <typename T1, typename T2>
inline std::ostream &operator<<(std::ostream &os_, std::pair<T1, T2> pair_)
{
    return os_ << "{ " << pair_.first << " , " << pair_.second << " }";
}

#endif

#endif // DBJ_TU_INCLUDED