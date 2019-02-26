// *****************************************************************************
/*!
  \file      src/NoWarning/pugixml.h
  \copyright 2012-2015, J. Bakosi, 2016-2019, Triad National Security, LLC.
  \brief     Include pugixml.hpp with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_pugixml_h
#define nowarning_pugixml_h

#include "Macro.h"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated"
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#include <pugixml.hpp>

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#endif // nowarning_pugixml_h
