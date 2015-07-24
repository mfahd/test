//  (C) Copyright Gennadiy Rozental 2005-2015.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : defines parser - public interface for CLA parsing and accessing
// ***************************************************************************

#ifndef BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP
#define BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP

// Boost.Runtime.Parameter
#include <boost/test/utils/runtime/config.hpp>
#include <boost/test/utils/runtime/fwd.hpp>
#include <boost/test/utils/runtime/argument.hpp>

#include <boost/test/utils/runtime/cla/fwd.hpp>
#include <boost/test/utils/runtime/cla/modifier.hpp>
#include <boost/test/utils/runtime/cla/argv_traverser.hpp>

// Boost
#include <boost/optional.hpp>

// STL
#include <list>

namespace boost {
namespace runtime {
namespace cla {

// ************************************************************************** //
// **************             runtime::cla::parser             ************** //
// ************************************************************************** //

namespace cla_detail {

template<typename Modifier>
class global_mod_parser {
public:
    global_mod_parser( parser& p, Modifier const& m )
    : m_parser( p )
    , m_modifiers( m )
    {}

    template<typename Param>
    global_mod_parser const&
    operator<<( shared_ptr<Param> param ) const
    {
        param->accept_modifier( m_modifiers );

        m_parser << param;

        return *this;
    }

private:
    // Data members;
    parser&             m_parser;
    Modifier const&     m_modifiers;
};

}

// ************************************************************************** //
// **************             runtime::cla::parser             ************** //
// ************************************************************************** //

class parser {
public:
    typedef std::list<parameter_ptr>::const_iterator param_iterator;
    typedef std::list<parameter_ptr>::size_type param_size_type;

    // Constructor
    explicit                    parser( cstring program_name = cstring() );

    // parameter list construction interface
    parser&                     operator<<( parameter_ptr param );

    // parser and global parameters modifiers
    template<typename Modifier>
    cla_detail::global_mod_parser<Modifier>
    operator-( Modifier const& m )
    {
        nfp::optionally_assign( m_traverser.p_separator.value, m, input_separator );
        nfp::optionally_assign( m_traverser.p_ignore_mismatch.value, m, ignore_mismatch_m );

        return cla_detail::global_mod_parser<Modifier>( *this, m );
    }

    // input processing method
    void                        parse( int& argc, char** argv );

    // parameters access
    param_iterator              first_param() const;
    param_iterator              last_param() const;
    param_size_type             num_params() const  { return m_parameters.size(); }
    void                        reset();

    // arguments access
    const_argument_ptr          operator[]( cstring string_id ) const;
    cstring                     get( cstring string_id ) const;

    template<typename T>
    T const&                    get( cstring string_id ) const
    {
        return arg_value<T>( valid_argument( string_id ) );
    }

    template<typename T>
    void                        get( cstring string_id, boost::optional<T>& res ) const
    {
        const_argument_ptr actual_arg = (*this)[string_id];

        if( actual_arg )
            res = arg_value<T>( *actual_arg );
        else
            res.reset();
    }

    // help/usage
    void                        usage( std::ostream& ostr );
    void                        help(  std::ostream& ostr );

private:
    argument const&             valid_argument( cstring string_id ) const;

    // Data members
    argv_traverser              m_traverser;
    std::list<parameter_ptr>    m_parameters;
    std::string                 m_program_name;
};

//____________________________________________________________________________//

} // namespace cla
} // namespace runtime
} // namespace boost

#ifndef BOOST_TEST_UTILS_RUNTIME_PARAM_OFFLINE

#ifndef BOOST_TEST_UTILS_RUNTIME_PARAM_INLINE
#   define BOOST_TEST_UTILS_RUNTIME_PARAM_INLINE inline
#endif
# include <boost/test/utils/runtime/cla/parser.ipp>

#endif

#endif // BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP