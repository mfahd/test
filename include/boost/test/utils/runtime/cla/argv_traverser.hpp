//  (C) Copyright Gennadiy Rozental 2001.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : defines facility to hide input traversing details
// ***************************************************************************

#ifndef BOOST_TEST_UTILS_RUNTIME_CLA_ARGV_TRAVERSER_HPP
#define BOOST_TEST_UTILS_RUNTIME_CLA_ARGV_TRAVERSER_HPP

#include <algorithm>
#include <list>

// Boost.Test Runtime parameters
#include <boost/test/utils/runtime/fwd.hpp>

#include <boost/test/detail/suppress_warnings.hpp>

namespace boost {
namespace runtime {
namespace cla {

// ************************************************************************** //
// **************          runtime::cla::argv_traverser        ************** //
// ************************************************************************** //

/// End of input token indicator
static const char END_OF_TOKEN = '\0';

class argv_traverser {
    typedef char const*         argv_element_type; //!@todo
    typedef argv_element_type*  argv_type;
    typedef std::list< std::pair<bool, argv_type> >::iterator iterator_t;
public:
    /// Constructs traverser based on argc/argv pair
    /// argv is taken "by reference" and later can be
    /// updated in remainder method
    argv_traverser( int argc, argv_type argv )
    : m_argc( argc )
    , m_curr_arg( 0 )
    , m_arg_size( 0 )
    , m_arg_pos( 0 )
    , m_argv( argv )
    {
        for(int i = 1; i < argc; i++)
            m_remainder.push_back(std::make_pair(false, argv + i));
        next_arg();
    }

    /// Eat the argument (does not appear in the remainder)
    void        eat(int index)
    {
        iterator_t it(m_remainder.begin());
        std::advance(it, index-1);
        it->first = true;
    }

    /// Updates argv to contain the remainder of the input
    /// and returns new argc
    int         remainder()
    {
        std::size_t new_argc = 0;

        for(  iterator_t it(m_remainder.begin());
              it != m_remainder.end() ;
              ++it ) {
            if(it->first)
                continue; // if eaten, continue

            new_argc ++;  // we start at argv[1]
            m_argv[new_argc] = *(it->second);
        }

        m_remainder.clear(); // so that the method is idempotent

        // Raffi: ???
        //m_argv[1] += m_arg_pos;

        return (int)new_argc + 1; // argv starts at 1, we still have argv[0]
    }

    /// Returns true, if we reached end on input
    bool        eoi() const
    {
        return m_curr_arg == m_argc;
    }

    /// For the purposes of error reporting produces current token being parsed
    /// (from the begining)
    cstring     current_token()
    {
        if( eoi() )
            return cstring();

        return cstring( m_argv[m_curr_arg], m_arg_size );
    }

    /// Skips ahead by num_chars characters
    void        skip( std::size_t num_chars )
    {
        m_arg_pos += num_chars;
        if( m_arg_pos >= m_arg_size ) {
            next_arg();
        }
    }

    /// Gets single character from input. If we reached end of
    /// input, alwars returns END_OF_TOKEN. If we reached end
    /// of token returns END_OF_TOKEN and moves to next token.
    /// Note that END_OF_TOKEN is returned after we read the
    /// last charter in a token
    char        get_char()
    {
        if( eoi() )
            return END_OF_TOKEN;

        if( m_arg_pos == m_arg_size ) {
            next_arg();
            return END_OF_TOKEN;
        }

        return m_argv[m_curr_arg][m_arg_pos++];
    }

    /// Returns all the characters remaining in the current token and moves
    /// to next token
    cstring     get_token()
    {
        if( eoi() )
            return cstring();

        cstring token( m_argv[m_curr_arg] + m_arg_pos, m_arg_size - m_arg_pos );

        next_arg();

        return token;
    }

private:
    void        next_arg()
    {
        ++m_curr_arg;

        if( !eoi() ) {
            m_arg_size = ::strlen( m_argv[m_curr_arg] );
            m_arg_pos  = 0;
        }
    }

    // Data members
    std::size_t m_argc;         // total number of arguments
    std::size_t m_curr_arg;     // current argument index in argv
    std::size_t m_arg_size;     // current argument size
    std::size_t m_arg_pos;      // current argument position
    argv_type   m_argv;         // all arguments
    std::list< std::pair<bool, argv_type> > m_remainder;
};

} // namespace cla
} // namespace runtime
} // namespace boost

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_UTILS_RUNTIME_CLA_ARGV_TRAVERSER_HPP
