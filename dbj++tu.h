#pragma once
/*
(c) 2019 by dbj.org

Appache 2.0 licence. Please review the licence file in this project
*/

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
};

constexpr inline static auto MSCVER = _MSC_VER;          // 1921
constexpr inline static auto MSCFULLVER = _MSC_FULL_VER; //192127702
constexpr inline static auto MSCBUILD = _MSC_BUILD;      // 2

using tu_function = void (*)();
using units_ = vector<tu_function>;

inline void line() noexcept { printf("\n----------------------------------------------------------------------"); }

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
        line();
        printf("\n " _DBJ_STRINGIZE(MSCVER) " :\t\t %d", MSCVER);
        printf("\n " _DBJ_STRINGIZE(MSCFULLVER) " :\t\t %d", MSCFULLVER);
        printf("\n " _DBJ_STRINGIZE(MSCBUILD) " :\t\t %d", MSCBUILD);
        printf("\n\n Catalogue has %zd test units", units.size());
        line();
    }

    void end() const
    {
        printf("\n\n All tests done.\n\n");
    }

    void execute() const
    {
        unsigned counter_{};
        start();
        for (auto &tu_ : units)
        {
            printf("\n\nTest Unit: %d ", counter_++);
            timer timer_{};
            tu_();
            // line();
            printf("\nDone in: %s", as_buffer(timer_).data());
            line();
        }
        end();
    }
};

constexpr inline testing_system catalog;

#pragma region test macros
/*

I know my core principle is not to use iostreams, but I am not a zealot
I am an pragmatist. For simple and usefull testing one can use iostreams,
like in this macro bellow.

Usage: 

DBJ_TX( 4 + 2 );

TX stands for Test eXpression
*/
#define DBJ_TX(x)                                                                                          \
    do                                                                                                     \
    {                                                                                                      \
        std::cout << std::boolalpha << "\n\nExpression: '" << _DBJ_STRINGIZE(x) << "'\n\tResult: " << (x); \
    } while (0)

#pragma endregion

} // namespace dbj::tu