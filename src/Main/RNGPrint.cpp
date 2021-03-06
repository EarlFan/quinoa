// *****************************************************************************
/*!
  \file      src/Main/RNGPrint.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Pretty printer base for pretty printers supporting RNGs
  \details   Pretty printer base for pretty printers supporting RNGs.
*/
// *****************************************************************************

#include <map>
#include <utility>

#include "NoWarning/format.hpp"

#include "QuinoaConfig.hpp"
#include "Tags.hpp"
#include "Print.hpp"
#include "RNGParam.hpp"
#include "RNGPrint.hpp"
#include "Options/RNG.hpp"
#include "Options/RNGSSESeqLen.hpp"

#ifdef HAS_MKL
  #include "Options/MKLGaussianMethod.hpp"
  #include "Options/MKLGaussianMVMethod.hpp"
  #include "Options/MKLUniformMethod.hpp"
#endif

using tk::RNGPrint;

#ifdef HAS_MKL
void
RNGPrint::echoMKLParams( const ctr::RNGMKLParam& p ) const
// *****************************************************************************
//  Echo information on MKL random number generator
//! \param[in] p MKL RNG parameters
// *****************************************************************************
{
  ctr::MKLUniformMethod um;
  ctr::MKLGaussianMethod gm;
  ctr::MKLGaussianMVMethod gmvm;

  m_stream << m_item_name_value_fmt
              % m_item_indent
              % "seed"
              % p.get< tag::seed >();

  m_stream << m_item_name_value_fmt
              % m_item_indent
              % um.group()
              % um.name( p.get< tag::uniform_method >() );

  m_stream << m_item_name_value_fmt
              % m_item_indent
              % gm.group()
              % gm.name( p.get< tag::gaussian_method >() );

  m_stream << m_item_name_value_fmt
              % m_item_indent
              % gmvm.group()
              % gmvm.name( p.get< tag::gaussianmv_method >() );
}
#endif

void
RNGPrint::echoRNGSSEParams( const ctr::RNGSSEParam& p,
                            const ctr::RNG& rng,
                            const ctr::RNGType& r ) const
// *****************************************************************************
//  Echo information on RNGSSE random number generator
//! \param[in] p RNGSSE RNG parameters
//! \param[in] rng RNG options object
//! \param[in] r RNG type enum
// *****************************************************************************
{
  m_stream << m_item_name_value_fmt
              % m_item_indent
              % "seed"
              % p.get< tag::seed >();

  if ( rng.supportsSeq(r) ) {
    ctr::RNGSSESeqLen seq;
    m_stream << m_item_name_value_fmt
                % m_item_indent
                % seq.group()
                % seq.name( p.get< tag::seqlen >() );
  }
}

void
RNGPrint::echoRandom123Params( const ctr::RNGRandom123Param& p ) const
// *****************************************************************************
//  Echo information on Random123 random number generator
//! \param[in] p Random123 RNG parameters
// *****************************************************************************
{
  m_stream << m_item_name_value_fmt
              % m_item_indent
              % "seed"
              % p.get< tag::seed >();
}

#ifdef HAS_MKL
void
RNGPrint::MKLParams( const std::vector< ctr::RNGType >& vec,
                     const ctr::RNGMKLParameters& map ) const
// *****************************************************************************
//  Print all fields of MKL RNG parameters
//! \param[in] vec Vector of RNG type enums to print
//! \param[in] map MKL RNG parameters map
// *****************************************************************************
{
  ctr::RNG rng;

  for (auto& r : vec) {
    if (rng.lib(r) == ctr::RNGLibType::MKL) {
      subsection( rng.name(r) );
      const auto& m = map.find(r);
      if (m == map.end()) {   // no parameter map entry, print defaults
        echoMKLParams( ctr::RNGMKLParam() );
      } else {
        echoMKLParams( m->second );
      }
    }
  }
}
#endif

void
RNGPrint::RNGSSEParams( const std::vector< ctr::RNGType >& vec,
                        const ctr::RNGSSEParameters& map ) const
// *****************************************************************************
//  Print all fields of RNGSSE RNG parameters
//! \param[in] vec Vector of RNG type enums to print
//! \param[in] map RNGSSE RNG parameters map
// *****************************************************************************
{
  ctr::RNG rng;

  for (auto& r : vec) {
    if (rng.lib(r) == ctr::RNGLibType::RNGSSE) {
      subsection( rng.name(r) );
      const auto& m = map.find(r);
      if (m == map.end()) {   // no parameter map entry, print defaults
        echoRNGSSEParams( ctr::RNGSSEParam(), rng, r );
      } else {
        echoRNGSSEParams( m->second, rng, r );
      }
    }
  }
}

void
RNGPrint::Random123Params( const std::vector< ctr::RNGType >& vec,
                           const ctr::RNGRandom123Parameters& map ) const
// *****************************************************************************
//  Print all fields of Random123 RNG parameters
//! \param[in] vec Vector of RNG type enums to print
//! \param[in] map Random123 RNG parameters map
// *****************************************************************************
{
  ctr::RNG rng;

  for (auto& r : vec) {
    if (rng.lib(r) == ctr::RNGLibType::R123) {
      subsection( rng.name(r) );
      const auto& m = map.find(r);
      if (m == map.end()) {   // no parameter map entry, print defaults
        echoRandom123Params( ctr::RNGRandom123Param() );
      } else {
        echoRandom123Params( m->second );
      }
    }
  }
}
