#pragma once
#ifndef DBJ_SEQUENCE_PRINT_INC
#define DBJ_SEQUENCE_PRINT_INC

/// #include <iostream>
#include <array>
#include <cassert>

// (c) 2019/2020 by dbj@dbj.org
// Licence CC BY SA 4.0
namespace dbj::nanolib {

    // dc == delimiter code
    enum class dc : char { LEFT_BRACE = '{', RIGHT_BRACE = '}', LEFT_SQ_BRACE = '[', RIGHT_SQ_BRACE = ']', COMMA = ',', DOT = '.', SPACE = ' '};

    constexpr char default_delimiters(dc idx_) {  return char(idx_); };

    using delimiters_function = char (*)(dc);

    // print a sequence
    // arguments are two iterators
    // pointing to it
    template<typename Iterator >
    inline void sequence_print(
        Iterator begin_, 
        Iterator end_, 
        delimiters_function delimiters, 
        bool show_size) 
    {
        using namespace std;
        auto size_ = [&] { return  size_t(distance(begin_, end_));  };
        if (show_size) {
            DBJ_PRINT( " sequence " , delimiters(dc::LEFT_SQ_BRACE) , "size:" , size_() , delimiters(dc::RIGHT_SQ_BRACE));
        }
        log( delimiters(dc::SPACE) , delimiters(dc::LEFT_BRACE));
        auto walker = begin_;
        if (walker != end_)
        {
            // first sequence element
            // no leading comma
            DBJ_PRINT( delimiters(dc::SPACE) , *walker);
            walker++;
            // other elements, if any
             // with a leading comma
            while (walker != end_) {
                DBJ_PRINT( delimiters(dc::SPACE) , delimiters(dc::COMMA) , delimiters(dc::SPACE) , *walker);
                // operator '++' advances the iterator
                // logically to the 'right'
                // see the diagram below
                walker++;
            }
        }
        DBJ_PRINT( delimiters(dc::SPACE) , delimiters(dc::RIGHT_BRACE));
    }

    // print a sequence with a comma in between elements
    // the only requirement is the 'sequence' conforms
    // to the concept as on the diagram bellow
    // sequence size is also printed by default
    template<typename Sequence >
    inline void sequence_print(
        Sequence const& seq_,
        bool show_size = true,
        delimiters_function delimiters = default_delimiters
    ) {
        using namespace std;
        sequence_print(begin(seq_), end(seq_), delimiters, show_size);
    }

} // dbj::nanolib



#endif // !DBJ_SEQUENCE_PRINT_INC
