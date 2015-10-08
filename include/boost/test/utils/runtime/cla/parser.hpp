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
//  Description : defines parser - public interface for CLA parsing and accessing
// ***************************************************************************

#ifndef BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP
#define BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP

// Boost.Runtime.Parameter
#include <boost/test/utils/runtime/config.hpp>
#include <boost/test/utils/runtime/argument.hpp>
#include <boost/test/utils/runtime/modifier.hpp>
#include <boost/test/utils/runtime/parameter.hpp>

#include <boost/test/utils/runtime/cla/argv_traverser.hpp>

// Boost.Test
#include <boost/test/utils/foreach.hpp>
#include <boost/test/detail/throw_exception.hpp>

namespace boost {
namespace runtime {
namespace cla {

// ************************************************************************** //
// **************         runtime::cla::parameter_trie         ************** //
// ************************************************************************** //

namespace rt_cla_detail {

class parameter_trie;
typedef shared_ptr<parameter_trie> parameter_trie_ptr;
typedef std::map<char,parameter_trie_ptr> trie_per_char;

class parameter_trie {
public:
    /// If subtrie corresponding to the char c exists returns it otherwise creates new
    parameter_trie_ptr  make_subtrie( char c )
    {
        trie_per_char::const_iterator it = m_subtrie.find( c );

        if( it == m_subtrie.end() )
            it = m_subtrie.insert( std::make_pair( c, parameter_trie_ptr( new parameter_trie ) ) ).first;

        return it->second;
    }

    /// Creates series of sub-tries per characters in a string
    parameter_trie_ptr  make_subtrie( cstring s )
    {
        parameter_trie_ptr res;

        BOOST_TEST_FOREACH( char, c, s )
            res = (res ? res->make_subtrie( c ) : make_subtrie( c ));

        return res;
    }

    /// Registers candidate parameter for this subtrie. If final, it needs to be unique
    void                add_param_candidate( basic_param_ptr param, bool final )
    {
        if( final ) {
            if( !m_candidates.empty() )
                BOOST_TEST_IMPL_THROW( init_error() << "Parameter " << param->p_name << " conflicts with "
                                                     << "parameter " << m_candidates.front()->p_name );
            m_final_candidate = param;
        }
        else {
            if( m_final_candidate )
                BOOST_TEST_IMPL_THROW( init_error() << "Parameter " << param->p_name << " conflicts with "
                                                     << "parameter " << m_final_candidate->p_name );
        }

        m_candidates.push_back( param );
    }

    /// Gets subtrie for specified char if present or nullptr otherwise
    parameter_trie_ptr  get_subtrie( char c ) const
    {
        trie_per_char::const_iterator it = m_subtrie.find( c );

        return it != m_subtrie.end() ? it->second : parameter_trie_ptr();
    }
private:
    typedef std::vector<basic_param_ptr> param_list;

    // Data members
    trie_per_char       m_subtrie;
    param_list          m_candidates;
    basic_param_ptr     m_final_candidate;
};

} // namespace rt_cla_detail

// ************************************************************************** //
// **************             runtime::cla::parser             ************** //
// ************************************************************************** //

class parser {
public:
    /// Initializes a parser and builds internal trie representation used for
    /// parsing based on the supplied parameters
    template<typename Modifiers=nfp::no_params_type>
    parser( parameters_store const& parameters, Modifiers const& m = nfp::no_params)
    {
        build_trie( parameters );
    }

    // input processing method
    int         parse( int argc, char** argv, runtime::arguments_store& res )
    {
        // 10. Set up the traverser
        argv_traverser tr( argc, (char const**)argv );

        // 20. Loop till we reach end of input
        while( tr.eoi() ) {
            trie_ptr curr_trie = m_param_trie;

            // 30. Dig through the prefix and name characters
            char next_char;
            while( curr_trie && (next_char = tr.get_char()) != cla::END_OF_TOKEN ) {
                trie_ptr sub_trie = curr_trie->get_subtrie( next_char );

                if( !sub_trie ) {
                    BOOST_TEST_IMPL_THROW( init_error() << "Failed to recognize the parameter in token " 
                                                         << tr.current_token() );
                }

                curr_trie = sub_trie;
            }
        }

        return tr.remainder();
    }

    // help/usage
    void        usage( std::ostream& ostr );

private:
    void    build_trie( parameters_store const& parameters )
    {
        m_param_trie.reset( new rt_cla_detail::parameter_trie );

        // 10. Iterate over all parameters
        BOOST_TEST_FOREACH( parameters_store::storage_type::value_type const&, v, parameters.all() ) {
            basic_param_ptr param = v.second;

            // 20. Register all parameter's ids in trie.
            BOOST_TEST_FOREACH( parameter_cla_id const&, id, param->cla_ids() ) {
                // 30. This is going to be cursor pointing to tail trie. Process prefix chars first.
                trie_ptr next_trie = m_param_trie->make_subtrie( id.m_prefix );

                // 40. Process name chars.
                for( size_t index = 0; index < id.m_full_name.size(); ++index ) {
                    next_trie = next_trie->make_subtrie( id.m_full_name[index] );

                    next_trie->add_param_candidate( param, index == (id.m_full_name.size() - 1) );
                }
            }
        }
    }

    typedef rt_cla_detail::parameter_trie_ptr trie_ptr;

    // Data members
    std::string m_program_name;
    trie_ptr    m_param_trie;
};

} // namespace cla
} // namespace runtime
} // namespace boost

#endif // BOOST_TEST_UTILS_RUNTIME_CLA_PARSER_HPP
