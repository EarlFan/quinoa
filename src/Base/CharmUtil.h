// *****************************************************************************
/*!
  \file      src/Base/CharmUtil.h
  \copyright 2012-2015, J. Bakosi, 2016-2017, Los Alamos National Security, LLC.
  \brief     Charm++ utilities
  \details   Charm++ utilities
*/
// *****************************************************************************
#ifndef CharmUtil_h
#define CharmUtil_h

#include "NoWarning/bool.h"

namespace tk {

//! Type trait querying whether T is a strongly typed enum
template< typename T >
using is_enum_class = typename boost::mpl::bool_<
                        std::is_enum< T >::value &&
                        !std::is_convertible< T, uint8_t >::value >;

} // tk::

#endif // CharmUtil_h
