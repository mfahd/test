//  (C) Copyright Gennadiy Rozental 2001.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : model of actual argument (both typed and abstract interface)
// ***************************************************************************

#ifndef BOOST_TEST_UTILS_RUNTIME_ARGUMENT_HPP
#define BOOST_TEST_UTILS_RUNTIME_ARGUMENT_HPP

// Boost.Runtime.Parameter
#include <boost/test/utils/runtime/config.hpp>
#include <boost/test/utils/runtime/fwd.hpp>

// Boost.Test
#include <boost/test/utils/class_properties.hpp>
#include <boost/test/utils/rtti.hpp>

// STL
#include <cassert>

namespace boost {
namespace runtime {

// ************************************************************************** //
// **************              runtime::argument               ************** //
// ************************************************************************** //

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable:4244)
#endif

class argument {
public:
    // Constructor
    argument( basic_param const& p, rtti::id_t value_type )
    : p_formal_parameter( p )
    , p_value_type( value_type )
    {}

    // Destructor
    virtual     ~argument()  {}

    // Public properties
    unit_test::readonly_property<basic_param const&> p_formal_parameter;
    unit_test::readonly_property<rtti::id_t>         p_value_type;
};

// ************************************************************************** //
// **************             runtime::typed_argument          ************** //
// ************************************************************************** //

template<typename T>
class typed_argument : public argument {
public:
    // Constructor
    explicit typed_argument( basic_param const& p )
    : argument( p, rtti::type_id<T>() )
    {}
    typed_argument( basic_param const& p, T const& t )
    : argument( p, rtti::type_id<T>() )
    , p_value( t )
    {}

    unit_test::readwrite_property<T>    p_value;
};

// ************************************************************************** //
// **************               runtime::arg_value             ************** //
// ************************************************************************** //

template<typename T>
inline T const&
arg_value( argument const& arg_ )
{
    assert( arg_.p_value_type == rtti::type_id<T>() ); // detect logic error

    return static_cast<typed_argument<T> const&>( arg_ ).p_value.value;
}

//____________________________________________________________________________//

template<typename T>
inline T&
arg_value( argument& arg_ )
{
    assert( arg_.p_value_type == rtti::type_id<T>() ); // detect logic error

    return static_cast<typed_argument<T>&>( arg_ ).p_value.value;
}

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

//____________________________________________________________________________//

} // namespace runtime
} // namespace boost

#endif // BOOST_TEST_UTILS_RUNTIME_ARGUMENT_HPP
